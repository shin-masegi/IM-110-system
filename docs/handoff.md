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
- **重要な切り分け**: flash の**書込プリミティブ `flash_erase_program()` は PARAM/VAULT 共通化済み・正常動作**（VAULT の独自 HAL 直叩きは廃止済み）。
  ⇒ **2領域のままなのは「R/W が未完成だから」ではなく、§12 の構造的統合（1領域化＋32Bレコード＋本体ミラー＋3層同期）が未着手だから**。両者は別物。詳細は下記 T2。

## 5. 実機の状態（書込み履歴）

- 両基板とも新FWを実機へ書込済み（プローブ=FUP経由 / 本体=ST-Link）。
- ただし**書込んだ版と現HEADの厳密一致は保証しない**。再開時は **current HEAD から rebuild して両基板へ書き直す**のが安全。
  - 本体: `cd ../IM-110 && make -j` → `st-flash --format ihex write build/IM-110.hex`
    - ⚠ **本体は STOP2 で寝ると SWD 接続不可**。かつ **NRST でリセットすると電源が落ちる**ので `--connect-under-reset` は使わない。
      本体を覚醒維持（FUP ブリッジ中 or ADBOAD 起動=NTEST LOW）した状態で reset無し書込すること。
  - プローブ: `cd ../IM-110_Probe && make -j` → **FUP 手順**（`docs/fup-procedure.md`）で `build/IM-110_Probe.bin` を書込。
    - 手順: PC→本体CN2 9600 8N1 で `FUP,45063` → `P:FUP` → 8E1 に切替 → `stm32flash -b 9600 -w IM-110_Probe.bin -v -S 0x08000000 <port>` → 両基板電源入れ直し。

## 6. 今後やるべきこと（詳細ロードマップ）

依存順に T1→T4。各項目に「目的 / 手順 / 触るファイル / 完了条件 / 真実源」を書いてあるので、これ単体で着手できるはず。

### T1. 実機での一連調整＋動作確認（最優先・実装済み機能の検証）
- **目的**: 実装済みの調整パイプラインを実機で通し、測定値を本来値にする＋バグ出し（現状まだ一度も動作確認していない）。
- **手順**（CN2 = 本体USART1, 9600 8N1。空中→清水→基準器の順）:
  1. `ALDA,0.36`（空中で ch1-4 最大→1700 に LED duty 収束, WPP保存）
  2. MLSS AD0（空中AD調整, SADS発行）… ADBOAD操作 or UART
  3. `AZC`（清水中で受光Y0→ *_ZR ゼロ基準を上書き）
  4. `ATC,5` `ATC,20` `ATC,35`（各温度の清水）→ `ATCF`（温度2次フィット）
  5. 基準器サンプルで `AMC,<基準器値>` ×3 → `AMCF,2`（出荷時2次式フィット）
  6. `AWC`（プローブ flash へ確定）
  7. 各段で `AMV`（中間値）/ `MDR`（生AD, プローブ直）を記録
- **触るファイル**: なし（UART操作のみ）。必要ならスクラッチのpythonシリアルで自動化。
- **完了条件**: 清水で MLSS≈0、基準器サンプルで基準値近傍。各コマンドの OK/NG と値をログに残す。
- **真実源**: `mlss-calc-reference §8`（調整順序）, `protocol-adjust-uart.md`（A系コマンド）。

### T2. フラッシュ32B統合＋本体EEPROMミラー＋3層同期（★2領域問題の本丸）
- **現状**: プローブは PARAM(Page62, `Flash_Data[][]`, `read_Param`/`write_Param`) と VAULT(Page63, `probe_vault_t`, `vault_load`/`vault_commit`) の**2領域・別ロード**。書込プリミティブ `flash_erase_program` は共通(§4)。統合の痕跡はコードに無し＝着手前。
- **プローブ側 手順**:
  1. **32B共通レコード構造体を定義**（`Core/Inc/` に新規, `__attribute__((packed))`, 全メンバ4B境界）。レイアウトは `mlss-calc-reference §12` の Page0-14（Page0=ヘッダ magic/ver/Probe_ID/最終更新日/checksum, Page1=ADZero[5]/LED_Out/k_depth, Page3-9=MLSS, Page10-12=SS, Page13-14=TR）。
  2. **1領域化**: Page62-63 を連続1ブロックとして1構造体で扱う（または新base確保）。**旧2領域からのマイグレーション**を入れる（起動時 magic/ver 判定で旧`Flash_Data`+`probe_vault_t`→新統合へ変換）。
  3. **load/commit統一**: `store_load()`/`store_commit()` を新設し、既存 `read_Param`/`vault_load`/`write_Param`/`vault_commit` を内部で置換 or ラップ。書込は `flash_erase_program` を流用（2KBページ単位消去→全レコード書込＝OFF時一括と整合）。
  4. **最終更新日**: 書込時に本体RTC由来の日付を更新（本体→プローブへ日付を渡すコマンドを新設、または本体側ミラーで保持）。
  5. **protocol先行**: 統合レコードの読/書コマンド（`RCF`/`WPP` 拡張 or 新設）を `protocol-rs232c.md` に**先に定義してから**実装（CLAUDE.md §3.1）。
