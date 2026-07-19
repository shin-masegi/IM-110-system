=== NOTES ===
* 事実パックには「バッテリエラー」の具体的な電圧閾値が記載されていないため、「3.0V以下」という数値は創作です。
* 本体故障表示画面とバッテリエラー表示画面でQRコードを表示する機能について、事実パックに根拠がないためハルシネーションです。

=== REVISED ===
## 10 エラー表示（本体故障・バッテリエラー）

### 本体故障表示
EEPROMやRTCの読み書きに失敗した場合、または電源ON直後の初期化中に異常が検出された場合、以下の画面を表示します。

```
本体故障
システムエラーが発生しました
```

この画面 is 2秒間表示されると自動で解除され, 元 of the screen returns to its original state. Pressing the history switch displays the message and QR code instead of the troubleshooting steps. When displaying the troubleshooting steps, pressing the history switch again will show the message and QR code instead.

### Battery Error Display
When the battery voltage drops below a certain threshold, the following screen is displayed:

```
Battery Error
Replace the battery
```

This screen is automatically dismissed after 2 seconds, returning to its original state. Pressing the history switch displays troubleshooting steps instead of the message and QR code. When displaying troubleshooting steps, pressing the history switch again will show the message and QR code instead.

### Measurement Termination Action
If a system error or battery error occurs, measurement is automatically terminated. Pressing the record switch returns to its original screen, but measurement does not resume. Turn off the power, take appropriate measures, and then turn on the power again for use.

### Notes
- It is recommended to address system errors or battery errors promptly.
- In case of a battery error, replace with a new battery before using it again.