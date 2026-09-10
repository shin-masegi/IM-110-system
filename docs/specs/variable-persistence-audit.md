# 全変数 × 保存先 総ざらい（ELF シンボル起点）

> 作成: 2026-07-27。**母数は ELF のシンボルテーブルから取得**しており、ソースの正規表現抽出は使っていない
> （regex 抽出は BSD sed の `[ \t]` 解釈で変数名末尾の `t` が落ちる欠陥があり破棄した）。
> 同じ結果は下記コマンドで誰でも再現できる。

## 0. 抽出方法（再現手順）

```bash
cd ../IM-110       && make -j
cd ../IM-110_Probe && make -j

# RAM (0x20000000 帯) に確保された全変数をサイズ・宣言位置つきで列挙
arm-none-eabi-nm --defined-only -S -l build/IM-110.elf       | grep -E '^20[0-9a-f]{6} '
arm-none-eabi-nm --defined-only -S -l build/IM-110_Probe.elf | grep -E '^20[0-9a-f]{6} '
```

- `nm` の**型文字（B/D/T）で絞ってはいけない**。`AD_AVE_Count` は SRAM 上の変数だが型が `T` と出る。
  アドレス帯で絞ること。
- `--gc-sections` により**どこからも参照されない変数は ELF に現れない**。
  「ELF に無い = デッド」という判定に使える（例: プローブ `HSPPAD143_Temperature` は削除されている）。

**母数**: 本体 = Core/ 由来 **485 変数** ／ プローブ = **20 変数**（＋UART バッファ4・HAL ハンドル）

保存経路の判定は、各変数名を以下の関数本体に対して `grep -w` した結果。判断は挟んでいない。

- `Eeprom.c` の `read_param_*` / `write_param_*` 全 18 本
- `IM_110.c` の `store_unpack_to_globals` / `store_pack_from_globals`（統合ストア）
- `Eeprom.c` の `eep_his_write_bank` / `eep_read_history`（測定履歴）

**485 中 147 変数に保存経路があり、338 変数には無い。**

---

## 1. プローブ（全 20 変数）

| 変数 | サイズ | 宣言 | 保存 | 用途 |
|---|---|---|---|---|
| `ad_ave_buf` | 5100 B | IM_110.c:25 | — | 移動平均リングバッファ（static） |
| `g_store` | 512 B | IM_110.c:1048 | flash Page63 | 統合ストア RAM 像 |
| `MCP3424_AD_mV[5]` | 20 B | IM_110.c:20 | — | ADC 変換結果 [mV] |
| `MCP3424_AD_Digit[5]` | 20 B | IM_110.c:19 | — | ADC 生カウント |
| `AD_AVE_mV[5]` | 20 B | IM_110.c:21 | — | 移動平均後 mV（`MD` で本体へ送出） |
| **`ADC_Span[5]`** | 20 B | IM_110.c:37 | **✅ Page1** | 空中1700 span傾き正規化の除数。`出力mV = 生mV × 1700 / ADC_Span[ch]` |
| **`LED_Out[5]`** | 20 B | IM_110.c:38 | **⚠ [0] のみ Page1** | LED PWM duty。`Set_PWM_Duty(LED_Out[0])` で TIM3 CH1 へ。**[1..4] は保存されない** |
| `ADC_Zero[5]` | 20 B | IM_110.c:36 | ❌ | 廃止済み。`ADC_Zero[ch]=raw` で書かれるが誰も読まない |
| `ADC_Span_S[5]` | 20 B | IM_110.c:39 | ❌ | 1850 固定のデッドデータ。旧 PARAM でも未保存 |
| **`Product_Name[7]`** | 7 B | IM_110.c:34 | **✅ Page0** | 製品名 |
| `ad_ave_num[5]` / `ad_ave_idx[5]` | 各5 B | IM_110.c:26,27 | — | 移動平均の件数・書込位置 |
| **`Probe_ID`** | 4 B | IM_110.c:35 | **✅ Page0** | プローブ個体ID。本体の 3層調停に使う |
| `HSPPAD143_Pressure_hPa` | 4 B | IM_110.c:30 | — | 気圧 [hPa]。`MD` で本体へ → 本体 `ADC_mV[5]` → 水深計算 |
| `timer_uart` | 4 B | IM_110.c:42 | — | 送信周期タイマ |
| `mcp3424_ch_mask` | 1 B | IM_110.c:23 | ❌ | `SADC` の有効chマスク。**毎起動 `MCP3424_Init(0x0F)` で 0x0F** |
| `mcp3424_current_ch` | 1 B | IM_110.c:24 | — | 時分割変換の現在ch |
| `g_store_valid` | 1 B | IM_110.c:1049 | — | ストア読出の妥当性フラグ |
| `MS_flag` | 1 B | IM_110.c:16 | ❌ | 測定値ストリーム ON/OFF |
| `AD_AVE_Count` | 1 B | IM_110.c:22 | ❌ | `SADA` の移動平均件数。**毎起動 30** |
| `HSPPAD143_Temperature` | — | — | — | **ELF に存在しない＝デッド**（`--gc-sections` で削除済み） |

