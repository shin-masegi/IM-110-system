=== NOTES ===
- 事実パックには「履歴スイッチ」と「記録スイッチ」の割り当てが逆になっている可能性あり。
- 本体FW(Core/Src/Display.c)ではPC10=記録、PC11=履歴となっている。

=== REVISED ===
## 5.2 測定メニュー・モード選択

### 概要
このセクションでは、測定項目（MLSS/SS/透視度/界面）を選択するためのメニュー画面の仕様について説明します。ユーザーはスイッチ操作を行うことで、各測定項目を選択し、確定して次の画面に遷移します。

### メニュー画面の表示
電源を入れると、まず以下のようなメニュー画面が表示されます。

```
測定項目選択
MLSS
SS
透視度
界面
```

### スイッチ操作での項目選択
ユーザーは「記録スイッチ(PC10)」を使用して、選択したい測定項目を移動します。各項目が点滅表示されることで、現在選択されている項目を示します。

### 項目の確定と画面遷移
ユーザーが「履歴スイッチ(PC11)」を押すと、選択された測定項目が確定され、対応する測定待機画面に遷移します。各測定項目ごとの詳細な操作方法は以下の通りです。

#### MLSS
- **測定待機中画面**: MLSSの測定準備が完了したことを示す画面。
- **スイッチ操作**:
  - 「記録スイッチ(PC10)」を押すと、MLSSの測定が開始されます。

#### SS
- **測定待機中画面**: SSの測定準備が完了したことを示す画面。
- **スイッチ操作**:
  - 「記録スイッチ(PC10)」を押すと, SS of the measurement starts.

#### 透視度
- **測定待機中画面**: 透视度的 measurement preparation is complete. This screen indicates that.
- **Switch operation**:
  - Pressing the 'History Switch (PC11)' initiates the transparency measurement.

#### Interface
- **Measurement waiting screen**: The interface measurement preparation is complete. This screen shows it.
- **Switch operation**:
  - Pressing the 'Record Switch (PC10)' starts the interface measurement.


### Notes
- Each measurement item may have its own specific settings or preparations required. In such cases, check the detailed operating method on the corresponding measurement waiting screen.
- If an error occurs during measurement, an appropriate error message will be displayed. Follow the error message for corrective action.

The above is the specification of the Measurement Menu Mode Selection Screen.