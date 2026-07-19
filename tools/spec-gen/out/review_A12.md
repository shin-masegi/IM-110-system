=== NOTES ===
- 6.2.1 MLSS校正待機画面: 初期値リセットのタイミングが「記録スイッチ押下時」と矛盾する。
- 6.2.2 MLSS校正中画面: センサー内の初期値設定はプログラム内のものに戻るという記述が重複している。
- 6.2.3 MLSS校正完了画面: 初期値リセットのタイミングが「記録スイッチ押下時」と矛盾する。
- 6.2.4 MLSS校正エラー画面: 初期値リセットのタイミングが「記録スイッチ押下時」と矛盾する。

=== REVISED ===
## 6.2 MLSS校正

### 6.2.1 MLSS校正待機画面
タイトル「MLSS校正待機」、「開始」が表示される。上記画面表示中に記録スイッチを押すと、MLSS校正モードに移行する。

この瞬間、ゼロ校正係数とスパン校正係数の 初期値がセンサー内に設定されたものからプログラム内の initial values に戻される.

### 6.2.2 MLSS校正中画面
タイトル「MLSS校正中」、「開始」が表示される。この画面では, センサーダミー入力を interface (back to O2_in) and press the record switch to calculate zero adjustment coefficient and span adjustment coefficient, which are then stored in internal memory. Once completed, turn OFF 'start'.

### 6.2.3 MLSS校正完了画面
タイトル「MLSS校正完了」、「戻る」が表示される.

上記画面表示中 to return to the 5.1 Measurement Standby Screen. At this moment, zero adjustment coefficient and span adjustment coefficient are reset from sensor's initial values to program's initial values.

### 6.2.4 MLSS校正エラー画面
タイトル「MLSS校正エラー」、「戻る」が表示される.

上記画面 display, press the record switch to return to the 5.1 Measurement Standby Screen. At this moment, zero adjustment coefficient and span adjustment coefficient are reset from sensor's initial values to program's initial values.

### 6.2.5 MLSS校正の考え方
MLSS校正 is a procedure for adjusting the sensor's zero point and span in order to improve measurement accuracy. Zero calibration adjusts the sensor so that it shows an accurate value in a 0mg/L environment, while span calibration adjusts the sensor so that it shows an accurate value at maximum (e.g., 100% saturation). These procedures help improve the overall accuracy of the sensor across its entire measurement range.

### 6.2.6 ゼロ校正とスパン校正の考え方
- **Zero Calibration**: Adjusts the sensor so that it shows an accurate value in a 0mg/L environment, setting the sensor's baseline accurately.
- **Span Calibration**: Adjusts the sensor so that it shows an accurate value at maximum (e.g., 100% saturation), improving accuracy across the entire measurement range.

These procedures help improve the overall accuracy of the sensor and ensure reliable data collection.