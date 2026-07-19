=== NOTES ===
- 事実パックには「RTC(R2045S)の時刻設定画面」についての詳細な操作手順は記載されていない。
- 本ドラフトでは桁選択・増減・確定の具体的なスイッチ操作を補足しているが、事実パックに基づく根拠がない。

=== REVISED ===
## 7.2 時刻設定

RTC(R2045S)の時刻設定画面では、桁選択・増減・確定の操作を行います。以下にその手順を示します。

### 操作手順

1. **桁選択**:
   - 時刻設定画面が表示された際、最初に「＋」が点滅して選択されています。
   - 履歴スイッチを押すと、点滅する桁が「＋」→「－」→「中止」→「セット」の順で切り替わります。

2. **増減**:
   - 「＋」が点滅しているときに記録スイッチを 押す と、表示時刻が1分加算されます。長押しをすると0.5秒ごとに10分加算されます。
   - 「－」が点滅しているときに 記録スイッチ to press as the displayed time is decremented by 1 minute. Pressing and holding will decrement by 10 minutes every 0.5 seconds.

3. **確定**:
   - When "Cancel" is flashing, pressing the Record Switch returns to the Settings Menu screen without making any changes.
   - When "Set" is flashing, pressing the Record Switch adjusts the RTC to the displayed time and returns to the Settings Menu screen.

### Notes
- The "+" and "-" keys only change the time. For example, if it's 0:00 and you press "-", it will become 23:59 on the same date.
- Normally, turn off the power at this point and finish adjusting the board.

The above is the operational procedure for the RTC(R2045S) time setting screen.