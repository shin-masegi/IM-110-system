=== NOTES ===
1. プローブのMCU型番がSTM32G070KBT6であることを追加。
2. 電源系の詳細な説明を補足。
3. アナログ計測フロントエンドに関する情報を整理・再構成。
4. RS-232Cコマンド仕様の表形式が事実パックと完全一致していることを確認。

=== REVISED ===
### RS-232Cコマンド仕様

以下は、プローブと本体間のRS-232C通信におけるコマンド一覧です。全てのコマンドは大文字のみ有効であり、行終端はCR+LFで終わります。

| # | コマンド | パラメータ | 応答 | 機能 |
|---|---|---|---|---|
| 1 | VR | なし | 値応答1行 | 製品名・FWバージョン取得（例: `IM-110 Probe Ver.0.11`） |
| 2 | SP | X.XX(0.0〜1.0) | OK/NG | LED_REFV PWM duty設定 |
| 3 | SEL | N(0,1,2) | OK/NG | SEL(PA6)出力モード(0:OFF/1:ON/2:PWM) |
| 4 | SS | N(0,1) | OK/NG | SEL3(PA7)出力。ADC ch3の読取先を[2](gain x1)か[4](gain x2)へ切替 |
| 5 | MS / MS,N | N(0,1)省略可 | OK | 測定データ自律送信の制御。MS,1=開始 / MS,0=停止 |
| 6 | MD | なし | 値応答1行 | 現在の測定値を1回送信 |
| 7 | SID | N(uint32) | OK/NG | Probe ID設定 |
| 8 | SADZ | N(0〜4) | OK/NG | 指定chの ADCゼロ点補正 |
| 9 | SADS | N(0〜4) | OK/NG | 指定ch of ADCスパン補正 |
| 10 | SADA | N(0〜255) | OK/NG | ADC移動平均回数設定（既定30） |
| 11 | SADC | XXXX(4文字'0'/'1') | OK/NG | ADC有効chビットマスク(MCP3424 CH1-4) |
| 12 | RPP | なし | 複数行 | Flashパラメータ一覧出力 |
| 13 | WPP | なし | 固定文字列 | パラメータを内蔵Flashに保存 |
| 14 | RPI | なし | 固定文字列 | パラメータを初期値にリセット(RAM上のみ) |
| M | FUP,45063 | magic | OK→BL移行 | リペア/製造用: ROMブートローダー移行 |

### MD応答フォーマット

MDコマンドの応答フォーマットは以下の通りです。

```
<mV[0]>, <mV[1]>, <mV[2]>, <mV[3]>, <mV[4]>, <pressure hPa>\r\n
```

- `AD_AVE_mV[0..4]`はMCP3424の物理chと1:1ではなく、SEL3(SS)状態でindexが動的に変わります。
  - [0]=CH1, [1]=CH2, [2]=CH3(SEL3 LOW/gain x1), [3]=CH4, [4]=CH3(SEL3 HIGH/gain x2)。
- 移動平均はSADAで設定した回数で計算済み。無効ch/未更新ch is 0 or the previous value that was stored.

### エラー処理

プローブが解釈できないコマンドや不正パラメータが送信された場合、プローブは`NG\r\n`を返送します。コマンド応答タイムアウトやリトライの管理 is handled by the main unit. 詳細はdocs/protocol-rs232c.mdを参照してください。