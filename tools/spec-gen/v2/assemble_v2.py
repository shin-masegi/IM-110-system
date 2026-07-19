#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
assemble_v2.py — merge v2 body fragments (+ reuse v1 probe section B) into the spec.

Body (section A) = code-derived v2 fragments (out/frag_S*.md).
Probe (section B) = the already-faithful v1 fragments (../out/final_B*.md), which
were generated from the protocol fact pack and needed only light fixes.

Writes docs/specs/IM-110プログラム仕様書.md (v2 = the real deliverable).
Missing/failed sections are shown as _(未生成/失敗: 要再実行)_ so gaps are visible.

Usage:  python3 tools/spec-gen/v2/assemble_v2.py
"""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent
V1OUT = ROOT.parent / "out"
REPO = ROOT.parents[2]
SPECS = REPO / "docs" / "specs"
OUT = ROOT / "out"
DEST = SPECS / "IM-110プログラム仕様書.md"

# a few representative screen figures anchored to sections (full set in assets/)
IMG = {
    "S01": "meas_01", "S05": "meas_04", "S06": "meas_05",
    "S20": "meas_14", "S10": "calib_01", "S15": "calib_10",
    "S21": "settings_01",
}
# v1 probe section B (num, title, v1 fragment id)
PROBE = [
    ("1", "プローブ概要・役割", "B00"), ("2", "ハードウェア構成（電源・MCU）", "B01"),
    ("3", "計測フロントエンド（アナログ信号系）", "B02"), ("4", "測定チャネルとMD応答", "B03"),
    ("5", "RS-232Cコマンド仕様", "B04"), ("6", "ゼロ点・スパン補正（SADZ/SADS）", "B05"),
    ("7", "補正式・調整パラメータ", "B06"), ("8", "エラー処理・タイムアウト", "B07"),
]


def strip_heading(text: str) -> str:
    """Drop a leading model-added heading, and demote any internal headings to
    at least level 3 so they nest under the ## section heading (formatting only)."""
    lines = text.lstrip().splitlines()
    while lines and (lines[0].lstrip().startswith("#") or not lines[0].strip()):
        lines.pop(0)
    out = []
    for ln in lines:
        s = ln.lstrip()
        if s.startswith("#"):
            hashes = len(s) - len(s.lstrip("#"))
            if hashes < 3 and s[hashes:hashes + 1] == " ":
                ln = "###" + s[hashes:]
        out.append(ln)
    return "\n".join(out).strip()


def img_block(iid: str) -> str:
    png = f"assets/im-110/{iid}.png"
    bmp = f"assets/im-110/{iid}.bmp"
    if not (SPECS / png).exists():
        return ""
    return f"\n![{iid}]({png})\n<sub>画面図 {iid} — BMP版: [{iid}.bmp]({bmp})</sub>\n"


def main() -> int:
    data = json.loads((ROOT / "sections_v2.json").read_text(encoding="utf-8"))
    secs = data["sections"]
    L = [f"# {data['meta']['title']}\n",
         "> 本仕様書は IM-110T 本体・プローブ両基板のプログラム動作仕様をまとめる。"
         "**セクションA=本体、セクションB=プローブ。**\n",
         "> **作成方法**: 本体(A)は 77サーバーの Devstral(24B) が本体FW `IM-110/Core/Src/Display.c` の"
         "各画面関数のソースを読み、その実装から画面仕様を執筆したもの。"
         "プローブ(B)は通信仕様 `docs/protocol-rs232c.md` から生成。Claude Code が真実源と突き合わせ検証。\n",
         "> `(要確認: 呼出側FW)` は、画面遷移やスイッチ処理が本コード(描画)ではなく"
         "呼出側 `Normal.c` / `Setting.c` / `Adjust.c` にあり、本ラウンドでは未解析であることを示す。\n",
         "> 画面図はモノクロLCD(400×240)モックアップ。全図は `docs/specs/assets/im-110/`。\n",
         "\n---\n\n# セクションA：IM-110T 本体\n",
         "> 各節は本体FW `Display.c` の画面描画関数（節見出しに関数名を併記）の実装に基づく。\n"]

    for s in secs:
        L.append(f"\n## {s['num']} {s['title']}\n")
        L.append(f"<sub>実装: `{'`, `'.join(s['functions'])}`</sub>\n")
        frag = OUT / f"frag_{s['id']}.md"
        L.append(strip_heading(frag.read_text(encoding="utf-8")) if frag.exists()
                 else "_(未生成/失敗: 要再実行)_")
        if s["id"] in IMG:
            L.append(img_block(IMG[s["id"]]))
        L.append("")

    L.append("\n---\n\n# セクションB：プローブ\n")
    L.append("> プローブは画面を持たない。本体からRS-232Cで制御されるアナログ計測モジュール。"
             "記述は `docs/protocol-rs232c.md` に基づく。\n")
    for num, title, fid in PROBE:
        L.append(f"\n## B-{num} {title}\n")
        f = V1OUT / f"final_{fid}.md"
        L.append(strip_heading(f.read_text(encoding="utf-8")) if f.exists() else "_(未生成)_")
        L.append("")

    DEST.write_text("\n".join(L) + "\n", encoding="utf-8")
    done = sum(1 for s in secs if (OUT / f"frag_{s['id']}.md").exists())
    print(f"assembled {DEST.relative_to(REPO)} — body {done}/{len(secs)} sections, probe {len(PROBE)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
