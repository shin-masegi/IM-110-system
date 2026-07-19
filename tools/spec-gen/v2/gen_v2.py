#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
gen_v2.py — v2 generation: Devstral authors each screen spec by READING the FW.

For each section in sections_v2.json:
  prompt = compact facts header + the section's Display.c function source(s)
           + an ID-200T-depth writing instruction.
  -> Devstral (devstral:24b) reads the code and writes the spec subsection.
  -> out/frag_<id>.md

Claude's role is manager+verifier only; the analysis (code -> spec) is Devstral's.

Robustness (this is an experiment — failure is acceptable but must be analyzable):
  - Every call logs to out/gen_v2_log.jsonl: prompt/eval tokens, duration,
    ok/err, retry count, prompt char size.
  - The exact prompt sent is saved to out/prompt_<id>.txt for post-mortem.
  - On hard failure a section is marked failed and the run CONTINUES.
  - Resumable: a section with out/frag_<id>.md is skipped unless --force.
  - 45s cooldown between sections (GPU thermal relief; fan-control is unreliable).

Usage:
  python3 tools/spec-gen/v2/gen_v2.py                 # all
  python3 tools/spec-gen/v2/gen_v2.py --only S01,S05   # subset
  SLEEP_BETWEEN=45 python3 tools/spec-gen/v2/gen_v2.py
