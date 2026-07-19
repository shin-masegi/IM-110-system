#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
assemble_v3.py — merge v3 body (code+caller derived) + appendices + probe section.

Body (A)   = v3/out/frag_S*.md  (Display.c + caller state-machine → transitions)
Appendix   = v3/out/frag_APX_*.md (計算・補正式 / EEPROM割付表)
Probe (B)  = ../out/final_B*.md  (regenerated Japanese-only in run_v3.sh; ① fix)

Writes docs/specs/IM-110プログラム仕様書.md. Missing sections shown explicitly.
"""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent
V1OUT = ROOT.parent / "out"
V2 = ROOT.parent / "v2"
REPO = ROOT.parents[2]
SPECS = REPO / "docs" / "specs"
OUT = ROOT / "out"
DEST = SPECS / "IM-110プログラム仕様書.md"

IMG = {"S01": "meas_01", "S05": "meas_04", "S06": "meas_05", "S20": "meas_14",
       "S10": "calib_01", "S15": "calib_10", "S21": "settings_01"}
PROBE = [("1", "プローブ概要・役割", "B00"), ("2", "ハードウェア構成（電源・MCU）", "B01"),
         ("3", "計測フロントエンド（アナログ信号系）", "B02"), ("4", "測定チャネルとMD応答", "B03"),
         ("5", "RS-232Cコマンド仕様", "B04"), ("6", "ゼロ点・スパン補正（SADZ/SADS）", "B05"),
         ("7", "補正式・調整パラメータ", "B06"), ("8", "エラー処理・タイムアウト", "B07")]


def strip_heading(text: str) -> str:
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


def frag(sid: str) -> str:
    p = OUT / f"frag_{sid}.md"
    return strip_heading(p.read_text(encoding="utf-8")) if p.exists() else "_(未生成/失敗: 要再実行)_"


def img(sid: str) -> str:
    if sid not in IMG:
        return ""
    iid = IMG[sid]
    if not (SPECS / f"assets/im-110/{iid}.png").exists():
        return ""
    return (f"\n![{iid}](assets/im-110/{iid}.png)\n"
            f"<sub>画面図 {iid} — BMP版: [{iid}.bmp](assets/im-110/{iid}.bmp)</sub>\n")


def main() -> int:
    body = json.loads((V2 / "sections_v2.json").read_text(encoding="utf-8"))["sections"]
    L = ["# IM-110 プログラム仕様書\n",
         "> IM-110T 本体・プローブ両基板のプログラム動作仕様。**セクションA=本体、B=プローブ、末尾に付録。**\n",
         "> **作成方法**: 本体(A)は 77サーバーの Devstral(24B) が本体FWの描画コード `Display.c` と"
         "呼出側 状態遷移 `Normal.c`/`Setting.c`/`Adjust.c` を読み、実装から画面仕様・遷移を執筆。"
         "付録は `Calc.c`／補正式手順／`Eeprom.h` から生成。プローブ(B)は `protocol-rs232c.md` から生成。"
         "Claude Code が真実源と突き合わせ検証。文体はである調に統一。\n",
         "> 画面図はモノクロLCD(400×240)。全図は `docs/specs/assets/im-110/`。\n",
         "\n---\n\n# セクションA：IM-110T 本体\n"]
    for s in body:
        L.append(f"\n## {s['num']} {s['title']}\n")
        L.append(f"<sub>実装: 描画 `{'`, `'.join(s['functions'])}` / 遷移 呼出側 case</sub>\n")
        L.append(frag(s["id"]))
        L.append(img(s["id"]))
        L.append("")

    L.append("\n---\n\n# セクションB：プローブ\n")
    L.append("> プローブは画面を持たず、本体からRS-232Cで制御される。記述は `protocol-rs232c.md` に基づく。\n")
    for num, title, fid in PROBE:
        L.append(f"\n## B-{num} {title}\n")
        f = V1OUT / f"final_{fid}.md"
        L.append(strip_heading(f.read_text(encoding="utf-8")) if f.exists() else "_(未生成)_")
        L.append("")

    L.append("\n---\n\n# 付録\n")
    for sid, num, title in [("APX_A", "付録A", "計算式・補正式"), ("APX_B", "付録B", "EEPROM割付表")]:
        L.append(f"\n## {num} {title}\n")
        L.append(frag(sid))
        L.append("")

    DEST.write_text("\n".join(L) + "\n", encoding="utf-8")
    done = sum(1 for s in body if (OUT / f"frag_{s['id']}.md").exists())
    apx = sum(1 for a in ("APX_A", "APX_B") if (OUT / f"frag_{a}.md").exists())
    print(f"assembled {DEST.relative_to(REPO)} — body {done}/{len(body)}, appendix {apx}/2, probe {len(PROBE)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
