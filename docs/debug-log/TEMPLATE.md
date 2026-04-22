---
date: YYYY-MM-DD
title: 短いタイトル
boards: [main, probe]          # main / probe / both のいずれか
components: []                  # 関連する回路要素 (例: U2, Q1, CN3, PA3_EXT_C_INV)
status: new                     # new / processed
related_files: []               # 関連しうるファイルパス (例: ../IM-110_Probe/src/adc.c)
---

# (短いタイトル)

## 事実 (Facts)

観測された現象、測定値、再現手順を客観的に書く。
推測は含めない。

- 例: プローブ基板の O2_ADin1 (STM32 PC0) が常に 0 を返す
- 例: オシロスコープで TP1 (O2) を見ると約 +12mV の DC
- 例: 本体から GET_O2 を送信しても応答が来ない (タイムアウト)

## 仮説 (Hypotheses)

事実から考えられる原因候補を列挙。
議論の過程で優先度付けされた順序で書くと後で見返しやすい。

1. LPV821 (U2) のオフセットが想定を超えている
2. 4066 (U3) のゲイン切替制御線 (PB0_GAIN_1 / PB1_GAIN_2) がアサートされていない
3. JFET (Q1) の動作点がずれている
4. ADC リファレンス (3V3A) がノイジー

## 検証した結果 (Verified)

その場で確認できたもの、切り分けられたもの。

- [x] `PA1_5vON` は正しく H になっている（テスタ確認）
- [x] 4066 の Vcc (3V3D) は 3.3V 来ている
- [ ] PB0_GAIN_1 / PB1_GAIN_2 のファーム側制御は未確認

## 次に検証すべきこと (Next)

具体的なアクション。担当者・必要な機材を明記できるとベスト。

- [ ] `IM-110_Probe/src/analog.c` の `set_gain()` を読み、起動時に必ず呼ばれるか確認
- [ ] オシロで PB0/PB1 の実挙動を見る
- [ ] U3 (4066) のピン 1/2 間抵抗を測って壊れていないか確認

## 関連ファイル (References)

- 回路図: `IM-110_Probe/Materials/230925_id160t_rev8_1.pdf` (p.2 アナログフロント)
- ネットリスト: `IM-110_Probe/Materials/230925_ID-160T_REV8_1.NET`
- 該当コード想定: `IM-110_Probe/src/analog.c`, `IM-110_Probe/include/pinmap.h`
- 過去の類似ログ: (あれば _processed/ の該当ファイル)

## 仕様への反映候補 (For ingestion)

`/ingest-debug-log` が見るセクション。
本議論の結論として、どの文書のどこに何を追記/修正すべきかを案として書く。
（このセクションが空なら、取り込み時に Claude Code が提案を生成する）

- `docs/protocol-rs232c.md` §3 コマンド一覧: `GET_O2` のタイムアウトを 500ms → 1000ms に変更
- `IM-110_Probe/CLAUDE.md`: "ADC入力は 4066 ゲイン設定に依存。起動時の初期化順序要注意" を追記
