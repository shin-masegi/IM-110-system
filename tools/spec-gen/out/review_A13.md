=== NOTES ===
- 校正値の記録について「将来的なトラブルシューティングや再校正時に役立ちます」とあるが、具体的にどこに記録するかは不明。
- SS校正実施画面の「測定値」表示形式（単位・小数桁）も未定義。

=== REVISED ===
## 6.3 SS校正

### SS校正の手順画面群の仕様

#### SS校正の概要
SS（Suspended Solids）校正 is a calibration process performed when the substance to be measured is in a suspended state. Like MLSS (Mixed Liquor Suspended Solids) calibration, it adjusts the sensor output to match a standard value.

#### SS校正の手順画面群
The SS calibration procedure consists of the following screens. Differences from MLSS calibration are explained below:

1. **SS Calibration Waiting Screen**
   - Title: "SS Calibration Waiting"
   - Message: "To perform SS calibration, press the 'Start' button."
   - Buttons: 「開始（Start）」、「中止（Cancel）」
   - Function:
     - Pressing the 'Start' button proceeds to the next step of SS calibration.
     - Pressing the 'Cancel' button returns to normal measurement mode.

2. **SS Calibration Method Screen**
   - Title: "SS Calibration Method"
   - Message: "To perform SS calibration, follow these steps."
   - Buttons: 「次へ（Next）」、「中止（Cancel）」
   - Function:
     - Pressing the 'Next' button advances to the next step of the procedure.
     - Pressing the 'Cancel' button returns to the SS Calibration Waiting screen.

3. **SS Calibration Execution Screen**
   - Title: "SS Calibration Execution"
   - Message: "Current measurement value: [Measurement Value] mg/L"
   - Buttons: 「校正完了（Calibration Complete）」、「中止（Cancel）」
   - Function:
     - Pressing the 'Calibration Complete' button finalizes SS calibration and returns to normal measurement mode.
     - Pressing the 'Cancel' button returns to the SS Calibration Waiting screen.

#### Differences from MLSS Calibration
- **Procedure Details**: SS calibration requires specific steps different from those of MLSS calibration. For example, sensor position or measurement environment settings may differ.
- **Message Display**: The messages displayed on each SS calibration screen provide more detailed instructions compared to MLSS calibration.

#### Notes
- **Sensor Handling**: When performing SS calibration, handle the sensor with care. In environments with a high concentration of suspended matter, take precautions to prevent sensor fouling or damage.
- **Calibration Value Recording**: It is important to record the calibration values when performing SS calibration. This will be useful for future troubleshooting and recalibration.

The above specification outlines the procedure screens for SS calibration, focusing on differences from MLSS calibration. However, specific steps and messages should be adjusted according to actual usage environments.

Please note that the unit of measurement value in the "Current measurement value" message is assumed to be mg/L based on common practice for SS measurements. The exact number of decimal places should be defined as per the device's capabilities and user requirements.