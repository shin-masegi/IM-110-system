#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
gen_v3.py — v3 generation. Devstral authors each screen spec by reading BOTH the
drawing code (Display.c) AND the caller state-machine (Normal.c/Setting.c/Adjust.c),
so transitions + switch handling (②) are covered. Adds appendices for calc/
correction formulas (④, Calc.c + xlsx) and the EEPROM allocation table (⑤,
Eeprom.h). Unifies style to である調 (③). Probe English (①) is fixed by
regenerating the probe sections from probe FW in a separate step.

★ INCREMENTAL REGENERATION (experiment feature the user requested):
  Each section's INPUT is hashed (its Display.c function source(s) + caller
  case-blocks + appendix sources + PROMPT_VERSION). Hashes are stored in
  out/section_hashes.json. With --changed-only, ONLY sections whose input hash
  changed (i.e. the underlying code was edited) are regenerated. --dry-run
  reports what WOULD regenerate without calling the model.
  => edit a screen's function/handler, re-slice, and only that section rebuilds.

Usage:
  python3 tools/spec-gen/v3/gen_v3.py                  # all (first build)
  python3 tools/spec-gen/v3/gen_v3.py --changed-only   # only code-changed sections
  python3 tools/spec-gen/v3/gen_v3.py --changed-only --dry-run
  python3 tools/spec-gen/v3/gen_v3.py --only S05,APX_B
