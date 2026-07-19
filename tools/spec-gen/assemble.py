#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
assemble.py — merge generated fragments into the single spec markdown.

Reads sections.json + out/final_<id>.md (falls back to out/frag_<id>.md) and
the image manifest, then writes docs/specs/IM-110プログラム仕様書.md with:
  - a title + auto ToC
  - Section A (本体) and Section B (プローブ) with ## headings
  - screen images embedded as PNG (inline) with a BMP link beside each

Usage:  python3 tools/spec-gen/assemble.py
        python3 tools/spec-gen/assemble.py --raw   # use frag_ (pre-review) fragments
"""
import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent
REPO = ROOT.parents[1]
SPECS = REPO / "docs" / "specs"
OUT = ROOT / "out"
DEST = SPECS / "IM-110プログラム仕様書.md"

SECTION_TITLES = {"A": "セクションA：IM-110T 本体", "B": "セクションB：プローブ"}


def load_images() -> dict:
    manifest = json.loads(
        (ROOT / "context" / "image_manifest.json").read_text(encoding="utf-8"))
    return {m["id"]: m for m in manifest}


def strip_leading_heading(text: str) -> str:
    """Drop a stray leading '#'-heading the model added; assemble.py owns headings."""
    lines = text.lstrip().splitlines()
    while lines and (lines[0].lstrip().startswith("#") or not lines[0].strip()):
        lines.pop(0)
    return "\n".join(lines).strip()


def frag_text(sid: str, raw: bool) -> str:
    if not raw:
        p = OUT / f"final_{sid}.md"
        if p.exists():
            return strip_leading_heading(p.read_text(encoding="utf-8"))
    p = OUT / f"frag_{sid}.md"
    if p.exists():
        return strip_leading_heading(p.read_text(encoding="utf-8"))
    return "_(未生成)_"


def image_block(ids: list, images: dict) -> str:
    out = []
    for iid in ids:
        m = images.get(iid)
        if not m:
            out.append(f"_(画像 {iid} が見つかりません)_")
            continue
        # paths in the manifest are relative to docs/specs/, same dir as DEST
        out.append(f"![{iid}]({m['png']})  \n"
                   f"<sub>画面図 {iid} — BMP版: [{Path(m['bmp']).name}]({m['bmp']})</sub>")
    return "\n\n".join(out)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--raw", action="store_true", help="use pre-review fragments")
    args = ap.parse_args()

    data = json.loads((ROOT / "sections.json").read_text(encoding="utf-8"))
    sections = data["sections"]
    images = load_images()

    lines: list = []
    lines.append(f"# {data['meta']['title']}\n")
    lines.append("> 本仕様書は IM-110T 本体基板とプローブ基板のプログラム動作仕様を"
                 "1ファイルにまとめたものである。セクションA=本体、セクションB=プローブ。\n")
    lines.append("> 画面図はモノクロLCD(400×240)のモックアップ。PNGでインライン表示し、"
                 "BMP版へのリンクを併記する（GitHub上はPNG、VSCodeプレビューはBMPも表示可）。\n")

    # ToC
    lines.append("\n## 目次\n")
    for sec_key in ("A", "B"):
        lines.append(f"\n**{SECTION_TITLES[sec_key]}**\n")
        for s in sections:
            if s["section"] == sec_key:
                anchor = f"{s['num']}-{s['title']}".replace(" ", "-")
                lines.append(f"- {s['num']} {s['title']}")
    lines.append("\n---\n")

    for sec_key in ("A", "B"):
        lines.append(f"\n# {SECTION_TITLES[sec_key]}\n")
        for s in sections:
            if s["section"] != sec_key:
                continue
            lines.append(f"\n## {s['num']} {s['title']}\n")
            lines.append(frag_text(s["id"], args.raw))
            if s.get("images"):
                lines.append("\n" + image_block(s["images"], images))
            lines.append("")

    DEST.write_text("\n".join(lines) + "\n", encoding="utf-8")
    n_final = sum(1 for s in sections if (OUT / f"final_{s['id']}.md").exists())
    print(f"assembled {DEST.relative_to(REPO)} "
          f"({len(sections)} sections, {n_final} finalized) raw={args.raw}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
