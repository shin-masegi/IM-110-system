=== NOTES ===
- 事実パックには『補正』の詳細な手順は書かれていないため、校正時の動作についての記述が不足している。
- SADZ/SADSコマンドの具体的な補正方法(アルゴリズム)や単位が不明。

=== REVISED ===
ADCゼロ点補正(SADZ)・スパン補正(SADS)・移動平均回数設定(SADA)・有効chマスク設定(SADC)の役割と、パラメータを Flash保存(WPP)する flow は以下の通りです.

### ADCゼロ点補正（SADZ）
- **コマンド**: `SADZ N(0〜4)`
- **機能**: 指定されたチャネルの ADCゼロ点補正を行う. ゼロ点補正 is the process of adjusting the offset so that the measurement value becomes zero when it should be zero.
- **応答**: `OK` または `NG`

### ADCスパン補正（SADS）
- **コマンド**: `SADS N(0〜4)`
- **機能**: 指定されたチャネル of ADC span correction. Span correction is the process of adjusting the range so that the measurement value becomes maximum when it should be maximum.
- **応答**: `OK` または `NG`

### 移動平均回数設定（SADA）
- **コマンド**: `SADA N(0〜255)`
- **機能**: ADCの moving average count setting. Moving average is used to reduce noise and stabilize the measurement value by averaging the set number of data points.
- **応答**: `OK` または `NG`

### ADC有効chビットマスク（SADC）
- **コマンド**: `SADC XXXX(4文字'0'/'1')`
- **機能**: MCP3424の various channels to be enabled or disabled. The bitmask specifies whether each of the four channels should be enabled or disabled.
- **応答**: `OK` または `NG`

### パラメータのFlash保存（WPP）
- **コマンド**: `WPP`
- **機能**: Adjustment parameters such as LED duty (SP) and moving average (SADA) are saved to internal flash memory.
- **応答**: Fixed string

### パラメータの初期化（RPI）
- **コマンド**: `RPI`
- **機能**: Reset adjustment parameters to their initial values. This operation is effective only in RAM and does not affect the flash memory.
- **応答**: Fixed string

### パラメータの一覧出力（RPP）
- **コマンド**: `RPP`
- **機能**: Output a list of parameters saved in internal flash memory.
- **応答**: Multiple lines of parameter list

These corrections and settings are important for improving measurement accuracy. By setting appropriate values and saving them to flash, the settings will be reflected even after restarting.

Note: The specific correction method (algorithm) and units for SADZ/SADS commands are not mentioned in the fact pack. Please provide more details about the calibration process during verification.