"""
import argparse
import json
import os
import time
import traceback
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parent
CODE = ROOT / "code"
OUT = ROOT / "out"

HOST = os.environ.get("HOST", "192.168.0.77:11434")
MODEL = os.environ.get("GEN_MODEL", "devstral:24b")
NUM_CTX = int(os.environ.get("NUM_CTX", "32768"))
TIMEOUT = int(os.environ.get("TIMEOUT", "600"))
RETRIES = int(os.environ.get("RETRIES", "3"))
SLEEP_BETWEEN = float(os.environ.get("SLEEP_BETWEEN", "45"))

FACTS = """\
# 対象機器の前提事実（IM-110T 本体）
- 製品: MLSSメーター／IM-110T（活性汚泥処理の水質計測。測定項目 MLSS/SS/透視度/界面）。
- 表示: モノクロLCD 400×240（Sharp LS027B7）。関数内の `LS027_disp_icon` / `LS027_disp_number` 等が画面への描画呼び出し。
- 操作スイッチは3個: 電源(SW3) / 記録(SW1) / 履歴(SW2)。タッチパネルではない。画面内の「開始」等はアイコン表示であってタッチボタンではない。
- 各画面の最上段にタイトル・日付時刻・電池残量、BT接続中はBTアイコン(共通表示)。
- `batvol` は電池残量、`flash_flg`/`fl` は点滅表示制御、`select`/`num_sel` は選択/桁位置、`en` は有効/無効。
"""

SYSTEM = (
    "あなたは日本の計測器メーカーの技術ライターで、C言語の組込みFWを読める。"
    "与えられた『実装コード』を読み、その画面の動作仕様を日本語で執筆する。厳守事項:\n"
    "0. 出力は日本語のみ（型番・関数名・信号名・コマンド名などの固有名詞のみ英字可）。英文への切替は禁止。\n"
    "1. 画面に表示される要素は、コード中の描画呼び出し(LS027_disp_icon / LS027_disp_number 等)と"
    "そのメッセージ定数・座標・引数から具体的に読み取って記述する。推測で表示物を創作しない。\n"
    "2. 関数の引数(batvol, flash_flg, select, op_mode 等)が画面挙動にどう効くかを説明する。\n"
    "3. コードから読み取れない事項（他画面への遷移条件、スイッチ割当の詳細など。"
    "これらは呼び出し側 Normal.c/Setting.c/Adjust.c にあり本コードには無い）は"
    "『(要確認: 呼出側FW)』と明記し、創作しない。\n"
    "4. ID-200T 仕様書相当の粒度で、である調。見出し(#)は付けず本文のみ。前置き・自己言及・英語の結論文は禁止。"
)


def ollama(prompt: str) -> dict:
    body = json.dumps({
        "model": MODEL, "system": SYSTEM, "prompt": prompt, "stream": False,
        "options": {"temperature": 0.2, "num_ctx": NUM_CTX},
    }).encode("utf-8")
    req = urllib.request.Request(f"http://{HOST}/api/generate", data=body,
                                 headers={"Content-Type": "application/json"})
    last = None
    for attempt in range(1, RETRIES + 1):
        try:
            with urllib.request.urlopen(req, timeout=TIMEOUT) as r:
                return {"ok": True, "attempt": attempt, **json.loads(r.read().decode("utf-8"))}
        except Exception as e:  # noqa: BLE001
            last = e
            print(f"      ! attempt {attempt}/{RETRIES}: {e}")
            time.sleep(5 * attempt)
    return {"ok": False, "attempt": RETRIES, "error": repr(last)}


def build_prompt(sec: dict) -> str:
    code_blocks = []
    for fn in sec["functions"]:
        p = CODE / f"{fn}.c"
        src = p.read_text(encoding="utf-8") if p.exists() else f"// (missing {fn})"
        code_blocks.append(f"// ===== {fn} =====\n{src}")
    code = "\n\n".join(code_blocks)
    note = f"\n# 補足指示\n{sec['note']}\n" if sec.get("note") else ""
    return (
        f"{FACTS}\n"
        f"# 執筆対象画面: {sec['num']} {sec['title']}\n"
        f"# 実装コード（IM-110本体 Core/Src/Display.c より抜粋）\n"
        f"```c\n{code}\n```\n"
        f"{note}\n"
        f"上記コードが実装する画面「{sec['title']}」の動作仕様を、"
        f"表示要素・引数の効果・状態ごとの差異を含めてID-200T相当の粒度で日本語で書いてください。"
    )


def log(rec: dict) -> None:
    with (OUT / "gen_v2_log.jsonl").open("a", encoding="utf-8") as f:
        f.write(json.dumps(rec, ensure_ascii=False) + "\n")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--only", default="")
    ap.add_argument("--force", action="store_true")
    args = ap.parse_args()

    OUT.mkdir(parents=True, exist_ok=True)
    data = json.loads((ROOT / "sections_v2.json").read_text(encoding="utf-8"))
    sections = data["sections"]
    only = {s.strip() for s in args.only.split(",") if s.strip()}
    todo = [s for s in sections if not only or s["id"] in only]
    print(f"gen_v2: host={HOST} model={MODEL} num_ctx={NUM_CTX} "
          f"sections={len(todo)} cooldown={SLEEP_BETWEEN}s force={args.force}")

    ok = fail = 0
    for i, sec in enumerate(todo, 1):
        sid = sec["id"]
        frag = OUT / f"frag_{sid}.md"
        if frag.exists() and not args.force:
            print(f"[{i}/{len(todo)}] {sid} {sec['title']} — skip (exists)")
            continue
        prompt = build_prompt(sec)
        (OUT / f"prompt_{sid}.txt").write_text(prompt, encoding="utf-8")
        print(f"[{i}/{len(todo)}] {sid} {sec['num']} {sec['title']} "
              f"({len(sec['functions'])} funcs, {len(prompt)} chars) — generating ...")
        t0 = time.time()
        try:
            r = ollama(prompt)
            dt = time.time() - t0
            if r.get("ok") and r.get("response", "").strip():
                frag.write_text(r["response"].strip(), encoding="utf-8")
                ok += 1
                print(f"      OK {r.get('eval_count')} tok in {dt:.1f}s "
                      f"(prompt {r.get('prompt_eval_count')} tok, try {r['attempt']})")
                log({"id": sid, "num": sec["num"], "title": sec["title"], "ok": True,
                     "funcs": sec["functions"], "prompt_chars": len(prompt),
                     "prompt_tok": r.get("prompt_eval_count"), "eval_tok": r.get("eval_count"),
                     "sec": round(dt, 1), "attempts": r["attempt"]})
            else:
                fail += 1
                err = r.get("error") or "empty response"
                print(f"      FAIL: {err}")
                log({"id": sid, "num": sec["num"], "title": sec["title"], "ok": False,
                     "funcs": sec["functions"], "prompt_chars": len(prompt),
                     "error": err, "sec": round(dt, 1), "attempts": r.get("attempt")})
        except Exception as e:  # noqa: BLE001 - never let one section kill the run
            fail += 1
            print(f"      EXCEPTION: {e}")
            log({"id": sid, "num": sec["num"], "title": sec["title"], "ok": False,
                 "error": repr(e), "traceback": traceback.format_exc(),
                 "prompt_chars": len(prompt)})
        if SLEEP_BETWEEN > 0 and i < len(todo):
            print(f"      cooldown {SLEEP_BETWEEN:g}s")
            time.sleep(SLEEP_BETWEEN)

    print(f"gen_v2: done. ok={ok} fail={fail}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