"""
import argparse
import hashlib
import json
import os
import time
import traceback
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parent
V2 = ROOT.parent / "v2"
CODE = V2 / "code"
CALLER = V2 / "caller"
OUT = ROOT / "out"

HOST = os.environ.get("HOST", "192.168.0.77:11434")
MODEL = os.environ.get("GEN_MODEL", "devstral:24b")
NUM_CTX = int(os.environ.get("NUM_CTX", "12288"))  # sized to fit 6900XT 16GB VRAM (no CPU offload); max prompt ~6.7k tok
TIMEOUT = int(os.environ.get("TIMEOUT", "900"))
RETRIES = int(os.environ.get("RETRIES", "3"))
SLEEP_BETWEEN = float(os.environ.get("SLEEP_BETWEEN", "45"))

# bump this when the prompt template changes -> forces full regeneration
PROMPT_VERSION = "v3.0"

FACTS = """\
# 前提事実（IM-110T 本体）
- 製品: MLSSメーター／IM-110T（測定項目 MLSS/SS/透視度/界面）。表示はモノクロLCD 400×240。
- 操作スイッチ3個: 電源 / 記録(MEM) / 履歴(DISP)。`MEM_sw_check()` `DISP_sw_check()` の戻り値≥3が押下・==4が長押し相当。
- 画面遷移は状態変数 `operation_mode`（AllDef.h 参照）で管理し、各 case ブロックで次状態を代入して遷移する。
- 描画は `Display.c` の `disp_*` 関数（`LS027_disp_*` 呼出）、遷移・スイッチ処理は呼出側 `Normal.c`/`Setting.c`/`Adjust.c` の case ブロックにある。
"""

SYSTEM = (
    "あなたは日本の計測器メーカーの技術ライターで、C言語の組込みFWを読める。"
    "与えられた実装コードを読み、画面の動作仕様を日本語で執筆する。厳守事項:\n"
    "0. 出力は日本語のみ（型番・関数名・状態名・コマンド名などの固有名詞のみ英字可）。英文への切替は禁止。\n"
    "1. 文体は「である調」に統一する（です・ます調を混在させない）。\n"
    "2. 画面の表示要素は描画コード(LS027_disp_* 等)から、画面遷移とスイッチ操作は呼出側 case ブロック"
    "（operation_mode への代入、MEM_sw_check/DISP_sw_check の判定）から具体的に読み取って記述する。推測で創作しない。\n"
    "3. 各 case の状態名(operation_mode)と、記録(MEM)/履歴(DISP)スイッチ押下・長押し時の遷移先を明記する。\n"
    "4. コードから読み取れない事項のみ『(要確認)』と明記する。\n"
    "5. ID-200T 仕様書相当の粒度。見出し(#)は付けず本文のみ。前置き・自己言及・英語の結論文は禁止。"
)

APX_SYSTEM = (
    "あなたは日本の計測器メーカーの技術ライターで、C言語と表計算の校正資料を読める。"
    "与えられたソース/資料から、計算式・補正式・データ割付を日本語(である調)で整理する。"
    "英文への切替禁止。数式は資料・コードに現れる形で正確に記す。推測で係数や式を創作せず、"
    "不明点は『(要確認)』と明記。見出し(#)は付けず本文のみ。"
)


def read(p: Path) -> str:
    return p.read_text(encoding="utf-8", errors="ignore") if p.exists() else ""


def section_inputs(sec: dict) -> list[Path]:
    """The source files whose content defines this section (for hashing + prompt)."""
    if sec.get("kind") == "appendix":
        return [ROOT / s for s in sec["sources"]]
    files = []
    for fn in sec["functions"]:
        files.append(CODE / f"{fn}.c")
        c = CALLER / f"{fn}.c"
        if c.exists():
            files.append(c)
    return files


def input_hash(sec: dict) -> str:
    h = hashlib.sha256(PROMPT_VERSION.encode())
    h.update(sec.get("note", "").encode())
    for p in section_inputs(sec):
        h.update(p.name.encode())
        h.update(read(p).encode())
    return h.hexdigest()[:16]


def build_prompt(sec: dict) -> tuple[str, str]:
    if sec.get("kind") == "appendix":
        blocks = "\n\n".join(f"// ===== {Path(s).name} =====\n{read(ROOT / s)}"
                             for s in sec["sources"])
        return APX_SYSTEM, (
            f"# 付録: {sec['title']}\n# 資料\n```\n{blocks}\n```\n\n{sec['note']}\n"
            f"上記から「{sec['title']}」を日本語(である調)で表・箇条書きに整理してください。")
    alldef = read(V2 / "AllDef_excerpt.h")
    disp = "\n\n".join(f"// ===== {fn} (描画) =====\n{read(CODE / f'{fn}.c')}"
                       for fn in sec["functions"])
    caller = "\n\n".join(read(CALLER / f"{fn}.c") for fn in sec["functions"]
                         if (CALLER / f"{fn}.c").exists()) or "// (呼出側 case ブロックは自動抽出できず。遷移は要確認)"
    note = f"\n# 補足\n{sec['note']}\n" if sec.get("note") else ""
    return SYSTEM, (
        f"{FACTS}\n# 状態定義（AllDef.h 抜粋 / operation_mode）\n```c\n{alldef[:4000]}\n```\n\n"
        f"# 執筆対象画面: {sec['num']} {sec['title']}\n"
        f"# 描画コード（Display.c）\n```c\n{disp}\n```\n\n"
        f"# 呼出側 状態遷移・スイッチ処理（Normal.c/Setting.c/Adjust.c の該当 case）\n```c\n{caller}\n```\n{note}\n"
        f"画面「{sec['title']}」の動作仕様を、表示要素・状態(operation_mode)ごとの遷移・"
        f"記録/履歴スイッチ操作の効果を含めてID-200T相当の粒度・である調で日本語で書いてください。")


def ollama(system: str, prompt: str) -> dict:
    body = json.dumps({"model": MODEL, "system": system, "prompt": prompt, "stream": False,
                       "options": {"temperature": 0.2, "num_ctx": NUM_CTX}}).encode()
    req = urllib.request.Request(f"http://{HOST}/api/generate", data=body,
                                 headers={"Content-Type": "application/json"})
    last = None
    for attempt in range(1, RETRIES + 1):
        try:
            with urllib.request.urlopen(req, timeout=TIMEOUT) as r:
                return {"ok": True, "attempt": attempt, **json.loads(r.read().decode())}
        except Exception as e:  # noqa: BLE001
            last = e
            print(f"      ! attempt {attempt}/{RETRIES}: {e}")
            time.sleep(5 * attempt)
    return {"ok": False, "attempt": RETRIES, "error": repr(last)}


def log(rec: dict) -> None:
    with (OUT / "gen_v3_log.jsonl").open("a", encoding="utf-8") as f:
        f.write(json.dumps(rec, ensure_ascii=False) + "\n")


def load_sections() -> list[dict]:
    body = json.loads((V2 / "sections_v2.json").read_text(encoding="utf-8"))["sections"]
    appendices = [
        {"id": "APX_A", "num": "付録A", "title": "計算式・補正式", "kind": "appendix",
         "sources": ["Calc.c", "corr_xlsx_text.txt"],
         "note": "本体 Calc.c の水温補正等の計算と、補正式作成手順(xlsx)の校正・補正式(スパン校正係数・1次演算式・吸光度/透視度式)を整理する。"},
        {"id": "APX_B", "num": "付録B", "title": "EEPROM割付表", "kind": "appendix",
         "sources": ["Eeprom.h"],
         "note": "Eeprom.h の EEP_*_PAGE 等のページ割付(ヘッダ/基板調整/界面/共通/MLSS/SS/TR校正係数/ロガー等)を、ページ番号と用途の表に整理する。"},
    ]
    return body + appendices


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--only", default="")
    ap.add_argument("--changed-only", action="store_true",
                    help="regenerate only sections whose input source changed")
    ap.add_argument("--dry-run", action="store_true", help="with --changed-only: report only")
    ap.add_argument("--force", action="store_true")
    args = ap.parse_args()

    OUT.mkdir(parents=True, exist_ok=True)
    sections = load_sections()
    only = {s.strip() for s in args.only.split(",") if s.strip()}
    hashes_path = OUT / "section_hashes.json"
    old_hashes = json.loads(hashes_path.read_text()) if hashes_path.exists() else {}
    new_hashes = dict(old_hashes)

    # decide work set
    todo = []
    for s in sections:
        if only and s["id"] not in only:
            continue
        h = input_hash(s)
        new_hashes[s["id"]] = h
        frag = OUT / f"frag_{s['id']}.md"
        changed = old_hashes.get(s["id"]) != h or not frag.exists()
        if args.changed_only and not args.force:
            if changed:
                todo.append((s, h))
        else:
            todo.append((s, h))

    if args.changed_only:
        print(f"[incremental] {len(todo)} section(s) changed/new: "
              f"{[s['id'] for s, _ in todo]}")
        if args.dry_run:
            return 0

    print(f"gen_v3: host={HOST} model={MODEL} num_ctx={NUM_CTX} "
          f"todo={len(todo)}/{len(sections)} cooldown={SLEEP_BETWEEN}s")
    ok = fail = 0
    for i, (sec, h) in enumerate(todo, 1):
        sid = sec["id"]
        system, prompt = build_prompt(sec)
        (OUT / f"prompt_{sid}.txt").write_text(prompt, encoding="utf-8")
        print(f"[{i}/{len(todo)}] {sid} {sec['num']} {sec['title']} ({len(prompt)} chars) ...")
        t0 = time.time()
        try:
            r = ollama(system, prompt)
            dt = time.time() - t0
            if r.get("ok") and r.get("response", "").strip():
                (OUT / f"frag_{sid}.md").write_text(r["response"].strip(), encoding="utf-8")
                new_hashes[sid] = h  # commit hash only on success
                ok += 1
                print(f"      OK {r.get('eval_count')} tok in {dt:.1f}s "
                      f"(prompt {r.get('prompt_eval_count')} tok)")
                log({"id": sid, "title": sec["title"], "ok": True, "hash": h,
                     "prompt_chars": len(prompt), "prompt_tok": r.get("prompt_eval_count"),
                     "eval_tok": r.get("eval_count"), "sec": round(dt, 1), "attempts": r["attempt"]})
            else:
                fail += 1
                print(f"      FAIL: {r.get('error') or 'empty'}")
                log({"id": sid, "title": sec["title"], "ok": False,
                     "error": r.get("error") or "empty", "sec": round(dt, 1)})
        except Exception as e:  # noqa: BLE001
            fail += 1
            print(f"      EXCEPTION: {e}")
            log({"id": sid, "title": sec["title"], "ok": False,
                 "error": repr(e), "traceback": traceback.format_exc()})
        hashes_path.write_text(json.dumps(new_hashes, ensure_ascii=False, indent=2))
        if SLEEP_BETWEEN > 0 and i < len(todo):
            print(f"      cooldown {SLEEP_BETWEEN:g}s")
            time.sleep(SLEEP_BETWEEN)

    print(f"gen_v3: done. ok={ok} fail={fail}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