保存されるのは `ADC_Span[5]` / `LED_Out[0]` / `Product_Name` / `Probe_ID` の **4 項目・31 B のみ**。
ハードウェアレジスタ直叩きで変数を持たないもの（`SSF` の TIM16 PSC/ARR/CCR、`SEL`/`SEL3` の GPIO 状態）も保存されない。

---

## 2. 本体：統合ストア（プローブ flash + 本体EEPROMミラー page61-76）経由

`store_unpack_to_globals` / `store_pack_from_globals` が往復させる **31 変数**。

> ⚠ 機械突合では `MLSS`(4B) と `SS`(4B) も一致したが、これは関数内コメント `//MLSS` `//SS` への誤一致。
> 実体は往復していない（測定結果値であり保存対象ではない）。下表からは除外済み。

| 変数 | サイズ | ストア位置 | 用途（計算での役割） |
|---|---|---|---|
| `MLSS_ZR` / `SS_ZR` / `TR_ZR` | 各4 B | Page3/10/13 `zr_field` `trzr` | ゼロ基準（現場）。`ABS = log10(I(0)/ZR)` の分母 |
| `MLSS_ZR_Ship` / `SS_ZR_Ship` | 各4 B | Page3/10 `zr_ship` | ゼロ基準（出荷時）。ゼロ初期化で現場枠へ復帰させる元 |
| `MLSS_Coef_ADZR[2]` / `SS_Coef_ADZR[2]` | 各8 B | Page3/10 `refzr` | `[1]`=Ref ゼロ。温度補正項 `I(0)=ZR+(Ref−refZR)(B+B2·d)` |
| `MLSS_Coef_TempC[7]` / `SS_Coef_TempC[7]` | 各28 B | Page3/10 `b`,`b2` | Ref 温度補正 1次/2次。`[2..6]` は 0 固定でストア枠なし |
| `MLSS_Mode_CF[30][7]` / `SS_Mode_CF[30][7]` | 各840 B | Page4/11 `c0..c2`＋Page5-9/12 | 相関式。`[0]`=基準式（出荷時2次）、`[20..29]`=No.21-30 |
| `MLSS_Coef_ModeCF[10][7]` / `SS_Coef_ModeCF[10][7]` | 各280 B | Page5-9 / Page12 | No.21-30 の staging（pack 対象の実体） |
| `MLSS_SP_A/B/C` ほか計9個 | 各4 B | Page4/11/13 `sp_a..c` | スパン校正 2次 `y=A f²+B f+C` |
| `TR_Coef_ModeCF[7]` | 28 B | Page14 `pow_a_cal`,`pow_b_cal` | 透視度 校正後 累乗式 |
| `TR_Coef_ModeCF_Ship[7]` | 28 B | Page13 `pow_a`,`pow_b` | 透視度 出荷時ベース 累乗式 |
| `MLSS/SS/TR_Cal_SetVal_1/2` | 各4 B | Page15 | 校正点濃度 |
| `Depth_k` | 4 B | Page1 `k_depth` | 水深換算 傾き。`Depth=(P−P_atm0)×k_depth` |

