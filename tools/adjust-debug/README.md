# IM-110 調整機能 実機デバッグ ハーネス

本体 調整機能A（シリアル調整インターフェース）を使い、**計算式を実機で検証・デバッグ**するための道具一式。
Claude Code が OpenOCD/GDB とシリアル(A コマンド)の**両方を駆動**する前提。

- `openocd_l452.cfg` — STM32L452 + ST-Link の OpenOCD 設定
- `adj.gdb` — GDB 初期化（観測マクロ + fit/adjust ブレーク）
- `adjctl.py` — 依存なしシリアルドライバ（A コマンド送受、115200 8N1 LF）
- 設計の真実源: `../../docs/adjust-mode-design.md`（★A 節にコマンド表）

## 接続
- **ST-Link** → 本体 **CN1**（SWD/JTAG, CON8）
- **USB-シリアル** → 本体 **CN2**（USART1 引き出し, 115200 8N1）
- **プローブ** → 本体 **CN4**（給電＋RS-232C）。プローブ単独給電は不可

## 起動（3 つ）
```bash
# ① OpenOCD サーバ (Claude が別 shell/background で)
openocd -f tools/adjust-debug/openocd_l452.cfg

# ② GDB (Claude が駆動)。attach 後 armfit でブレーク配備
arm-none-eabi-gdb ../IM-110/build/IM-110.elf -x tools/adjust-debug/adj.gdb
#   (gdb) armfit
#   (gdb) adcs / mlssv / caps ...

# ③ シリアル (Claude が駆動)。ポートは自動検出 or -p 指定
python3 tools/adjust-debug/adjctl.py -i          # 対話
python3 tools/adjust-debug/adjctl.py AMV         # 単発
```

## デバッグの噛み方（計算式）
1. GDB で `armfit` → `adj_fit_poly` / `Adj_*` にブレーク。
2. シリアルで `AMR` → `AMC,<mgL>` を数点 → `AMD`（捕捉確認）→ `AMCF,<次数>`。
3. `AMCF` 送信で **GDB が `adj_fit_poly` で停止** → `p adj_x` / `p adj_y` / step で S,T,Gauss を追う。
4. 返った係数 `coef` と R² を、`AMD` ダンプ値でホスト再計算（`scratchpad/test_fit.c` 系）と突合。
5. `mlssv`/`ssv`/`trv` で live 反映と vault staging を確認。ホットパスは `stepmlss` で 1 サイクル捕捉し
   `I(0)`/`abss`/`fabss` を step。

> A コマンドは `CheckSerialCMD`（main ループ）で実行。fit/vault 書込中はブロッキングだが、
> ブレークで停止しても測定ループが止まるだけで安全（プローブ MS は本体側でポーリング）。

## 調整シーケンス（xlsx 手順1–9 → A コマンド）
> モードは `AMODE,<0/1/2>`（MLSS/SS/TR）で切替、MLSS の相関式は `AEQ,<21-30>`。各段の確定は
> 最後にまとめて `AWC`（WCFC）でも、段ごとでも可。**新品プローブ前提**。

| # | 手順 | コマンド列 |
|---|---|---|
| 1 | LED エージング | （通電待ち。コマンドなし） |
| 2 | duty 調整（空中 1750±30mV） | `ALDA,0.36` → `ALDA,OK,duty:..,max:..` を確認（WPP 自動保存）|
| 3 | ADZR 取得（遮光板挿入） | `AMODE,0`→`AZR` / `AMODE,1`→`AZR`（受光ダーク→CZ）|
| 4 | Ref 温度係数（5/20/35℃ 清水） | `AMODE,0` 各温度で `ATC,5`/`ATC,20`/`ATC,35`→`ATCF`。SS も `AMODE,1` で同様 |
| 4-1 | MLSS 黒 1次（8000 2点） | `AMODE,0`→`AEQ,21`→`AMR`→`AMC,<mgL>`×2→`AMCF,1`（R²確認）|
| 4-2 | MLSS 白 4次（500–8000 6点） | `AEQ,22`→`AMR`→`AMC,<mgL>`×6→`AMCF,4`（R²確認）|
| 5 | MLSS 機差 | 基準器溶液で `AKZ,<基準器mgL>` |
| 6 | SS 1次（黒2点/白6点） | `AMODE,1`→`AMR`→`AMC,<mgL>`×n→`AMCF,<1 or 4>` |
| 7 | SS 機差 | `AKZ,<mgL>` |
| 8 | 透視度 累乗（2–100cm 6点） | `AMODE,2`→`AMR`→`AMC,<cm>`×6→`AMCP`（R²確認）|
| 9 | 透視度 器差 | `AKZ,<cm>` |
| — | 確定 | `AWC`（プローブ flash へ WCFC）|

途中確認: `AMV`（生値/ABS/FABSS/最終）, `AST`（mode/eq/点数）, `AMD`（捕捉点）。

## 既知の前提・注意
- `ATCF` は refZR(adzr[1]) と B(tempc[0]) を書く。**受光ダーク adzr[0] は `AZR` が書く**（両者は WCZ で
  相手フィールドを保持）。ZR（清水受光_20）は別途ゼロ/清水校正が `MLSS_ZR/SS_ZR` に設定する前提。
- `ALDA` の開始 duty は引数（本体はプローブ現 duty を保持しない）。製造直後 ≒ 0.36。
- `adj.gdb` の `mlssv` は `MLSS_MODE≥20`（調整対象 No.21-30）前提で vault 添字を計算。
- 静的グローバル（`adj_n` 等）が GDB で曖昧なら `p 'IM_110.c'::adj_n` と file 修飾。
