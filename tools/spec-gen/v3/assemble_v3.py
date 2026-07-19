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
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent
V1OUT = ROOT.parent / "out"
V2 = ROOT.parent / "v2"
REPO = ROOT.parents[2]
SPECS = REPO / "docs" / "specs"
OUT = ROOT / "out"
DEST = SPECS / "IM-110プログラム仕様書.md"

# per-section screen figures (each state/mode shown). (image_id, caption)
IMG_MAP = {
    "S01": [("meas_01", "起動画面")],
    "S02": [("meas_06", "測定メニュー（測定モード/相関式/校正/設定）"), ("meas_07", "相関式選択")],
    "S04": [("meas_05", "測定モード選択（MLSS/SS/透視度/界面）")],
    "S05": [("meas_02", "MLSS 測定待機"), ("meas_03", "MLSS 測定中"),
            ("meas_08", "SS 測定待機"), ("meas_09", "SS 測定中"),
            ("meas_11", "透視度 測定待機"), ("meas_12", "透視度 測定中"),
            ("meas_14", "界面 測定待機"), ("meas_15", "界面 測定中")],
    "S10": [("calib_02", "MLSS 校正メニュー"), ("calib_16", "SS 校正メニュー"),
            ("calib_30", "透視度 校正メニュー")],
    "S13": [("calib_07", "ゼロ校正 待機"), ("calib_08", "ゼロ校正 中"),
            ("calib_09", "校正完了"), ("calib_10", "ゼロ校正 値表示")],
    "S14": [("calib_11", "3点校正 待機"), ("calib_12", "3点校正 値"),
            ("calib_13", "3点校正 中"), ("calib_14", "校正完了")],
    "S15": [("calib_03", "2点校正 待機"), ("calib_04", "2点校正 中"), ("calib_05", "校正完了")],
    "S20": [("meas_04", "測定履歴（MLSS）"), ("meas_10", "測定履歴（SS）"),
            ("meas_13", "測定履歴（透視度）"), ("meas_16", "測定履歴（界面）")],
    "S21": [("settings_01", "設定メニュー"), ("settings_05", "製品情報")],
    "S25": [("settings_02", "時刻設定")],
    "S30": [("settings_03", "初期化 確認"), ("settings_04", "初期化 完了")],
}


def unwrap_prose_fences(text: str) -> str:
    """Devstral sometimes wraps a whole prose block (headings/lists/tables) in a
    ``` code fence, which then renders as literal monospace. Drop the fence
    markers around any block that contains markdown structure; keep genuine
    code/formula fences (whose content is plain text with no #/-/|/1. lines)."""
    lines = text.splitlines()
    # find fenced blocks
    out = []
    i = 0
    struct = re.compile(r"^\s*(#|[-*]\s|\d+\.\s|\|)")
    while i < len(lines):
        if lines[i].strip().startswith("```"):
            j = i + 1
            while j < len(lines) and not lines[j].strip().startswith("```"):
                j += 1
            block = lines[i + 1:j]
            if any(struct.match(b) for b in block):   # mis-wrapped prose -> unwrap
                out.extend(block)
            else:                                       # real code/formula fence -> keep
                out.append(lines[i])
                out.extend(block)
                if j < len(lines):
                    out.append(lines[j])
            i = j + 1
        else:
            out.append(lines[i])
            i += 1
    return "\n".join(out)


def delatex(t: str) -> str:
    """Convert the LaTeX math Devstral insists on emitting into plain text that
    renders identically in any markdown viewer / print (their pipeline has no
    MathJax). Idempotent; safe on text with no LaTeX."""
    t = t.replace("$$", "").replace("$", "")
    for _ in range(4):  # \text{...} may nest / repeat on a line
        t = re.sub(r"\\text\{([^{}]*)\}", r"\1", t)
    t = t.replace("\\times", "*").replace("\\cdot", "*").replace("\\div", "/")
    for _ in range(3):  # resolve \frac after \text so braces are simple
        t = re.sub(r"\\frac\{([^{}]*)\}\{([^{}]*)\}", r"(\1) / (\2)", t)
    for tok in ("\\left", "\\right", "\\,", "\\;", "\\!", "\\[", "\\]"):
        t = t.replace(tok, "")
    # strip display-math bracket wrappers "[ ... ]" (space-guarded so mV[0] survives)
    t = re.sub(r"\[\s+", "", t)
    t = re.sub(r"\s+\]", "", t)
    return t
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


def img_block(sid: str) -> str:
    out = []
    for iid, cap in IMG_MAP.get(sid, []):
        if not (SPECS / f"assets/im-110/{iid}.png").exists():
            continue
        out.append(f"\n**{cap}**  \n![{iid}](assets/im-110/{iid}.png)  \n"
                   f"<sub>画面図 {iid} — BMP版: [{iid}.bmp](assets/im-110/{iid}.bmp)</sub>\n")
    return "".join(out)


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
        L.append(img_block(s["id"]))
        L.append("")

    L.append("\n---\n\n# セクションB：プローブ\n")
    L.append("> プローブは画面を持たず、本体からRS-232Cで制御される。記述は `protocol-rs232c.md` に基づく。\n")
    for num, title, fid in PROBE:
        L.append(f"\n## B-{num} {title}\n")
        f = V1OUT / f"final_{fid}.md"
        L.append(strip_heading(f.read_text(encoding="utf-8")) if f.exists() else "_(未生成)_")
        L.append("")

    L.append("\n---\n\n# 付録\n")
    for sid, num, title in [("APX_A", "付録A", "計算式・補正式"), ("APX_B", "付録B", "EEPROM割付表"),
                            ("APX_C", "付録C", "相関式・項目別演算")]:
        L.append(f"\n## {num} {title}\n")
        # unwrap mis-fenced prose, drop the redundant leading title it exposes, LaTeX -> plain
        L.append(delatex(strip_heading(unwrap_prose_fences(frag(sid)))))
        L.append("")

    DEST.write_text("\n".join(L) + "\n", encoding="utf-8")
    done = sum(1 for s in body if (OUT / f"frag_{s['id']}.md").exists())
    apx = sum(1 for a in ("APX_A", "APX_B") if (OUT / f"frag_{a}.md").exists())
    print(f"assembled {DEST.relative_to(REPO)} — body {done}/{len(body)}, appendix {apx}/2, probe {len(PROBE)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
