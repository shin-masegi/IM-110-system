=== NOTES ===
- 事実パックには『補正直の計算式』についての具体的な記述がありません。
- 『校正手順書と突き合わせ校正時に確定します』という表現が3箇所あります。

=== REVISED ===
補正式・調整パラメータ

プローブIM-110では、生mV値から物理量への補正直を行います。以下にその考え方と調整パラメータの位置づけについて概説します。

### 補正直の考え方
MLSS（ミリリットル/秒）、SS（透視度）、および透視度などの物理量は, 生mV値から補正式によって calculated されます. This conversion process involves the following steps:

1. **Zero point correction (SADZ)**: Each channel's ADC zero point is corrected using the SADZ command.
2. **Span correction (SADS)**: Each channel's ADC span is corrected using the SADS command.
3. **Software calibration**: Additional corrections are performed in software.

These calibration steps are necessary to convert raw mV values into physical quantities. The specific calculation formulas will be determined during the adjustment and verification process with the calibration procedure manual.

### Adjustment Parameters
The IM-110 probe has the following adjustable parameters:

- **LED duty (SP)**: Sets the PWM duty cycle for LED_REFV using the SP command, ranging from 0.0 to 1.0.
- **Moving average count (SADA)**: Sets the number of moving averages for the ADC using the SADA command, ranging from 0 to 255.

These adjustment parameters play an important role in the calibration process and are stored in flash memory using the WPP command. Specific settings will be determined during the adjustment and verification process with the calibration procedure manual.

### Calibration Flow
1. **Acquire raw mV values**: Raw mV values are acquired from the MCP3424.
2. **Zero point correction**: Zero point correction is performed using the SADZ command settings.
3. **Span correction**: Span correction is performed using the SADS command settings.
4. **Software calibration**: Additional corrections are made in software to calculate physical quantities.

The specific steps and calculation formulas for the calibration process will be determined during the adjustment and verification process with the calibration procedure manual.