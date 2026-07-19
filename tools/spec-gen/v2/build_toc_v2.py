#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
build_toc_v2.py — emit sections_v2.json (the granular ToC for v2).

MANAGER role only: this maps each real screen function in Display.c to a spec
subsection, mirroring ID-200T's per-screen granularity. It does NOT describe
behavior — Devstral reads each function's source and writes the spec.

Groups are validated against v2/func_index.json so a typo'd/renamed function
is caught here rather than silently dropped.

Run:  python3 tools/spec-gen/v2/build_toc_v2.py
"""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent

# (num, title, [functions], note-for-devstral)
# note = optional extra hint routed to Devstral (kept minimal; the CODE is the source of truth)
GROUPS = [
    ("5.1",  "起動画面", ["disp_STRTDSP", "disp_STRTDSP2"],
     "電源ON直後のロゴ/バージョン表示と操作説明表示。表示要素と遷移をコードから読む。"),
    ("5.2",  "測定メニュー", ["disp_M_MENU"], "測定項目の選択メニュー。"),
    ("5.3",  "モード切替", ["disp_M_CHANGE"], "測定/ロガー等のモード切替画面。"),
    ("5.4",  "測定モード設定", ["disp_MEASMODE"], ""),
    ("5.5",  "MLSS測定画面（待機・測定中）", ["disp_MLSS_Meas_2", "disp_MLSS_Meas_3"],
     "測定待機と測定中。表示値・水温・BTアイコン・点滅の有無をコードから読む。"),
    ("5.6",  "MLSS測定完了画面", ["disp_MLSS_Meas_4"], ""),
    ("5.7",  "測定レンジ表示", ["disp_RANGE1", "disp_RANGE2"],
     "op_mode/range_mv 引数からレンジ表示の切替を読む。"),
    ("5.8",  "水温表示", ["disp_WAT1", "disp_WAT2"], ""),
    ("5.9",  "界面閾値設定", ["disp_Depth_Setting"], "界面測定の閾値設定画面。"),
    ("6.1",  "校正メニュー", ["disp_C_MENU"], "calmode 引数で校正対象が変わる点を読む。"),
    ("6.2",  "校正設定", ["disp_CAL_SETTING"], ""),
    ("6.3",  "補正設定", ["disp_Corr_setting", "disp_Corr_setting_30"], ""),
    ("6.4",  "MLSSゼロ校正", ["disp_MLSS_ZCal_2", "disp_MLSS_ZCal_3"], ""),
    ("6.5",  "MLSS中間校正", ["disp_MLSS_MCal_2", "disp_MLSS_MCal_3"], ""),
    ("6.6",  "MLSSスパン校正", ["disp_MLSS_SCal_1", "disp_MLSS_SCal_2", "disp_MLSS_SCal_3"],
     "校正値入力(SCal_1)→校正中(SCal_2)→完了(SCal_3)の流れを読む。"),
    ("6.7",  "AD校正", ["disp_ADCAL1", "disp_ADCAL2", "disp_ADCAL3"],
     "基板調整のAD校正。fdo/ddo/temp 引数の意味をコードから読む。"),
    ("6.8",  "自動校正完了", ["disp_AUTOCAL_COMP"], ""),
    ("6.9",  "校正リセット", ["disp_C_RESET"], ""),
    ("6.10", "後校正 記録選択", ["disp_CAL_HSEL"], ""),
    ("6.11", "校正履歴表示", ["disp_DISPCAL"], ""),
    ("5.10", "測定履歴表示", ["disp_DISPHIS"], ""),
    ("7.1",  "設定メニュー", ["disp_S_MENU"], ""),
    ("7.2",  "自動校正設定", ["disp_ACALSET"], ""),
    ("7.3",  "淡水/海水設定", ["disp_TANSUI"], "IM-110に実在する画面。コードの表示・選択を読む。"),
    ("7.4",  "アプリDL先表示", ["disp_APPDL"], ""),
    ("7.5",  "時刻設定（入口）", ["disp_SETTIME"], ""),
    ("7.6",  "年設定", ["disp_SETYEAR"], ""),
    ("7.7",  "月日設定", ["disp_SETDAYS"], ""),
    ("7.8",  "時分設定", ["disp_SETHOUR"], ""),
    ("7.9",  "水温校正", ["disp_CALTEMP"], ""),
    ("7.10", "初期化", ["disp_RESET"], ""),
    ("7.11", "BT初期化", ["disp_BT_RESET"], ""),
    ("7.12", "QRコード表示", ["disp_QR"], ""),
    ("8.1",  "基板調整：プログラムVer表示", ["disp_PRGVER"], ""),
    ("8.2",  "基板調整：EEPROM確認", ["disp_EEP1", "disp_EEP2", "disp_EEP3"], ""),
    ("8.3",  "基板調整：電池電圧確認", ["disp_BATVOL"], ""),
    ("9.1",  "ガイダンス表示", ["disp_GUIDE1", "disp_GUIDE2", "disp_GUIDE3", "disp_GUIDE4", "disp_GUIDE5"],
     "5種のガイダンス画面。各々の表示内容をコードから読む。"),
    ("10.1", "エラー表示（共通）", ["disp_ERROR"], "err_num 引数でエラー種別が変わる共通表示。"),
    ("10.2", "エラー画面 1〜9", [f"disp_ERROR{i}_{p}" for i in range(1, 10) for p in (1, 2)],
     "エラー1〜9それぞれの1画面目/2画面目。各エラーの表示内容と処置手順をコードから読む。"),
    ("10.3", "エラー画面 17/19", ["disp_ERROR17_1", "disp_ERROR17_2", "disp_ERROR19_1", "disp_ERROR19_2"], ""),
    ("11.1", "省電力測定中画面", ["disp_ECOMODE"], ""),
    ("11.2", "ロガー測定画面", ["disp_LOG1", "disp_LOG3", "disp_LOG4", "disp_LOG5"],
     "ロガー測定の待機/測定中/完了/履歴。ID-200T由来の機能。"),
    ("11.3", "ロガー履歴表示", ["disp_DISPLOG"], ""),
]


def main() -> int:
    idx = {f["name"] for f in json.loads((ROOT / "func_index.json").read_text(encoding="utf-8"))}
    sections = []
    missing = []
    for i, (num, title, funcs, note) in enumerate(GROUPS, 1):
        for fn in funcs:
            if fn not in idx:
                missing.append(fn)
        sections.append({
            "id": f"S{i:02d}", "num": num, "title": title,
            "functions": funcs, "note": note,
        })
    if missing:
        print("!! functions not found in Display.c:", missing)
    covered = {fn for s in sections for fn in s["functions"]}
    uncovered = sorted(idx - covered)
    out = {
        "meta": {"title": "IM-110 プログラム仕様書", "source": "IM-110/Core/Src/Display.c",
                 "sections": len(sections), "functions_covered": len(covered),
                 "functions_uncovered": uncovered},
        "sections": sections,
    }
    (ROOT / "sections_v2.json").write_text(
        json.dumps(out, ensure_ascii=False, indent=2), encoding="utf-8")
    print(f"wrote sections_v2.json: {len(sections)} sections, "
          f"{len(covered)}/{len(idx)} functions covered")
    if uncovered:
        print("uncovered functions (intentional or review):", uncovered)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
