# 引き継ぎ (別PCで pull して再開する人向け)

> 最終更新: 2026-07-21 / このファイルは「今どうなっているか」を1枚で把握するための現状メモ。
> 詳細仕様の真実源は各 `docs/*.md`・コード本体。ここは索引＋現状スナップショット。

## 0. まず pull する3リポと現在の HEAD

| リポ | HEAD | 内容 |
|---|---|---|
| `IM-110`（本体, STM32L452） | `c8f0f85` [WIP] feat(adjust): ADBOAD 調整機能 本体側 | 計算式再設計＋ADBOAD＋UART |
| `IM-110_Probe`（プローブ, STM32G070） | `4069e57` feat(debug): MDR/RPF 追加 (Ver.0.18) | span傾き正規化＋MDR/RPF |
| `IM-110-system`（本リポ） | `7ad7c56` docs(protocol): MDR/RPF §3.2.7 | 仕様・ログ・ハーネス |

いずれも `main`・push済み・clean。**3リポとも同時に pull すること**（通信・計算は両側整合が前提）。

## 1. 真実源ドキュメント（最初に読む順）

1. `docs/mlss-calc-reference.md` — MLSS/SS/TR の調整・校正・測定・パラメータ記憶の**全設計**（真実源）。
2. `docs/protocol-rs232c.md` — 本体⇔プローブ通信（プローブコマンド SADZ/SADS/MDR/RPF 等、§3.2.7=MDR/RPF）。
3. `docs/protocol-adjust-uart.md` — 本体 CN2 の A系デバッグコマンド一覧。
4. `docs/probe-flash-map.md` — プローブ flash 配置（PARAM/VAULT の2領域）＋VAULT通信不安定の切り分け結果。
5. `../IM-110/CLAUDE.md §7` — 本体側 作業中の項目（タスクB/C/D）と未実装残件。

## 2. 現在の測定モデル（現行実装＝1700系・span傾き）

- **プローブ正規化**: `出力mV = 生mV × ADC_SPAN_INI(1700) / ADC_Span`（span傾き正規化）。空中出力ターゲット **1700mV**。
  ※ 一時期 ADZero オフセット方式にしていたが、現行は **span傾き＋SADS 能動化**（`ADC_Zero` オフセットは撤去、`ADC_Span` を使う）。
- **本体計算**: `I(0)=MLSSZR+(Ref−refZR)(B+B2·…)`（Ref温度補正2次）→ `ABS=log10(I(0)/Y0)`（暗時ADZR廃止）
  → ベース（MLSS/SS=出荷時2次式×k_No / TR=累乗）→ スパン校正 `SP_A·f²+SP_B·f+SP_C`。**機差補正 kiza は廃止**。
- **ゼロ基準**: `INI_*_ZR = 1700`（デフォルト）。実値は **AZC（清水ゼロ校正）が上書き**。
- 詳細と式は `docs/mlss-calc-reference.md` を参照（ここでは繰り返さない）。

## 3. 調整/デバッグの入口（全て UART で駆動可能）

CN2（本体 USART1, 9600 8N1）へ A系コマンド:

| 目的 | コマンド |
|---|---|
| LED PWM 調整（空中1700収束） | `ALDA,<開始duty>` |
| MLSS AD0（空中AD調整, SADS発行） | ADBOAD操作 / `SADS` 系 |
| 清水ゼロ校正（→ *_ZR 上書き） | `AZC` |
| 相関式初期化（No.21-30←出荷時） | `ACRI` |
| Ref温度補正（5/20/35℃→2次） | `ATC,<temp>` ×3 → `ATCF` |
| 出荷時3点調整（Mode_CF フィット） | `AMC,<基準器値>` → `AMCF,<次数>` |
| 水深6m スパン校正 | `AWD6` |
| 確定（flash） | `AWC` |
| 生AD取得（duty調整用, プローブ直） | `MDR`（プローブ Ver.0.18） |
| プローブ新品化リセット（破壊的） | `RPF`（プローブ Ver.0.18） |

- **ADBOAD 画面**: 本体側配線は c8f0f85 で入っているが**仮レベル**（タイトル流用・値表示未整備）。まず UART で駆動確認するのが早い。
- 調整順序: **LED PWM → AD調整 → Ref温度補正 → 出荷時3点調整**（`mlss-calc-reference §8`）。
  → これを一通りやるまで測定値は暫定（出荷時2次式データ未取得のため）。

## 4. フラッシュ配置（重要: まだ2領域のまま）

- プローブ flash は **PARAM (Page62, 0x0801F300) と VAULT (Page63, 0x0801F800) の2領域**構成のまま。
- `mlss-calc-reference §12` の **32B共通レコードによる1領域統合・本体EEPROMミラー・3層同期は未実装**（後日）。
  現状は既存の `read_Param`/`write_Param`（PARAM）＋ `vault_load`/`vault_commit`（VAULT）の別々機構で動く。
- VAULT 通信は不安定要因として切り分け済み（`probe-flash-map.md` 参照）。現状 **本体側 VAULT アクセスは起動時 暫定無効化**中（`IM-110` `bff0812`）。

## 5. 実機の状態（書込み履歴）

- 両基板とも新FWを実機へ書込済み（プローブ=FUP経由 / 本体=ST-Link）。
- ただし**書込んだ版と現HEADの厳密一致は保証しない**。再開時は **current HEAD から rebuild して両基板へ書き直す**のが安全。
  - 本体: `cd ../IM-110 && make -j` → `st-flash --format ihex write build/IM-110.hex`
    - ⚠ **本体は STOP2 で寝ると SWD 接続不可**。かつ **NRST でリセットすると電源が落ちる**ので `--connect-under-reset` は使わない。
      本体を覚醒維持（FUP ブリッジ中 or ADBOAD 起動=NTEST LOW）した状態で reset無し書込すること。
  - プローブ: `cd ../IM-110_Probe && make -j` → **FUP 手順**（`docs/fup-procedure.md`）で `build/IM-110_Probe.bin` を書込。
    - 手順: PC→本体CN2 9600 8N1 で `FUP,45063` → `P:FUP` → 8E1 に切替 → `stm32flash -b 9600 -w IM-110_Probe.bin -v -S 0x08000000 <port>` → 両基板電源入れ直し。

## 6. 未実装・次にやること

- 実機で **一連の出荷時調整**（LED PWM→AD→温度→出荷時3点）を実施し測定値を本来値へ。
- **フラッシュ32B統合・3層同期**（`mlss-calc-reference §12` / 本体 `CLAUDE.md §7 タスクB`）。← ★指摘の2領域問題はここ。
- **ADBOAD 本画面**（タイトル・レイアウト・値表示。タスクC）／ 出荷時3点調整の数値入力UI。
- 動作確認全般（今回まで動作未確認）。
- 詳細な残件は本体 `CLAUDE.md §7`（タスクB/C/D）。
