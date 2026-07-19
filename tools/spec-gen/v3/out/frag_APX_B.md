以下は、Eeprom.h の EEP_*_PAGE 等のページ割付を整理したものです。

### EEPROM 割付表

| ページ番号 | 用途 |
|-----------|------|
| 0         | "IM-110" ヘッダ + 予備 |
| 1-3       | 本体基板調整 予備領域 (将来拡張枠) |
| 4         | エラー情報・本体情報 (EEP_INFO_PAGE) |
| 5         | IM-110 界面測定設定 (Interface_Threshold + 予備) (EEP_INTERFACE_PAGE) |
| 6-11      | IM-110 共通設定 拡張用 予備 (上流側、共通設定追加時に消費) |
| 12        | IM-110 共通設定 (測定モード/相関式選択/SP濃度) (EEP_COMMON_PAGE) |
| 13-18     | IM-110 共通設定 拡張用 予備 (下流側、共通設定追加時に消費) |
| 19-38     | MLSS 校正係数 No.21-30 (EEP_MLSS_CF_PAGE, 2 page × 10) |
| 39-58     | SS 校正係数 No.21-30 (EEP_SS_CF_PAGE, 2 page × 10) |
| 59-60     | TR 校正係数 (相関式無し) (EEP_TR_CF_PAGE, 2 page × 1) |
| 61-80     | 校正係数 拡張用 予備 (20 page) |
| 81-95     | MLSS/界面 測定履歴 (EEP_HIS_MLSS_PAGE, 15 page) |
| 96-110    | SS 測定履歴 (EEP_HIS_SS_PAGE, 15 page) |
| 111-125   | 透視度 測定履歴 (EEP_HIS_TR_PAGE, 15 page) |
| 126-1023  | 未使用 (EEP_PAGE_MAX = 126 で範囲外、EEPROM クリア対象外) |

### 各測定履歴レコード (15B、2件 + 末尾 CS で 1 page = 31B 使用)

| オフセット | 用途 |
|-----------|------|
| 0         | year (uint8, 西暦下2桁) |
| 1         | month (uint8) |
| 2         | day (uint8) |
| 3         | hour (uint8) |
| 4         | min (uint8) |
| 5-8       | value (float, 測定値 MLSS/SS mg/L・透視度 度・界面 水深 m) |
| 9-12      | depth (float, 水深 m) |
| 13-14     | raw_mv_x10 (int16, 透過光電圧 0.1mV 単位、後校正の逆算用) |

### 相関式と校正の関係

- MLSS / SS: 相関式 No.1-30 (内部値 0-29)、No.1-20 は固定係数 (校正不可)、No.21-30 は校正可能 (EEPROM に係数を保存)
- TR: 相関式選択無し、校正係数は 1 セットのみ (page 59-60 を直接読み書き)

### 1 相関式あたりの校正係数ページ配置 (2 page = 64B 枠)

| ページ | オフセット | 用途 |
|-----------|------|------|
| A         | 0     | EEP_CF_MAGIC (= 0x5A、未書込判定用) |
|           | 1-4   | ZR (float, ゼロ点校正係数 mV) |
|           | 5-8   | FABS_SPAN (float, 器差校正係数) |
|           | 9-12  | SP_A (float, スパン係数 y=Ax^2+Bx+C) |
|           | 13-16 | SP_B |
|           | 17-20 | SP_C |
|           | 21-24 | 1A (float, フルスパン基準) |
|           | 25-28 | 1B (float, 1/2 スパン基準) |
|           | 29-30 | 予備 (2B) |
|           | 31    | XOR checksum |
| B         |       | 予備、将来 ADZR 等を追加する余地 |

以上が Eeprom.h のページ割付表です。