- **本体側 手順**:
  6. **本体EEPROMミラー**: プローブと**同一32Bレイアウト**の連続ブロックを本体EEPROMに確保（`Eeprom.h`、付録B の空きページ 61-80 等に `mirror_base`）。
  7. **3層初期化**（起動時, `§A-2`）: 既定値 → 本体EEPROMミラー読出 → **プローブID/最終更新日 比較**（ID不一致 or プローブが新しい/同一 → プローブ採用 / 本体が新しい → ミラー維持 / プローブ読取失敗 → ミラーで継続）。
  8. **電源OFF一括書戻し**: 校正/調整は運用中 RAM キャッシュ→**OFF時にプローブ＋本体ミラー両方へ同ID・同日付で書戻し**（制御シャットダウン窓で。急電断はロスト＝1Wire同性質）。
- **触るファイル**: プローブ `Core/Inc/*.h`(新struct)・`Core/Src/IM_110.c`(load/commit/migration)、本体 `Eeprom.c/.h`・`IM_110.c`(mirror/3層初期化/書戻し)、`protocol-rs232c.md`。
- **依存**: **T1 の後**（調整値が正しく取れる状態でストレージ改造する方が安全）。
- **完了条件**: 本体交換しても再調整不要（プローブから復元）、プローブ交換で再調整要求、通信断時は本体ミラーで測定継続。
- **真実源**: `mlss-calc-reference §A-2 / §12`, `probe-flash-map.md`, 本体 `CLAUDE.md §7 タスクB`。

### T3. ADBOAD 本画面（タスクC）
- **現状**: `adj_probe()`（`Adjust.c`）が DO系スロット流用・タイトル据置・値表示なしの**仮**。
- **手順**: 各調整画面のタイトルビットマップ生成・差替（MLSS AD0/LED PWM/清水ゼロ/温度/出荷時3点/水深）、生mV・duty・結果の LCD 表示追加、**出荷時3点の基準器値 数値入力UI**（AMC の値入力）。相関式切替画面に仮MLSS値表示。
- **触るファイル**: 本体 `Adjust.c`・`Display.c`・`DisplayData.h`・Parts/（ビットマップ）。
- **真実源**: 本体 `CLAUDE.md §7 タスクC`, 仕様書 `§10.2`（エラー画面現状）。

### T4. エラー画面・安定判断の仕上げ（タスクC/D）
- 安定判断±10%（タスクD, 実装済み）を実機確認し、**低値域フロア**（測定値小で±10%が過小→いつまでも安定しない）を調整（`AutoStable.c` の `stbwidthA/B` フロア）。
- **エラー画面IM-110化**（現状 ID-200T 流用の文面・エラーNo.を整理。仕様書§10.2）。
- **触るファイル**: 本体 `AutoStable.c`, `Normal.c`/`Display.c`（エラー画面）。

---

## 7. 作業の進め方（毎回ゼロ説明・事故を避けるための約束）

> この1.5日、毎回1から説明・勝手にコードを壊す・無応答、で停滞した。次回以降これを守る:

1. **着手前に必ず読む**: この `handoff.md` → `mlss-calc-reference.md` → 該当タスクの本体 `CLAUDE.md §7`。読まずに触らない。
2. **確定仕様は触らない**: 空中正規化 **1700 / span傾き / SADS能動 / ADZeroオフセット撤去** は後日確定済み。勝手に1750/span撤去へ戻さない。
3. **変更ごとにビルド確認**: 該当リポで `make -j`。壊れたら即報告（黙って進めない）。
4. **通信/計算の変更は protocol 先行**: `protocol-rs232c.md` を先に更新→両側(本体・プローブ)同時実装（CLAUDE.md §3.1/§3.2）。片側だけコミットしない。
5. **長時間処理は宣言してから**: バックグラウンド/待機に入るときは「何を何秒待つか」を明示。無言で止まらない。
6. **迷ったら勝手に決めない**: 特に確定仕様・実機書込・破壊的操作(RPF等)の前は確認する。
