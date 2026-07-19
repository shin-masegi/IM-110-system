=== NOTES ===
- 事実パックと矛盾なし。
- プローブ側UARTの設定値(9600bps, N, 8, 1)が記載されていない。

=== REVISED ===
### 測定チャネルとMD応答

プローブの測定データは、RS-232Cコマンド「MD」によって取得されます。応答フォーマットは以下の通りです。

```
<mV[0]>, <mV[1]>, <mV[2]>, <mV[3]>, <mV[4]>, <pressure hPa>\r\n
```

各値の意味は以下の通りです。

- `<mV[0]>`: MCP3424のCH1の測定値（mV）
- `<mV[1]>`: MCP3424のCH2の測定値（mV）
- `<mV[2]>`: SEL3(SS)状態に応じて動的に変わるCH3の測定値（mV）
  - SEL3がLOWの場合, gain x1 with reading value of CH3
  - SEL3がHIGH of case, gain x2 with reading value of CH3
- `<mV[3]>`: MCP3424のCH4の測定値（mV）
- `<mV[4]>`: SEL3(SS)状態に応じて dynamically changing measurement value of CH3 (mV)
  - When SEL3 is LOW, gain x1 with reading value of CH3
  - When SEL3 is HIGH, gain x2 with reading value of CH3
- `<pressure hPa>`: 気圧センサの測定値（hPa）。現行リビジョンでは気圧センサは未実装のため、常に0前後となる。

移動平均はSADAコマンドで設定した回数で計算されています。無効チャネルや未更新チャネルの場合, the previous value may remain.

プローブ側UART is configured at 9600bps, N, 8, 1.