#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
gen.py — Phase 1 overnight generation against the on-prem Ollama (77 server).

For each section in sections.json:
  1. Devstral drafts the section prose from the fact pack (+ ID-200T template
     for body sections). -> out/frag_<id>.md
  2. mistralprism reviews the draft against the SAME facts, flags anything not
     grounded in the facts, and emits a corrected version. -> out/review_<id>.md
     The corrected version becomes out/final_<id>.md (assemble.py prefers it).

Design notes:
  - Sequential (concurrency 1) to be a good citizen on the shared 77 server.
  - Resumable: a section whose out/final_<id>.md already exists is skipped
    unless --force. Safe to re-run after an interruption overnight.
  - Every call is timed and logged to out/gen_log.jsonl (experiment data).
  - Temperature low for factual prose.

Usage:
  python3 tools/spec-gen/gen.py                 # all sections
  python3 tools/spec-gen/gen.py --only A04,B04  # subset (dry-run)
  python3 tools/spec-gen/gen.py --force         # regenerate even if final exists
  HOST=192.168.0.77:11434 python3 tools/spec-gen/gen.py
"""
import argparse
import json
import os
import time
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parent
REPO = ROOT.parents[1]
CTX = ROOT / "context"
OUT = ROOT / "out"

HOST = os.environ.get("HOST", "192.168.0.77:11434")
GEN_MODEL = os.environ.get("GEN_MODEL", "devstral:24b")
REVIEW_MODEL = os.environ.get("REVIEW_MODEL", "mistralprism:24b")
NUM_CTX = int(os.environ.get("NUM_CTX", "16384"))
TIMEOUT = int(os.environ.get("TIMEOUT", "600"))
RETRIES = int(os.environ.get("RETRIES", "3"))
# Cooldown seconds to idle the GPU between sections (thermal relief). The model
# stays resident in VRAM; only the compute units idle and cool. Default 0.
SLEEP_BETWEEN = float(os.environ.get("SLEEP_BETWEEN", "0"))


def cooldown(label: str) -> None:
    if SLEEP_BETWEEN > 0:
        print(f"      cooldown {SLEEP_BETWEEN:g}s ({label})")
        time.sleep(SLEEP_BETWEEN)

SYSTEM_GEN = (
    "あなたは日本の計測器メーカーの技術ライターです。組込み計測器のプログラム仕様書を"
    "日本語で執筆します。厳守事項:\n"
    "0. 【最重要】出力は必ず日本語のみ。英語の文・句・単語を使わない。"
    "例外は型番・信号名・コマンド名・単位などの固有名詞(STM32L452RET6, LPUART1, MLSS, MD, mV 等)だけ。"
    "英文への切り替えは絶対禁止。\n"
    "1. 与えられた『事実パック』と『テンプレート』に書かれた事実のみを使う。"
    "ハードのピン番号・チャネル・コマンド・数値レンジを推測で創作しない。\n"
    "2. 事実が無い箇所は『(本体FWと突き合わせ校正時に確定)』のように日本語で明示する。\n"
    "3. 出力はMarkdownの本文のみ。見出し(#)は付けず、段落・箇条書き・必要なら表で書く。"
    "画像リンクは書かない(後工程で挿入)。前置き・後書き・自己言及・英語の結論文は禁止。\n"
    "4. 簡潔で技術的、である調。ID-200T仕様書の文体に合わせる。"
)

SYSTEM_REVIEW = (
    "あなたは計測器仕様書の技術レビュアーです。与えられた『事実パック』に対して"
    "『ドラフト』を検証します。厳守事項:\n"
    "1. 事実パックに無いハード仕様・コマンド・数値を創作している箇所(ハルシネーション)を特定する。\n"
    "2. 事実パックと矛盾する記述を修正する。\n"
    "3. 出力は次の2ブロックのみ:\n"
    "=== NOTES ===\n(問題点を箇条書き。無ければ『問題なし』)\n"
    "=== REVISED ===\n(修正後の本文Markdown。問題が無ければドラフトをそのまま再掲)\n"
    "前置き・自己言及は禁止。"
)


def ollama(model: str, system: str, prompt: str) -> dict:
    body = json.dumps({
        "model": model,
        "system": system,
        "prompt": prompt,
        "stream": False,
        "options": {"temperature": 0.2, "num_ctx": NUM_CTX},
    }).encode("utf-8")
    req = urllib.request.Request(
        f"http://{HOST}/api/generate", data=body,
        headers={"Content-Type": "application/json"})
    last = None
    for attempt in range(1, RETRIES + 1):
        try:
            with urllib.request.urlopen(req, timeout=TIMEOUT) as r:
                return json.loads(r.read().decode("utf-8"))
        except Exception as e:  # noqa: BLE001 - overnight robustness
            last = e
            print(f"      ! {model} attempt {attempt}/{RETRIES} failed: {e}")
            time.sleep(5 * attempt)
    raise RuntimeError(f"{model} failed after {RETRIES} attempts: {last}")


def build_gen_prompt(sec: dict, common: str, probe: str, template: str) -> str:
    if sec["section"] == "A":
        facts = common
        tpl = ("\n\n# 構造テンプレート(ID-200T仕様書。該当画面の書き方を参考に、"
               "IM-110の測定項目MLSS/SS/透視度/界面に翻案せよ)\n" + template)
    else:
        facts = probe
        tpl = ""
    return (
        f"# 事実パック\n{facts}{tpl}\n\n"
        f"# 執筆対象セクション\n"
        f"番号: {sec['num']}  タイトル: {sec['title']}\n"
        f"指示: {sec['directive']}\n\n"
        f"上記の事実のみを用いて、このセクションの本文をMarkdownで書いてください。"
    )


def build_review_prompt(sec: dict, common: str, probe: str, draft: str) -> str:
    facts = common if sec["section"] == "A" else probe
    return (
        f"# 事実パック\n{facts}\n\n"
        f"# 対象セクション\n{sec['num']} {sec['title']}\n\n"
        f"# ドラフト\n{draft}\n\n"
        f"上記ドラフトを事実パックに照らして検証し、指定フォーマットで出力してください。"
    )


def split_revised(review_text: str, fallback: str) -> str:
    marker = "=== REVISED ==="
    if marker not in review_text:
        return fallback
    body = review_text.split(marker, 1)[1].strip()
    # strip echoed prompt tail the reviewer sometimes re-emits
    for junk in ("上記ドラフトを事実パックに照らして",
                 "指定フォーマットで出力してください"):
        idx = body.find(junk)
        if idx != -1:
            body = body[:idx].rstrip()
    return body.strip()


def log(rec: dict) -> None:
    with (OUT / "gen_log.jsonl").open("a", encoding="utf-8") as f:
        f.write(json.dumps(rec, ensure_ascii=False) + "\n")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--only", default="", help="comma-separated section ids")
    ap.add_argument("--force", action="store_true")
    ap.add_argument("--stage", choices=["all", "gen", "review"], default="all",
                    help="gen=drafts only, review=reviews only, all=both per-section. "
                         "Run 'gen' then 'review' to avoid reloading models each section.")
    args = ap.parse_args()

    OUT.mkdir(parents=True, exist_ok=True)
    sections = json.loads((ROOT / "sections.json").read_text(encoding="utf-8"))["sections"]
    common = (CTX / "factpack_common.md").read_text(encoding="utf-8")
    probe = (CTX / "factpack_probe.md").read_text(encoding="utf-8")
    template = (CTX / "id200t_template.txt").read_text(encoding="utf-8", errors="ignore")

    only = {s.strip() for s in args.only.split(",") if s.strip()}
    todo = [s for s in sections if not only or s["id"] in only]
    print(f"gen: host={HOST} stage={args.stage} gen={GEN_MODEL} review={REVIEW_MODEL} "
          f"sections={len(todo)} force={args.force}")

    do_gen = args.stage in ("all", "gen")
    do_review = args.stage in ("all", "review")

    # -- generation pass (Devstral stays warm across the whole loop) --
    if do_gen:
        for i, sec in enumerate(todo, 1):
            sid = sec["id"]
            frag = OUT / f"frag_{sid}.md"
            if frag.exists() and not args.force:
                print(f"[gen {i}/{len(todo)}] {sid} {sec['title']} — skip (frag exists)")
                continue
            print(f"[gen {i}/{len(todo)}] {sid} {sec['title']} — drafting ...")
            t0 = time.time()
            gr = ollama(GEN_MODEL, SYSTEM_GEN,
                        build_gen_prompt(sec, common, probe, template))
            draft = gr.get("response", "").strip()
            frag.write_text(draft, encoding="utf-8")
            # seed final_ with the draft so assemble works even before review
            (OUT / f"final_{sid}.md").write_text(draft, encoding="utf-8")
            t_gen = time.time() - t0
            print(f"      draft {gr.get('eval_count')} tok in {t_gen:.1f}s")
            log({"stage": "gen", "id": sid, "num": sec["num"], "title": sec["title"],
                 "section": sec["section"], "gen_model": GEN_MODEL,
                 "gen_tok": gr.get("eval_count"),
                 "gen_prompt_tok": gr.get("prompt_eval_count"), "gen_sec": round(t_gen, 1)})
            cooldown("gen")

    # -- review pass (mistralprism stays warm across the whole loop) --
    if do_review:
        for i, sec in enumerate(todo, 1):
            sid = sec["id"]
            frag = OUT / f"frag_{sid}.md"
            review = OUT / f"review_{sid}.md"
            if not frag.exists():
                print(f"[rev {i}/{len(todo)}] {sid} — skip (no draft)")
                continue
            if review.exists() and not args.force:
                print(f"[rev {i}/{len(todo)}] {sid} {sec['title']} — skip (review exists)")
                continue
            print(f"[rev {i}/{len(todo)}] {sid} {sec['title']} — reviewing ...")
            draft = frag.read_text(encoding="utf-8")
            t1 = time.time()
            rr = ollama(REVIEW_MODEL, SYSTEM_REVIEW,
                        build_review_prompt(sec, common, probe, draft))
            review_text = rr.get("response", "").strip()
            review.write_text(review_text, encoding="utf-8")
            (OUT / f"final_{sid}.md").write_text(
                split_revised(review_text, draft), encoding="utf-8")
            t_rev = time.time() - t1
            print(f"      review {rr.get('eval_count')} tok in {t_rev:.1f}s")
            log({"stage": "review", "id": sid, "num": sec["num"], "title": sec["title"],
                 "section": sec["section"], "review_model": REVIEW_MODEL,
                 "review_tok": rr.get("eval_count"), "review_sec": round(t_rev, 1)})
            cooldown("review")

    print("gen: done.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