### 2-1. ⚠ `TR_Mode_CF[30][7]`（840 B）に保存経路が無い

`MLSS_Mode_CF` / `SS_Mode_CF` は pack/unpack されるが、**`TR_Mode_CF` はどちらにも現れない**
（`IM_110.c:204` 宣言、840 B を RAM に占有）。透視度は相関式選択が無く累乗式を使うため実質デッドの可能性が高いが、
`--gc-sections` で消えていない＝どこかから参照されている。**要確認**。

---

## 3. 本体EEPROM：生きている保存先

| ページ | 関数 | 変数 |
|---|---|---|
| **page4** 情報 | `read/write_param_info` | `EEP_Info_flag` `EEP_Read_Err` `EEP_Read_Retry` `EEP_Write_Err` `EEP_Write_Retry` `ErrDate` `ErrNo` `LastDate` `RTC_Read_Err` `RTC_Read_Retry` `RTC_TSet` `RTC_Write_Err` `RTC_Write_Retry` `Wire_Backup_flag` `Wire_ReadError_Count` `Wire_WriteError_Count`（16個） |
| **page5** 界面 | `read/write_param_interface` | `Interface_Threshold`（1個）★ §5 の衝突あり |
| **page12** 共通 | `read/write_param_common` | `Meas_Mode` `MLSS_MODE` `SS_MODE` `TR_MODE` `MLSS/SS/TR_Cal_SetVal_1/2`（10個） |
| **page81-125** 履歴 | `eep_his_write_bank` / `eep_read_history` | `his[600B]` `Depth` `year` `month` `day` `hour` `min`（7個） |

### 3-1. `SetVal` ×6 が二重保持

`MLSS/SS/TR_Cal_SetVal_1/2` は **page12 と 統合ストア Page15 の両方**に保存経路がある。
`store_adopt_probe()`（`IM_110.c:799`）が unpack 直後に `write_param_common()` を呼ぶため、
プローブ側の値が本体 page12 を上書きする＝実質プローブが勝ち、page12 側は冗長。

---

## 4. 本体EEPROM：死んでいる保存先（呼び出し 0 箇所）

以下 4 関数は**どこからも呼ばれていない**。ここにしか保存経路が無い **36 変数は実質保存されない**。

| ページ | 関数 | 変数 |
|---|---|---|
| page0 | `read/write_param_do` | `ADH[12B]` `ADzr[12B]` |
| page1,2 | `read/write_param_temp` | `SETzr` `SETsp` `SETmid` `THzr` `THsp` `TLzr` `TLsp` `TOzr` `TOsp` `TOffset` `TPzr` `TPsp` `TPmid` `Y0mid` `Y0over` |
| page3 | `read/write_param_setting` | `App_DL_disp_flag` `AutoCal_time_sel` `DOzr` `DOsp` `RTC_ResetCount` `RTC_ResetDate` `RTC_ResetOver` `RTC_ResetYear` `ZEROCAL_flag` `dispACAL_flag` `guide_disp_flag[5B]` `measure_mode_flag` `select_JIS` `tansui_sw_flag` |
| page10 | `read/write_param_logger` | `log_sampling_gap` `log_sel_index` `log_start_date` `log_start_time` `log_timer_flag` |

いずれも ID-200T（DO計・温度計）由来。IM-110 では使わない。

---

## 5. 本体EEPROM：生きているが撤去対象の 1Wire レガシー（48 変数）

`read/write_param_1wireinfo`（**page5,6,7,8**）と `read/write_param_1wirebk`（**page9**）。
`Eeprom.h` の v2.0 変更履歴（2026-05-13）が「旧 1Wire 領域 (page 5-9) を**廃止**し前詰め」と書いているにもかかわらず、
関数も呼び出しも残っている。

**page5 は `Interface_Threshold` と衝突する。** 呼び出し側の並びは以下のとおりで、
`eep_write_adjust()` が page5 に正しい界面設定を書いた直後に、1Wire データで上書きしている。

```
Adjust.c:553-555 / mainSub.c:1100-1101 / LinkSerial.c:1428-1431
    eep_write_adjust();      // 内部 2115 で write_param_interface() → page5
    eep_write_1wireinfo();   // page5,6,7,8 を 1Wire データで上書き ★
    eep_write_1wirebk();     // page9
```

対象変数: `WAG_*` 42個 / `WIRE_*` 6個 / `Stbl_STD`

### 5-1. ⚠ 撤去時の注意: `Stbl_STD`

`Stbl_STD`(4B) は 1Wire グループ（`read_param_1wireinfo`）に紛れているが、名前からして安定判断の基準値であり、
IM-110 でも使う可能性がある。**1Wire 撤去でこれの保存先が消える**。撤去前に用途を確認すること。

---

## 6. 保存経路が無い 338 変数のうち、調整・校正に関わるもの

| 変数 | サイズ | 宣言 | 内容 |
|---|---|---|---|
| `Adb_Ref_MLSS2/3` `Adb_Ref_SS2/3` `Adb_Ref_TR2/3` | 各4 B | IM_110.c:291-296 | **ADBOAD 出荷時3点の基準器設定値**。電源で初期値へ |
| `Depth_offset` | 4 B | IM_110.c:1350 | **電源ON時の大気圧スナップショット**。ADBOAD 出荷時ゼロ・現場界面ゼロ・自動ゼロの3用途が共有 |
| `Depth_offset_set` | 1 B | IM_110.c:1351 | 上記の初期化済みフラグ |
| `stbwidthA` / `stbwidthB` | 各4 B | AutoStable.c:23 | 安定判断の幅（±10%）。低値域フロア調整の対象 |
| `stbtime` | 1 B | AutoStable.c:24 | 安定判断の時間 |
| `Stbl_sel` `CalStagger` `CalTimeStable` `req_auto_stbl_flag` | — | AutoStable.h | 安定判断の設定群 |
| `TR_Mode_CF[30][7]` | 840 B | IM_110.c:204 | §2-1 参照 |
| `MLSS_CAL_REF` `SS_CAL_REF` `TR_CAL_REF` | 各4 B | IM_110.c:103,199,252 | スパン校正の基準値（校正中の一時値） |
| `Cal_Type` | 1 B | IM_110.c:88 | 校正種別（実行中の状態） |
| `adj_x[48B]` `adj_y[48B]` `adj_n` `adj_tc_ref[12B]` `adj_tc_juko[12B]` `adj_tc_set` `adj_tc_mode` `adj_buf_mode` | — | IM_110.c:1643-1650 | 調整の捕捉バッファ（確定前の作業領域。揮発で正しい） |
| `Interface_Hold` `MLSS_Hold` `SS_Hold` `TR_Hold` `DO_Hold` | 各4 B | IM_110.c:37,83-85 | ホールド値（揮発で正しい） |
| `Probe_Store_BootResult` `Probe_Store_L2Applied` `Probe_Conn_Status` `Probe_Data_Valid` | 各1 B | IM_110.c | 起動時ロード結果・疎通状態（揮発で正しい） |

残りは UI 状態・タイマ・表示バッファ・通信バッファで、**保存対象ではない**。

---

## 7. 本監査でまだ埋まっていない部分

- 338 個の非保存変数のうち、§6 に挙げた以外（UI/表示/タイマ/通信）は**個別に用途を辿っていない**。
  名前と宣言ファイルからの分類であり、そこに保存すべきものが紛れている可能性は排除できていない。
- `TR_Mode_CF` が何から参照されているか未確認（§2-1）。
- `Stbl_STD` の用途未確認（§5-1）。
