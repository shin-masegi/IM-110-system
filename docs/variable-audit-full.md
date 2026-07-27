# 全変数 総ざらい — 保存先・用途・参照箇所（ELF 起点、全 512 件）

> 作成: 2026-07-27 / **更新: 同日 ID-200T DO 撤去後に再取得**。
> DO 撤去で本体変数は 512 → 454 になった（実削除 50 個、差分は §1-2）。
>**本体 454 変数・プローブ 50 変数を1件も省略せず掲載する。**
> 母数は ELF のシンボルテーブル。ソースの正規表現抽出は使っていない（3 度壊れたため破棄）。

## 0. 再現手順

```bash
cd ../IM-110 && make -j && cd ../IM-110_Probe && make -j
# 母数（SRAM に確保された全変数。型文字で絞るとAD_AVE_Countのような例外を落とすのでアドレス帯で絞る）
arm-none-eabi-nm --defined-only -S -l build/IM-110.elf | grep -E '^20[0-9a-f]{6} '
```

用途の判定は Python スクリプトで全ソースを走査し、変数ごとに **書込を行う行／読出を行う行／宣言行のコメント** を
収集して行った。表の「書込」「読出」列はその件数。

**限界（先に明示する）**: 行番号の特定は正確だが、**関数名の帰属には誤りがある**。
例: `Depth_offset` を `Calc_Transparency` が書くと出るが、実際の書込は `Calc_Depth:1363` と
`Depth_Calib_Zero:1380` の 2 箇所のみ。関数名は参考値として扱い、断定には行番号を確認すること。

---

## 1. 結論サマリ

| 分類 | 件数 | 判定 |
|---|---|---|
| 保存済（保存経路あり） | 111 | §2 の内訳参照 |
| ★要判断 → 個別に追跡した | 141 | §3 で全件裁定 |
| ★未参照（デッド候補） | 9 | §3-6 |
| FW固定テーブル／読出専用 | 8 | 書込 0 件。保存不要 |
| エラーカウンタ | 34 | page4 に集約済 or 揮発 |
| タイマ／カウンタ | 44 | 揮発で正しい |
| UI 状態 | 20 | 揮発で正しい |
| 通信バッファ／状態 | 19 | 揮発で正しい |
| 関数内 static | 38 | 関数スコープ。揮発で正しい |
| HAL/libc | 30 | 対象外 |
| **合計** | **454** | |

---

## 2. 保存が必要なのに保存されていないもの（結論）

| 変数 | B | 宣言 | 現状 | 必要な対応 |
|---|---|---|---|---|
| `LED_Out[1..4]`（プローブ） | 16 | IM_110.c:38 | Page1 に `[0]` のみ | LED PWM 2系統化に必須 |
| `Adb_Ref_MLSS2/3` `Adb_Ref_SS2/3` `Adb_Ref_TR2/3` | 24 | IM_110.c:291-296 | 保存なし | ADBOAD 出荷時3点の基準器値。電源で初期値へ戻る |
| `Depth_offset` | 4 | IM_110.c:1350 | 保存なし・3用途で共有 | 出荷時ゼロを分離して保存。§3-1 |
| `AD_AVE_Count`（プローブ） | 1 | IM_110.c:22 | Page2 に枠のみ未結線 | 本体が毎起動 `SADA,0` を送るため実害は限定的。枠ごと削除が妥当 |
| `mcp3424_ch_mask`（プローブ） | 1 | IM_110.c:23 | Page2 に枠のみ未結線 | 同上 |
| `Stbl_STD` | 4 | mainSub.h:102 | **1Wire レガシー page5-8 にのみ保存** | 撤去すると保存先が消える。§3-2 |
| `stbwidthA` `stbwidthB` `stbtime` `stbsize` `Stbl_sel` | 13 | AutoStable.c:22-24 ほか | 保存なし | 安定判断の設定。現状 FW 固定でよいか要決定 |

---

## 3. ★要判断 162 件の裁定

### 3-1. `Depth_offset` / `Depth_offset_set` — 保存が要る（用途分離が前提）

書込は `Calc_Depth:1363`（電源ON初回の自動ゼロ）と `Depth_Calib_Zero:1380` の 2 箇所のみ。
`Depth_Calib_Zero()` は **現場の界面ゼロ校正（Normal.c:1214）と ADBOAD 出荷時ゼロ（Adjust.c:997）の
両方から呼ばれる**ため、3 用途が 1 変数を共有している。`Depth_Calib_Span6m():1393` もこの値を基準にする。
→ 出荷時ゼロを別変数に分離し、そちらを保存する。電源ON大気圧スナップショットは揮発のままでよい。

### 3-2. `Stbl_STD` — 保存先が 1Wire レガシーにしかない

コメント「自動安定判断条件の切替値（高／低濃度）」。読出は `normal_disp` と `start_measure`
＝ **IM-110 の測定表示で現役**。書込は `read_param_1wireinfo`（page5-8）/`WAG_HeaderRead`/`Default_InfoData_Set`。
→ 1Wire 撤去時に保存先が消える。移設先を決めること。

### 3-3. ID-200T 由来の DO 計・温度計変数（16 件）— 生きた経路から書かれている

`AD`(センサー出力) `AD20`(20℃補正DO) `AIR_PER`(空気飽和率) `DO` `DOH`(塩化物イオン補正後飽和DO)
`DOAcc`(DO表示小数桁) `DO_Hold` `FCL`(塩分補正) `FDOH`(飽和DO) `FH`(温度特性補正後DOmV)
`FTH`/`FTL`(温度素子変換式) `R`(レンジ毎AD) `WA_F`/`WA_T`(流速補正) `WTemp`(温度) `Y0`/`Y1_`/`Y2`(アナログmV)

これらを書く `start_measure()` は **`Normal.c:1089` `Normal.c:2606` `Setting.c:268` から呼ばれており、
IM-110 の測定 UI から到達する**。デッドコードではない。
さらに `do_syori4()`/`do_syori5()`（DO の水温20℃/35℃調整係数）が **`Adjust.c:2093`/`2099` から呼ばれており、
ADBOAD に ID-200T の水温調整画面が残っている**。

→ **これらは保存対象ではない**（計算中の中間値）。ただし ADBOAD 再設計時に、ID-200T の水温調整画面を
残すのか消すのかを決める必要がある。adboad.md には該当画面が無い。

### 3-4. 測定・校正の中間計算値（保存不要、揮発で正しい）

`MLSS/SS/TR_ADC_mV`（A/D補正後mV）、`MLSS/SS/TR_ABSS`（対数中間値）、`MLSS/SS/TR_FABSS`（モード変換後）、
`MLSS/SS/TR_1A`/`1B`（FABSS フル/半スパン校正値）、`MLSS/SS/TR_CAL_REF`（校正基準値＝校正実行中の一時値）、
`MLSS/SS/TR_Hold`（安定ホールド値）、`MLSS_inst`（瞬時値）、`Transparency`、`Interface_Hold`、
`ADC_mV[6]`/`ADC_mV_ave[6]`（プローブ受信値と本体移動平均）、`ad_ave_idx`/`ad_ave_num`。

いずれも毎測定サイクルで再計算される。確定値は統合ストア側に入っているため保存不要。

### 3-5. 調整・校正の作業領域（保存不要、確定時にストアへ入る）

`adj_x[48B]` `adj_y[48B]`（Mode_CF 捕捉点）、`adj_n`（捕捉点数）、`adj_tc_ref`/`adj_tc_juko`（温度補正捕捉）、
`adj_tc_set`（5/20/35℃ 捕捉フラグ）、`adj_tc_mode`、`adj_buf_mode`、`adj_progress_cb`、
`cal_sel_raw_full`/`cal_sel_raw_mid`（校正で選んだ raw mV）、`cal_hsel_*`、`cal_setting_sel`、`corr_sel`、
`span_setting_*`、`adb_busy_*`（ADBOAD 待機アイコン）、`g_hstore`/`g_hmirror`（ストア RAM 像）。

`AWC` 等の確定操作でストアへ pack されるため、作業領域自体は揮発で正しい。

### 3-6. ★未参照 9 件（デッド候補）

`initialized`(qrencode/rsecc.c:41 と衝突表示) ほか、書込・読出とも 0 件と判定されたもの。
関数内 static の名前マングリング（`name.N`）が絡むため、削除前に個別確認が要る。

### 3-7. 安定判断（AutoStable）— 設計判断が要る

`data_stable[1020B]`（サンプルバッファ）`cnt_stable` `cnt_OverWrite` `chk_stable` `f_stable`
`Stagger`/`OldStagger`（ふらつき幅）`TimeStable`/`CalTimeStable`/`CalStagger`（所要時間）は実行時状態＝揮発でよい。
一方 `stbwidthA`/`stbwidthB`（安定判断幅）`stbtime`（表示更新間隔）`stbsize`（サンプル数）`Stbl_sel`（条件選択）は
**判定パラメータ**であり、FW 固定にするか保存して調整可能にするかの決めが要る。
（handoff の T4「低値域フロア調整」は `stbwidthA/B` が対象）

### 3-8. その他（揮発で正しい）

`operation_mode`（画面遷移、W42/R46）`ret_mode` `oc_err_num` `Req_SetupData_Write`（電源OFF記憶要求）
`Probe_Conn_Status` `Probe_Data_Valid` `Probe_Store_BootResult` `Probe_Store_L2Applied` `probe_hs*` `probe_ms_on`
`power_off_flag` `wakeup_flag` `ena_pow` `lcd_*` `v33_*` `bt_*` `use_UART_flag` `force_range*` `now_range`
`WAFG_Flag` `DAFG_Flag` `wire_test_flag` `WAG_*`（1Wire 履歴、撤去対象）ほか。

---


## 1-2. DO 撤去で消えた変数（54 個）

`AD` `AD20` `ADH[3]` `ADzr[3]` `AIR_PER` `DO` `DOAcc` `DOH` `DO_Hold` `DO_buf` `DOsp` `DOzr` `FCL` `FDOH` `FH` `FTH` `FTL` `PowerOn_Amp` `PowerOn_Temp` `R` `SETmid` `SETsp` `SETzr` `THsp` `THzr` `TLsp` `TLzr` `TOffset` `TOsp` `TOzr` `TPmid` `TPsp` `TPzr` `WAG_HstCal_Amp` `WAG_HstCal_Temp` `WAG_HstMeas_Amp` `WAG_HstMeas_Meas` `WAG_HstMeas_Temp` `WA_F` `WA_T` `WTemp` `Y0` `Y0mid` `Y0over` `Y1` `Y1_` `do_flash_flag` `do_stop` `force_range` `force_range_set_flag` `log_dat` `log_head` `now_range` `temp_flash_flag`

`Y2` / `Y2_`（電池電圧、ADBOAD 3 で使用）は残置。
`log_dat` / `log_head` は DO ロガー表示の撤去に伴いリンカが除去した。

## 4. 全 454 件 一覧

分類順・サイズ降順。

| # | 変数 | B | 宣言 | 書込 | 読出 | 用途(宣言コメント/参照関数) | 保存先/判定 |
|---|---|---|---|---|---|---|---|
| 1 | `data_stable` | 1020 | Core/Src/AutoStable.c:25 | 4 | 4 | 安定判断サンプル ("MAX: 60秒×2(件／秒)" ×2倍 ＋15件) | ★要判断  |
| 2 | `g_hmirror` | 512 | Core/Src/IM_110.c:726 | 4 | 4 | W:Probe_Store_BootLoad,Probe_Store_DebugStatus,Probe_Store_Shu R:Probe_Store_BootLoad,Probe_Store_DebugStatus,mirror_app | ★要判断  |
| 3 | `g_hstore` | 512 | Core/Src/IM_110.c:551 | 8 | 4 | W:Mirror_WriteNewDefaults,Probe_FetchStore,Probe_Store_BootLoa R:Probe_SendWaitOK,Probe_Store_BootLoad,Probe_Store_Debug | ★要判断  |
| 4 | `hdma_lpuart_rx` | 72 | Core/Src/main.c:71 | 2 | 4 | W:HAL_UART_MspInit,DMA2_Channel7_IRQHandler R:,,HAL_UART_MspInit, | ★要判断  |
| 5 | `hdma_spi1_tx` | 72 | Core/Src/main.c:76 | 2 | 4 | W:HAL_SPI_MspInit,DMA2_Channel4_IRQHandler R:,,HAL_SPI_MspInit, | ★要判断  |
| 6 | `hdma_usart1_rx` | 72 | Core/Src/main.c:72 | 2 | 4 | W:HAL_UART_MspInit,DMA1_Channel5_IRQHandler R:,,HAL_UART_MspInit, | ★要判断  |
| 7 | `adj_x` | 48 | Core/Src/IM_110.c:1643 | 1 | 5 | Mode_CF 捕捉: x = ABS (対数中間値) | ★要判断  |
| 8 | `adj_y` | 48 | Core/Src/IM_110.c:1644 | 1 | 5 | Mode_CF 捕捉: y = 基準器 mg/L (透視度は cm) | ★要判断  |
| 9 | `ADC_mV` | 24 | Core/Src/IM_110.c:38 | 2 | 13 | Probe 受信値 (瞬時、SADA,0 で平均 OFF 前提) | ★要判断  |
| 10 | `ADC_mV_ave` | 24 | Core/Src/IM_110.c:39 | 2 | 16 | 05 MLSS   (ch1) | ★要判断  |
| 11 | `adj_tc_juko` | 12 | Core/Src/IM_110.c:1648 | 1 | 2 | 温度補正 捕捉 受光 | ★要判断  |
| 12 | `adj_tc_ref` | 12 | Core/Src/IM_110.c:1647 | 1 | 2 | 温度補正 捕捉 Ref  (0=5℃ 1=20℃ 2=35℃) | ★要判断  |
| 13 | `ad_ave_idx` | 6 | Core/Src/IM_110.c:43 | 1 | 2 | W:update_main_ave R:,update_main_ave | ★要判断  |
| 14 | `ad_ave_num` | 6 | Core/Src/IM_110.c:44 | 1 | 2 | バッファに格納済みのサンプル数 (0..MAIN_AD_AVE_COUNT) | ★要判断  |
| 15 | `Adb_Ref_MLSS2` | 4 | Core/Src/IM_110.c:291 | 2 | 3 | 10 設定中の基準器値を表示 | ★要判断  |
| 16 | `Adb_Ref_MLSS3` | 4 | Core/Src/IM_110.c:292 | 2 | 3 | 11 | ★要判断  |
| 17 | `Adb_Ref_SS2` | 4 | Core/Src/IM_110.c:293 | 2 | 3 | 13 | ★要判断  |
| 18 | `Adb_Ref_SS3` | 4 | Core/Src/IM_110.c:294 | 2 | 3 | 14 | ★要判断  |
| 19 | `Adb_Ref_TR2` | 4 | Core/Src/IM_110.c:295 | 2 | 3 | 16 透視度は 0.1cm 刻み | ★要判断  |
| 20 | `Adb_Ref_TR3` | 4 | Core/Src/IM_110.c:296 | 2 | 3 | 17 | ★要判断  |
| 21 | `CalStagger` | 4 | Core/Inc/AutoStable.h:19 | 2 | 3 | *10-45   ふらつき状態 | ★要判断  |
| 22 | `CalTimeStable` | 4 | Core/Inc/AutoStable.h:20 | 2 | 2 | （現在までの）所要時間セット | ★要判断  |
| 23 | `Depth_offset` | 4 | Core/Src/IM_110.c:1350 | 3 | 3 | 電源ON大気圧基準の相対気圧 | ★要判断  |
| 24 | `Interface_Hold` | 4 | Core/Src/IM_110.c:37 | 2 | 4 | 界面深度ホールド (初期値は FLT_MAX = 未捕捉、表示側で flash 白塗りされる) | ★要判断  |
| 25 | `MLSS_1A` | 4 | Core/Src/IM_110.c:101 | 3 | 4 | MLSS FABSSフルスパン校正値 | ★要判断  |
| 26 | `MLSS_1B` | 4 | Core/Src/IM_110.c:102 | 3 | 4 | MLSS FABSS 1/2スパン校正値 | ★要判断  |
| 27 | `MLSS_ABSS` | 4 | Core/Src/IM_110.c:96 | 1 | 3 | MLSS 対数中間値 | ★要判断  |
| 28 | `MLSS_ADC_mV` | 4 | Core/Src/IM_110.c:95 | 1 | 3 | MLSS A/D補正後の値 (mV) | ★要判断  |
| 29 | `MLSS_CAL_REF` | 4 | Core/Src/IM_110.c:103 | 2 | 3 | MLSS スパン校正基準値 (設定濃度) | ★要判断  |
| 30 | `MLSS_FABSS` | 4 | Core/Src/IM_110.c:97 | 1 | 5 | MLSS モード変換後の値 | ★要判断  |
| 31 | `MLSS_Hold` | 4 | Core/Src/IM_110.c:83 | 3 | 2 | 安定ホールドMLSS値 | ★要判断  |
| 32 | `MLSS_inst` | 4 | Core/Src/IM_110.c:33 | 1 | 5 | 移動平均無しの瞬時 MLSS (界面判断バー用) | ★要判断  |
| 33 | `OldStagger` | 4 | Core/Inc/AutoStable.h:18 | 3 | 2 | （現時点の）ふらつき幅セット | ★要判断  |
| 34 | `SS_1A` | 4 | Core/Src/IM_110.c:197 | 2 | 4 | SS FABSSフルスパン校正値 | ★要判断  |
| 35 | `SS_1B` | 4 | Core/Src/IM_110.c:198 | 2 | 3 | SS FABSS 1/2スパン校正値 | ★要判断  |
| 36 | `SS_ABSS` | 4 | Core/Src/IM_110.c:192 | 1 | 2 | SS 対数中間値 | ★要判断  |
| 37 | `SS_ADC_mV` | 4 | Core/Src/IM_110.c:191 | 1 | 3 | SS A/D補正後の値 (mV) | ★要判断  |
| 38 | `SS_CAL_REF` | 4 | Core/Src/IM_110.c:199 | 1 | 3 | SS スパン校正基準値 (設定濃度) | ★要判断  |
| 39 | `SS_FABSS` | 4 | Core/Src/IM_110.c:193 | 1 | 3 | SS モード変換後の値 | ★要判断  |
| 40 | `SS_Hold` | 4 | Core/Src/IM_110.c:84 | 3 | 2 | 安定ホールドSS値 | ★要判断  |
| 41 | `Stagger` | 4 | Core/Inc/AutoStable.h:18 | 2 | 3 | *10-45   ふらつき状態 | ★要判断  |
| 42 | `TR_1A` | 4 | Core/Src/IM_110.c:250 | 2 | 4 | TR FABSSフルスパン校正値 | ★要判断  |
| 43 | `TR_1B` | 4 | Core/Src/IM_110.c:251 | 2 | 3 | TR FABSS 1/2スパン校正値 | ★要判断  |
| 44 | `TR_ABSS` | 4 | Core/Src/IM_110.c:245 | 1 | 2 | TR 対数中間値 | ★要判断  |
| 45 | `TR_ADC_mV` | 4 | Core/Src/IM_110.c:244 | 1 | 3 | TR A/D補正後の値 (mV) | ★要判断  |
| 46 | `TR_CAL_REF` | 4 | Core/Src/IM_110.c:252 | 1 | 3 | TR スパン校正基準値 (設定濃度) | ★要判断  |
| 47 | `TR_FABSS` | 4 | Core/Src/IM_110.c:246 | 1 | 3 | TR モード変換後の値 | ★要判断  |
| 48 | `TR_Hold` | 4 | Core/Src/IM_110.c:85 | 3 | 2 | 安定ホールドTR値 | ★要判断  |
| 49 | `TimeStable` | 4 | Core/Inc/AutoStable.h:20 | 3 | 2 | （現在までの）所要時間セット | ★要判断  |
| 50 | `Transparency` | 4 | Core/Src/IM_110.c:34 | 8 | 16 | 15 | ★要判断  |
| 51 | `WAG_HstCal_Stable` | 4 | Core/Inc/mainSub.h:177 | 1 | 2 | *10-45   ふらつき状態 | ★要判断  |
| 52 | `WAG_HstCal_Time` | 4 | Core/Inc/mainSub.h:176 | 1 | 2 | *10-45   所要時間 | ★要判断  |
| 53 | `WAG_HstMeas_Stable` | 4 | Core/Inc/mainSub.h:185 | 1 | 2 | ふらつき具合 | ★要判断  |
| 54 | `WAG_HstMeas_Time` | 4 | Core/Inc/mainSub.h:184 | 1 | 2 | 所要時間 | ★要判断  |
| 55 | `Y2` | 4 | Core/Inc/Adc.h:25 | 1 | 5 | アナログ電圧値mV Y2:電池 (Y0:水温/Y1:DO は DO 撤去で削除) | ★要判断  |
| 56 | `adb_busy_val` | 4 | Core/Src/Adjust.c:916 | 1 | 2 | 表示中の値 | ★要判断  |
| 57 | `adj_n` | 4 | Core/Src/IM_110.c:1645 | 3 | 6 | Mode_CF 捕捉点数 | ★要判断  |
| 58 | `adj_progress_cb` | 4 | Core/Src/IM_110.c:1788 | 1 | 2 | W:Adj_SetProgressCallback R:Probe_Read_LED_Duty,adj_delay_progress | ★要判断  |
| 59 | `cal_sel_raw_full` | 4 | Core/Src/Normal.c:74 | 2 | 3 | 選択したフルスパン点 raw mV (ゼロ校正前) | ★要判断  |
| 60 | `cal_sel_raw_mid` | 4 | Core/Src/Normal.c:75 | 2 | 2 | 選択した中間点 raw mV (ゼロ校正前) | ★要判断  |
| 61 | `cnt_stable` | 4 | Core/Src/AutoStable.c:26 | 2 | 4 | 安定判断のデータ位置 | ★要判断  |
| 62 | `f_ad` | 4 | Core/Inc/Adc.h:28 | 1 | 2 | 移動平均計算用 | ★要判断  |
| 63 | `f_ad_data` | 4 | Core/Inc/Adc.h:29 | 1 | 2 | 移動平均計算用 | ★要判断  |
| 64 | `initialized` | 4 | qrencode/rsecc.c:41 | 1 | 1 | W:Update_Interface_Hold R:Update_Interface_Hold | ★要判断  |
| 65 | `log_interval_setting` | 4 | Core/Inc/Timer.h:67 | 1 | 1 | ロガー測定間隔（x50ms） | ★要判断  |
| 66 | `probe_hs_t0` | 4 | Core/Src/IM_110.c:988 | 3 | 1 | W:Probe_Boot_Start,Probe_Boot_Tick,Probe_Store_ShutdownWriteBa R:Probe_Boot_Tick | ★要判断  |
| 67 | `span_setting_target` | 4 | Core/Src/Normal.c:82 | 3 | 1 | 流用時のターゲットを解除 (値は書かない) | ★要判断  |
| 68 | `stbsize` | 4 | Core/Src/AutoStable.c:22 | 2 | 6 | 安定判断サンプル数 | ★要判断  |
| 69 | `stbwidthA` | 4 | Core/Src/AutoStable.c:23 | 2 | 2 | W:calcu_stable,calcu_stable_CAL R:,check_stable | ★要判断  |
| 70 | `stbwidthB` | 4 | Core/Src/AutoStable.c:23 | 2 | 2 | 安定判断値 | ★要判断  |
| 71 | `RTC_ResetDate` | 3 | Core/Inc/mainSub.h:196 | 3 | 4 | RTCチェック用の製造日 = "00/01/01" | ★要判断  |
| 72 | `WAG_HstCal_Date` | 3 | Core/Inc/mainSub.h:172 | 1 | 2 | 校正日(年) | ★要判断  |
| 73 | `WAG_HstMeas_Date` | 3 | Core/Inc/mainSub.h:180 | 1 | 2 | 記録日 (年) | ★要判断  |
| 74 | `log_start_date` | 3 | Core/Inc/mainSub.h:344 | 3 | 7 | ロガー開始年 | ★要判断  |
| 75 | `adjust_mode` | 2 | Core/Inc/mainSub.h:336 | 3 | 3 | プログラムバージョン表示 | ★要判断  |
| 76 | `cal_hsel_count` | 2 | Core/Src/Normal.c:73 | 2 | 1 | 有効な履歴件数 | ★要判断  |
| 77 | `log_sampling_gap` | 2 | Core/Inc/mainSub.h:346 | 3 | 6 | ロガー測定間隔 | ★要判断  |
| 78 | `operation_mode` | 2 | Core/Inc/mainSub.h:65 | 39 | 42 | EEPROMテストに移る | ★要判断  |
| 79 | `ret_mode` | 2 | Core/Inc/Normal.h:68 | 15 | 4 | センサー差し替えでOKの時復帰可能に変更 2025/10/28 柵木 | ★要判断  |
| 80 | `span_setting_return` | 2 | Core/Src/Normal.c:77 | 4 | 1 | W:,nrm_adjust_span,nrm_adjust_span_m,nrm_span_setting_begin R:nrm_span_setting | ★要判断  |
| 81 | `Cnt_After1wire` | 1 | Core/Inc/Wire.h:22 | 2 | 1 | 1wire操作後のA/D変換待ち時間セット | ★要判断  |
| 82 | `Comm_PowerOff_flag` | 1 | Core/Inc/mainSub.h:335 | 2 | 2 | コマンドによる電源OFFを要求 | ★要判断  |
| 83 | `DAFG_Flag` | 1 | Core/Inc/mainSub.h:219 | 2 | 7 | DAFG判別 | ★要判断  |
| 84 | `Depth_offset_set` | 1 | Core/Src/IM_110.c:1351 | 3 | 1 | W:Calc_Depth,Calc_Transparency,Depth_Calib_Zero R:Calc_Depth | ★要判断  |
| 85 | `EEP_RetryCount_Read` | 1 | Core/Inc/mainSub.h:98 | 10 | 6 | 読み込みリトライの発生回数 | ★要判断  |
| 86 | `EEP_RetryCount_Write` | 1 | Core/Inc/mainSub.h:99 | 9 | 7 | 書き込みリトライの発生回数 | ★要判断  |
| 87 | `MEM_lp` | 1 | Core/Inc/Setting.h:22 | 2 | 2 | MEM長押し中フラグ | ★要判断  |
| 88 | `Probe_Conn_Status` | 1 | Core/Src/IM_110.c:315 | 3 | 3 | 0=unknown / 1=connected / 2=NG(未応答) | ★要判断  |
| 89 | `Probe_Data_Valid` | 1 | Core/Src/IM_110.c:316 | 2 | 2 | 1=測定値有効 / 0=未受信 or 鮮度切れ(→ "----" 表示) | ★要判断  |
| 90 | `Probe_MS_started` | 1 | Core/Src/IM_110.c:312 | 4 | 1 | W:Probe_EnsureMS_On,Probe_Request_MD,Probe_ResumeMS,update_mai R:Probe_Request_MD | ★要判断  |
| 91 | `Probe_Store_BootResult` | 1 | Core/Src/IM_110.c:728 | 3 | 2 | 起動時3層ロードの結果 (AMIR で観測)。0xFF=未実行 | ★要判断  |
| 92 | `Probe_Store_L2Applied` | 1 | Core/Src/IM_110.c:729 | 2 | 2 | 起動時に層2(ミラー)を live へ適用したか (AMIR で観測) | ★要判断  |
| 93 | `RTC_ResetCount` | 1 | Core/Inc/mainSub.h:193 | 2 | 2 | RTCリセット回数 = 0 | ★要判断  |
| 94 | `RTC_ResetOver` | 1 | Core/Inc/mainSub.h:195 | 2 | 2 | RTCリセット回数ｵｰﾊﾞｰ有無フラグ = 0(取得要求) | ★要判断  |
| 95 | `RTC_ResetYear` | 1 | Core/Inc/mainSub.h:194 | 2 | 2 | RTCチェック用の年カウント = 0 | ★要判断  |
| 96 | `Req_SetupData_Write` | 1 | Core/Inc/mainSub.h:75 | 17 | 2 | 設定情報の記憶要求フラグ（電源OFF時記憶） | ★要判断  |
| 97 | `WAFG_Flag` | 1 | Core/Inc/mainSub.h:218 | 2 | 2 | WAFG判別 | ★要判断  |
| 98 | `WAG_HstCal_Type` | 1 | Core/Inc/mainSub.h:173 | 1 | 2 | 校正結果 | ★要判断  |
| 99 | `WAG_RestDay` | 1 | Core/Inc/mainSub.h:74 | 1 | 3 | センサー有効期限残日数 | ★要判断  |
| 100 | `WAG_Type` | 1 | Core/Inc/mainSub.h:124 | 1 | 4 | センサー種別 0:TL 1:TLB | ★要判断  |
| 101 | `ZEROCAL_flag` | 1 | Core/Inc/mainSub.h:290 | 1 | 1 | ゼロ校正係数更新フラグ | ★要判断  |
| 102 | `adb_busy_digit` | 1 | Core/Src/Adjust.c:917 | 1 | 2 | W:adj_probe R:adj_probe_max_mv,adj_probe_progress | ★要判断  |
| 103 | `adb_busy_frame` | 1 | Core/Src/Adjust.c:919 | 2 | 2 | 0/1 トグル | ★要判断  |
| 104 | `adb_busy_scr` | 1 | Core/Src/Adjust.c:915 | 1 | 2 | 表示中の画面番号 | ★要判断  |
| 105 | `adb_busy_unit` | 1 | Core/Src/Adjust.c:918 | 1 | 2 | W:adj_probe R:adj_probe_max_mv,adj_probe_progress | ★要判断  |
| 106 | `adj_tc_mode` | 1 | Core/Src/IM_110.c:1650 | 3 | 2 | 温度補正 捕捉のモード | ★要判断  |
| 107 | `adj_tc_set` | 1 | Core/Src/IM_110.c:1649 | 3 | 2 | 温度補正 捕捉フラグ (bit0/1/2 = 5/20/35℃) | ★要判断  |
| 108 | `auto_adjust_flag` | 1 | Core/Inc/mainSub.h:334 | 3 | 2 | 基板手動調整中(通常動作) | ★要判断  |
| 109 | `cal_from_powerOn` | 1 | Core/Inc/Normal.h:25 | 1 | 3 | 電源 ON 経由フラグ: 校正メニューを「ゼロ/2点/3点/リセット」4 ボタン配置に切替 | ★要判断  |
| 110 | `cal_hsel_purpose` | 1 | Core/Src/Normal.c:71 | 4 | 2 | 3点中間点 | ★要判断  |
| 111 | `cal_setting_sel` | 1 | Core/Src/Normal.c:777 | 4 | 4 | 校正モード選択: 0=ZCAL, 1=ADCAL(2点), 2=MCAL(3点) | ★要判断  |
| 112 | `chk_stable` | 1 | Core/Inc/AutoStable.h:17 | 2 | 2 | 安定データの有無 | ★要判断  |
| 113 | `cnt_OverWrite` | 1 | Core/Src/AutoStable.c:27 | 2 | 4 | cnt_stableのリセット有無 | ★要判断  |
| 114 | `corr_sel` | 1 | Core/Src/Normal.c:39 | 1 | 3 | 相関式選択: 0〜9 | ★要判断  |
| 115 | `ena_pow` | 1 | Core/Src/mainSub.c:494 | 2 | 1 | Powerボタン有効フラグ | ★要判断  |
| 116 | `f_stable` | 1 | Core/Inc/AutoStable.h:16 | 2 | 4 | 不安定状態フラグ | ★要判断  |
| 117 | `fl_flag` | 1 | Core/Inc/Setting.h:21 | 16 | 16 | 選択カーソル点滅フラグ（設定画面用） | ★要判断  |
| 118 | `fl_flag2` | 1 | Core/Inc/Adjust.h:20 | 3 | 4 | 選択カーソル点滅フラグ（調整画面用） | ★要判断  |
| 119 | `lcd_extcomin` | 1 | Core/Inc/mainSub.h:306 | 1 | 2 | LCD交流化信号用フラグ | ★要判断  |
| 120 | `lcd_on_flag` | 1 | Core/Inc/mainSub.h:360 | 3 | 7 | LCD電源 ONフラグ | ★要判断  |
| 121 | `log_sel_index` | 1 | Core/Inc/mainSub.h:342 | 3 | 5 | ロガー履歴No. | ★要判断  |
| 122 | `logger_sw_stop_flag` | 1 | Core/Inc/mainSub.h:371 | 2 | 2 | データ履歴を表示したら電源OFFしないようにする | ★要判断  |
| 123 | `measure_mode_flag` | 1 | Core/Inc/mainSub.h:61 | 6 | 5 | 測定モード選択 0:通常測定 | ★要判断  |
| 124 | `oc_err_num` | 1 | Core/Inc/Normal.h:67 | 17 | 3 | 発生エラー番号 | ★要判断  |
| 125 | `power_off_flag` | 1 | Core/Inc/mainSub.h:328 | 1 | 2 | 電源OFFフラグ | ★要判断  |
| 126 | `probe_hs` | 1 | Core/Src/IM_110.c:987 | 3 | 2 | 未起動時は DONE 扱い | ★要判断  |
| 127 | `probe_hs_retry` | 1 | Core/Src/IM_110.c:989 | 3 | 1 | W:Probe_Boot_Start,Probe_Boot_Tick,Probe_Store_ShutdownWriteBa R:Probe_Boot_Tick | ★要判断  |
| 128 | `probe_ms_on` | 1 | Core/Src/IM_110.c:321 | 5 | 1 | 0=OFF(電源ONベースライン) / 1=ON | ★要判断  |
| 129 | `qr_update_flag` | 1 | Core/Inc/mainSub.h:301 | 2 | 2 | LCD表示更新用フラグ（2sec） | ★要判断  |
| 130 | `req_auto_stbl_flag` | 1 | Core/Inc/AutoStable.h:22 | 3 | 3 | 判定要求フラグ | ★要判断  |
| 131 | `req_hst` | 1 | Core/Inc/mainSub.h:73 | 2 | 2 | 測定履歴の記憶許可フラグ | ★要判断  |
| 132 | `select_JIS` | 1 | Core/Inc/mainSub.h:59 | 2 | 2 | 新旧JIS選択＝新JIS（未使用） | ★要判断  |
| 133 | `span_setting_chain_mid` | 1 | Core/Src/Normal.c:78 | 4 | 1 | W:,nrm_adjust_span,nrm_adjust_span_m,nrm_span_setting_begin R:nrm_span_setting | ★要判断  |
| 134 | `span_setting_kind` | 1 | Core/Src/Normal.c:83 | 2 | 2 | W:,nrm_span_setting_begin R:,nrm_span_setting | ★要判断  |
| 135 | `stbtime` | 1 | Core/Src/AutoStable.c:24 | 1 | 2 | 安定中の表示更新間隔 | ★要判断  |
| 136 | `tansui_sw_flag` | 1 | Core/Inc/mainSub.h:58 | 4 | 11 | 淡水／海水選択＝淡水 | ★要判断  |
| 137 | `use_UART_flag` | 1 | Core/Inc/mainSub.h:333 | 2 | 6 | UARTを使用可能にする | ★要判断  |
| 138 | `v33_on_flag` | 1 | Core/Inc/mainSub.h:361 | 3 | 5 | 3.3V電源 ONフラグ | ★要判断  |
| 139 | `wakeup_flag` | 1 | Core/Inc/mainSub.h:368 | 2 | 2 | 測定前準備フラグ | ★要判断  |
| 140 | `wire_test_flag` | 1 | Core/Inc/mainSub.h:228 | 3 | 3 | W:main,,start_measure R:(宣言のみ),,start_measure | ★要判断  |
| 141 | `wn_poff_flag` | 1 | Core/Inc/mainSub.h:370 | 3 | 2 | WN受信時の定期監視フラグ 0:定期監視外 1:定期監視中 | ★要判断  |
| 142 | `generator` | 899 | qrencode/rsecc.c:54 | 0 | 0 | W:- R:- | ★未参照 デッド候補 |
| 143 | `aindex` | 256 | qrencode/rsecc.c:53 | 0 | 0 | W:- R:- | ★未参照 デッド候補 |
| 144 | `alpha` | 256 | qrencode/rsecc.c:52 | 0 | 0 | W:- R:- | ★未参照 デッド候補 |
| 145 | `tzinfo` | 88 |  | 0 | 0 | W:- R:- | ★未参照 デッド候補 |
| 146 | `generatorInitialized` | 29 | qrencode/rsecc.c:55 | 0 | 0 | W:- R:- | ★未参照 デッド候補 |
| 147 | `_tzname` | 8 |  | 0 | 0 | W:- R:- | ★未参照 デッド候補 |
| 148 | `_daylight` | 4 |  | 0 | 0 | W:- R:- | ★未参照 デッド候補 |
| 149 | `_timezone` | 4 |  | 0 | 0 | W:- R:- | ★未参照 デッド候補 |
| 150 | `prev_tzenv` | 4 |  | 0 | 0 | W:- R:- | ★未参照 デッド候補 |
| 151 | `TR_Mode_CF` | 840 | Core/Src/IM_110.c:204 | 0 | 3 | W:- R:(宣言のみ),TR_FABSS_from_raw_mv,update_main_ave | FW固定テーブル/読出専用 書込0 → 保存不要 |
| 152 | `cal` | 600 | Core/Inc/mainSub.h:287 | 0 | 6 | 校正履歴データ | FW固定テーブル/読出専用 書込0 → 保存不要 |
| 153 | `ad_ave_buf` | 240 | Core/Src/IM_110.c:42 | 0 | 2 | W:- R:,update_main_ave | FW固定テーブル/読出専用 書込0 → 保存不要 |
| 154 | `pre_cal` | 20 | Core/Inc/mainSub.h:288 | 0 | 2 | 前回校正成功データ | FW固定テーブル/読出専用 書込0 → 保存不要 |
| 155 | `WAG_SNo_CAL` | 4 | Core/Inc/mainSub.h:294 | 0 | 2 | 校正時センサーNo. | FW固定テーブル/読出専用 書込0 → 保存不要 |
| 156 | `adc_data` | 2 | Core/Inc/Adc.h:27 | 0 | 3 | A/Dデータ格納用 0:電池 | FW固定テーブル/読出専用 書込0 → 保存不要 |
| 157 | `Cal_Type` | 1 | Core/Src/IM_110.c:88 | 0 | 4 | 校正種別(0:ゼロ, 1:2pスパン, 2:3p中間) | FW固定テーブル/読出専用 書込0 → 保存不要 |
| 158 | `Stbl_sel` | 1 | Core/Inc/AutoStable.h:15 | 0 | 2 | 自安定判断条件の選択 | FW固定テーブル/読出専用 書込0 → 保存不要 |
| 159 | `MLSS_Mode_CF` | 840 | Core/Src/IM_110.c:114 | 0 | 9 | W:- R:(宣言のみ),Adj_ModeCF_FitShip,Apply_Coef_To_Live,MLSS_FABSS_from | 保存済 STORE |
| 160 | `SS_Mode_CF` | 840 | Core/Src/IM_110.c:151 | 0 | 8 | W:- R:(宣言のみ),Adj_ModeCF_FitShip,Apply_Coef_To_Live,SS_FABSS_from_r | 保存済 STORE |
| 161 | `his` | 600 | Core/Inc/mainSub.h:244 | 5 | 10 | 測定履歴データ (現在選択中の種別バンクを保持) | 保存済 HIST |
| 162 | `MLSS_Coef_ModeCF` | 280 | Core/Src/IM_110.c:262 | 0 | 7 | W:- R:(宣言のみ),Adj_ModeCF_FitPoly,Apply_Coef_To_Live,adj_copy_base_t | 保存済 STORE |
| 163 | `SS_Coef_ModeCF` | 280 | Core/Src/IM_110.c:276 | 0 | 7 | W:- R:(宣言のみ),Adj_ModeCF_FitPoly,Apply_Coef_To_Live,adj_copy_base_t | 保存済 STORE |
| 164 | `eep_rdata` | 32 | Core/Inc/Eeprom.h:152 | 0 | 12 | 最終電源ON年 | 保存済 read_param_1wirebk+read_param_1wireinfo+read_param_common+read_param_info+read_param_interface HIST |
| 165 | `eep_wdata` | 32 | Core/Inc/Eeprom.h:153 | 10 | 15 | 最終電源ON年 | 保存済 write_param_1wirebk+write_param_1wireinfo+write_param_common+write_param_info+write_param_interface HIST |
| 166 | `MLSS_Coef_TempC` | 28 | Core/Src/IM_110.c:302 | 1 | 6 | 温度補正 (Ref による受光補正) | 保存済 STORE |
| 167 | `SS_Coef_TempC` | 28 | Core/Src/IM_110.c:303 | 1 | 7 | TR は SS を参照 (実体持たず) | 保存済 STORE |
| 168 | `TR_Coef_ModeCF` | 28 | Core/Src/IM_110.c:298 | 2 | 7 | TR 累乗 a,b (校正後 = ストア Page14) | 保存済 STORE |
| 169 | `TR_Coef_ModeCF_Ship` | 28 | Core/Src/IM_110.c:299 | 1 | 3 | TR 累乗 a,b (出荷時ベース = ストア Page13, §12 表) | 保存済 STORE |
| 170 | `WAG_FtempH` | 28 | Core/Inc/mainSub.h:141 | 4 | 3 | ワグニット温度補正係数 | 保存済 read_param_1wireinfo+write_param_1wireinfo |
| 171 | `WAG_FairH` | 16 | Core/Inc/mainSub.h:136 | 4 | 3 | 空気校正値補正係数 | 保存済 read_param_1wireinfo+write_param_1wireinfo |
| 172 | `WAG_Fflow` | 16 | Core/Inc/mainSub.h:117 | 4 | 3 | 流速補正係数 | 保存済 read_param_1wireinfo+write_param_1wireinfo |
| 173 | `MLSS_Coef_ADZR` | 8 | Core/Src/IM_110.c:300 | 1 | 7 | [0]=受光ADZR [1]=RefADZR (per-mode 単一ゼロ) | 保存済 STORE |
| 174 | `SS_Coef_ADZR` | 8 | Core/Src/IM_110.c:301 | 1 | 10 | ref − refZR (Ref は SS と共有) | 保存済 STORE |
| 175 | `WAG_Header` | 6 | Core/Inc/mainSub.h:115 | 0 | 10 | センサー形式 | 保存済 write_param_1wireinfo |
| 176 | `WAG_Header_Back` | 6 | Core/Inc/mainSub.h:80 | 2 | 5 | 前回センサー形式 | 保存済 read_param_1wireinfo |
| 177 | `Depth` | 4 | Core/Src/IM_110.c:35 | 8 | 16 | 18-19 水深 | 保存済 HIST |
| 178 | `Depth_k` | 4 | Core/Src/IM_110.c:1354 | 4 | 3 | 6m 相当加圧時に Depth=6.0m | 保存済 STORE |
| 179 | `Interface_Threshold` | 4 | Core/Src/IM_110.c:36 | 4 | 11 | 界面検知しきい値 | 保存済 read_param_interface+write_param_interface |
| 180 | `MLSS` | 4 | Core/Src/IM_110.c:31 | 10 | 41 | 09 清水 (基準値0固定) | 保存済 STORE |
| 181 | `MLSS_Cal_SetVal_1` | 4 | Core/Src/IM_110.c:104 | 7 | 11 | スパン1濃度 | 保存済 read_param_common+write_param_common STORE |
| 182 | `MLSS_Cal_SetVal_2` | 4 | Core/Src/IM_110.c:105 | 9 | 10 | スパン2濃度 | 保存済 read_param_common+write_param_common STORE |
| 183 | `MLSS_SP_A` | 4 | Core/Src/IM_110.c:98 | 4 | 5 | MLSS スパン校正係数A (y=Ax^2+Bx+C) | 保存済 STORE |
| 184 | `MLSS_SP_B` | 4 | Core/Src/IM_110.c:99 | 4 | 6 | MLSS スパン校正係数B | 保存済 STORE |
| 185 | `MLSS_SP_C` | 4 | Core/Src/IM_110.c:100 | 4 | 5 | MLSS スパン校正係数C | 保存済 STORE |
| 186 | `MLSS_ZR` | 4 | Core/Src/IM_110.c:91 | 7 | 7 | MLSSゼロ校正係数 (mV) = 現場枠 zr_field | 保存済 STORE |
| 187 | `MLSS_ZR_Ship` | 4 | Core/Src/IM_110.c:94 | 3 | 2 | W:Adj_CaptureZR_Ship,store_unpack_to_globals,update_main_ave R:Adj_ResetZR,store_pack_from_globals | 保存済 STORE |
| 188 | `SS` | 4 | Core/Src/IM_110.c:32 | 9 | 36 | 12 | 保存済 STORE |
| 189 | `SS_Cal_SetVal_1` | 4 | Core/Src/IM_110.c:200 | 7 | 9 | SS 校正設定値1 (2点校正時のスパン濃度) | 保存済 read_param_common+write_param_common STORE |
| 190 | `SS_Cal_SetVal_2` | 4 | Core/Src/IM_110.c:201 | 8 | 8 | SS 校正設定値2 (3点校正時の中間濃度) | 保存済 read_param_common+write_param_common STORE |
| 191 | `SS_SP_A` | 4 | Core/Src/IM_110.c:194 | 4 | 5 | SS スパン校正係数A (y=Ax^2+Bx+C) | 保存済 STORE |
| 192 | `SS_SP_B` | 4 | Core/Src/IM_110.c:195 | 4 | 5 | SS スパン校正係数B | 保存済 STORE |
| 193 | `SS_SP_C` | 4 | Core/Src/IM_110.c:196 | 4 | 5 | SS スパン校正係数C | 保存済 STORE |
| 194 | `SS_ZR` | 4 | Core/Src/IM_110.c:189 | 7 | 4 | SSゼロ校正係数 (mV) = 現場枠 zr_field | 保存済 STORE |
| 195 | `SS_ZR_Ship` | 4 | Core/Src/IM_110.c:190 | 3 | 2 | 出荷時枠 zr_ship (§191/§193)。MLSS_ZR_Ship と同じ役割 | 保存済 STORE |
| 196 | `Stbl_STD` | 4 | Core/Inc/mainSub.h:67 | 3 | 1 | 自動安定判断条件の切替値（高／低濃度） | 保存済 read_param_1wireinfo |
| 197 | `TR_Cal_SetVal_1` | 4 | Core/Src/IM_110.c:253 | 7 | 11 | TR 校正設定値1 (2点校正時のスパン濃度) | 保存済 read_param_common+write_param_common STORE |
| 198 | `TR_Cal_SetVal_2` | 4 | Core/Src/IM_110.c:254 | 8 | 8 | TR 校正設定値2 (3点校正時の中間濃度) | 保存済 read_param_common+write_param_common STORE |
| 199 | `TR_SP_A` | 4 | Core/Src/IM_110.c:247 | 4 | 5 | TR スパン校正係数A (y=Ax^2+Bx+C) | 保存済 STORE |
| 200 | `TR_SP_B` | 4 | Core/Src/IM_110.c:248 | 4 | 5 | TR スパン校正係数B | 保存済 STORE |
| 201 | `TR_SP_C` | 4 | Core/Src/IM_110.c:249 | 4 | 5 | TR スパン校正係数C | 保存済 STORE |
| 202 | `TR_ZR` | 4 | Core/Src/IM_110.c:243 | 6 | 4 | TRゼロ校正係数 (mV) | 保存済 STORE |
| 203 | `WAG_Base` | 4 | Core/Inc/mainSub.h:118 | 5 | 3 | 基準出力（2バイト整数） | 保存済 read_param_1wireinfo+write_param_1wireinfo |
| 204 | `WAG_DOsp` | 4 | Core/Inc/mainSub.h:147 | 5 | 5 | ワグニットスパン校正係数 | 保存済 read_param_1wireinfo+write_param_1wireinfo |
| 205 | `WAG_DOzr` | 4 | Core/Inc/mainSub.h:148 | 5 | 4 | ワグニットゼロ校正係数 | 保存済 read_param_1wireinfo+write_param_1wireinfo |
| 206 | `WAG_SNo` | 4 | Core/Inc/mainSub.h:146 | 3 | 8 | ワグニット製造番号 | 保存済 write_param_1wireinfo |
| 207 | `WAG_SNo_Back` | 4 | Core/Inc/mainSub.h:81 | 4 | 4 | ワグニット製造番号 | 保存済 read_param_1wireinfo |
| 208 | `WAG_StableB` | 4 | Core/Inc/mainSub.h:128 | 4 | 4 | 低濃度側安定判断幅（2バイト整数） | 保存済 read_param_1wireinfo+write_param_1wireinfo |
| 209 | `WAG_StableBH` | 4 | Core/Inc/mainSub.h:132 | 4 | 4 | 高濃度側安定判断幅（2バイト整数） | 保存済 read_param_1wireinfo+write_param_1wireinfo |
| 210 | `WAG_StableC` | 4 | Core/Inc/mainSub.h:129 | 4 | 4 | 低濃度安定解除幅（2バイト整数） | 保存済 read_param_1wireinfo+write_param_1wireinfo |
| 211 | `WAG_StableCH` | 4 | Core/Inc/mainSub.h:133 | 4 | 4 | 高濃度安定解除幅（2バイト整数） | 保存済 read_param_1wireinfo+write_param_1wireinfo |
| 212 | `WAG_StbChange` | 4 | Core/Inc/mainSub.h:138 | 4 | 6 | 安定判断条件切替値 | 保存済 read_param_1wireinfo+write_param_1wireinfo |
| 213 | `ErrDate` | 3 | Core/Inc/mainSub.h:95 | 5 | 2 | エラー発生日(年) | 保存済 read_param_info+write_param_info |
| 214 | `LastDate` | 3 | Core/Inc/mainSub.h:69 | 4 | 8 | 最終電源ON年 | 保存済 read_param_info+write_param_info |
| 215 | `WAG_Date` | 3 | Core/Inc/mainSub.h:116 | 4 | 5 | ワグニット製造年 | 保存済 read_param_1wireinfo+write_param_1wireinfo |
| 216 | `WAG_Date_Meas1` | 3 | Core/Inc/mainSub.h:158 | 6 | 4 | 測定記録1履歴最新記憶年 | 保存済 read_param_1wirebk+write_param_1wirebk |
| 217 | `WAG_Date_SEnd` | 3 | Core/Inc/mainSub.h:163 | 6 | 4 | 有効期限切れ発生年 | 保存済 read_param_1wirebk+write_param_1wirebk |
| 218 | `WAG_Limit` | 3 | Core/Inc/mainSub.h:144 | 4 | 5 | ワグニット保証期限 | 保存済 read_param_1wireinfo+write_param_1wireinfo |
| 219 | `EEP_Read_Err` | 2 | Core/Inc/mainSub.h:104 | 4 | 4 | EEPROM読込失敗回数 | 保存済 read_param_info+write_param_info |
| 220 | `EEP_Read_Retry` | 2 | Core/Inc/mainSub.h:103 | 4 | 4 | EEPROM読込リトライ回数 | 保存済 read_param_info+write_param_info |
| 221 | `EEP_Write_Err` | 2 | Core/Inc/mainSub.h:106 | 4 | 4 | EEPROM書込失敗回数 | 保存済 read_param_info+write_param_info |
| 222 | `EEP_Write_Retry` | 2 | Core/Inc/mainSub.h:105 | 4 | 4 | EEPROM書込リトライ回数 | 保存済 read_param_info+write_param_info |
| 223 | `RTC_Read_Err` | 2 | Core/Inc/mainSub.h:86 | 4 | 4 | RTC読込失敗回数 | 保存済 read_param_info+write_param_info |
| 224 | `RTC_Read_Retry` | 2 | Core/Inc/mainSub.h:85 | 4 | 4 | RTC読込リトライ回数 | 保存済 read_param_info+write_param_info |
| 225 | `RTC_TSet` | 2 | Core/Inc/mainSub.h:82 | 4 | 4 | RTC設定回数 | 保存済 read_param_info+write_param_info |
| 226 | `RTC_Write_Err` | 2 | Core/Inc/mainSub.h:88 | 4 | 4 | RTC書込失敗回数 | 保存済 read_param_info+write_param_info |
| 227 | `RTC_Write_Retry` | 2 | Core/Inc/mainSub.h:87 | 4 | 4 | RTC書込リトライ回数 | 保存済 read_param_info+write_param_info |
| 228 | `WAG_OmakeDay2` | 2 | Core/Inc/mainSub.h:149 | 4 | 3 | センサー使用不可日数 | 保存済 read_param_1wireinfo+write_param_1wireinfo |
| 229 | `WAG_OmakeDay2_2` | 2 | Core/Inc/mainSub.h:151 | 4 | 4 | センサー使用不可日数2 | 保存済 read_param_1wireinfo+write_param_1wireinfo |
| 230 | `WAG_Total_POn` | 2 | Core/Inc/mainSub.h:161 | 6 | 4 | 電源ON回数累計（2バイト整数） | 保存済 read_param_1wirebk+write_param_1wirebk |
| 231 | `WAG_Total_Span` | 2 | Core/Inc/mainSub.h:160 | 6 | 4 | スパン校正回数累計（2バイト整数） | 保存済 read_param_1wirebk+write_param_1wirebk |
| 232 | `WAG_Total_Zero` | 2 | Core/Inc/mainSub.h:159 | 6 | 4 | ゼロ校正回数累計（2バイト整数） | 保存済 read_param_1wirebk+write_param_1wirebk |
| 233 | `WIRE_Init_Err` | 2 | Core/Inc/mainSub.h:165 | 6 | 4 | 1Wire初期化エラー回数（2バイト整数） | 保存済 read_param_1wirebk+write_param_1wirebk |
| 234 | `WIRE_Init_Retry` | 2 | Core/Inc/mainSub.h:164 | 6 | 4 | 1Wire初期化リトライ回数（2バイト整数） | 保存済 read_param_1wirebk+write_param_1wirebk |
| 235 | `WIRE_Read_Err` | 2 | Core/Inc/mainSub.h:167 | 6 | 4 | 1Wire読込エラー回数（2バイト整数） | 保存済 read_param_1wirebk+write_param_1wirebk |
| 236 | `WIRE_Read_Retry` | 2 | Core/Inc/mainSub.h:166 | 6 | 4 | 1Wire読込リトライ回数（2バイト整数） | 保存済 read_param_1wirebk+write_param_1wirebk |
| 237 | `WIRE_Write_Err` | 2 | Core/Inc/mainSub.h:169 | 6 | 4 | 1Wire書込エラー回数（2バイト整数） | 保存済 read_param_1wirebk+write_param_1wirebk |
| 238 | `WIRE_Write_Retry` | 2 | Core/Inc/mainSub.h:168 | 6 | 4 | 1Wire書込リトライ回数（2バイト整数） | 保存済 read_param_1wirebk+write_param_1wirebk |
| 239 | `year` | 2 | Core/Inc/mainSub.h:204 | 6 | 29 | RTC 年 | 保存済 HIST |
| 240 | `EEP_Info_flag` | 1 | Core/Inc/mainSub.h:79 | 5 | 4 | EEPROM書込フラグ 1Wire寿命を強制的にEEPROM側にする | 保存済 read_param_info+write_param_info |
| 241 | `ErrNo` | 1 | Core/Inc/mainSub.h:96 | 4 | 4 | エラー番号 | 保存済 read_param_info+write_param_info |
| 242 | `MLSS_MODE` | 1 | Core/Src/IM_110.c:90 | 8 | 21 | 相関式 No.1 (内部値 0) | 保存済 read_param_common+write_param_common |
| 243 | `Meas_Mode` | 1 | Core/Src/IM_110.c:30 | 7 | 41 | 測定モード = MLSS | 保存済 read_param_common+write_param_common |
| 244 | `SS_MODE` | 1 | Core/Src/IM_110.c:188 | 7 | 21 | SSモード | 保存済 read_param_common+write_param_common |
| 245 | `TR_MODE` | 1 | Core/Src/IM_110.c:242 | 7 | 21 | TRモード | 保存済 read_param_common+write_param_common |
| 246 | `WAG_AveSize` | 1 | Core/Inc/mainSub.h:137 | 4 | 3 | 移動平均件数 | 保存済 read_param_1wireinfo+write_param_1wireinfo |
| 247 | `WAG_Flag_SEnd` | 1 | Core/Inc/mainSub.h:162 | 6 | 6 | 有効期限切れ有無 | 保存済 read_param_1wirebk+write_param_1wirebk |
| 248 | `WAG_OmakeDay` | 1 | Core/Inc/mainSub.h:145 | 4 | 3 | 期限切れ後使用可能日数 | 保存済 read_param_1wireinfo+write_param_1wireinfo |
| 249 | `WAG_OmakeDay_2` | 1 | Core/Inc/mainSub.h:150 | 4 | 4 | 期限切れ後使用可能日数2 | 保存済 read_param_1wireinfo+write_param_1wireinfo |
| 250 | `WAG_Over` | 1 | Core/Inc/mainSub.h:121 | 4 | 4 | センサー出力過多判定値% | 保存済 read_param_1wireinfo+write_param_1wireinfo |
| 251 | `WAG_POS_Cal` | 1 | Core/Inc/mainSub.h:154 | 6 | 4 | 校正履歴最新記憶場所 | 保存済 read_param_1wirebk+write_param_1wirebk |
| 252 | `WAG_POS_Err` | 1 | Core/Inc/mainSub.h:157 | 6 | 4 | エラー履歴最新記憶場所 | 保存済 read_param_1wirebk+write_param_1wirebk |
| 253 | `WAG_POS_Meas1` | 1 | Core/Inc/mainSub.h:155 | 6 | 4 | 測定履歴1最新記憶場所 | 保存済 read_param_1wirebk+write_param_1wirebk |
| 254 | `WAG_POS_Meas2` | 1 | Core/Inc/mainSub.h:156 | 6 | 4 | 測定履歴2最新記憶場所 | 保存済 read_param_1wirebk+write_param_1wirebk |
| 255 | `WAG_Size` | 1 | Core/Inc/mainSub.h:122 | 4 | 5 | 1Wireサイズ 4(kB)か20(kB) | 保存済 read_param_1wireinfo+write_param_1wireinfo |
| 256 | `WAG_Span` | 1 | Core/Inc/mainSub.h:120 | 4 | 4 | 膜液交換判定値% | 保存済 read_param_1wireinfo+write_param_1wireinfo |
| 257 | `WAG_StableA` | 1 | Core/Inc/mainSub.h:127 | 4 | 5 | 低濃度側安定判断時間 | 保存済 read_param_1wireinfo+write_param_1wireinfo |
| 258 | `WAG_StableAH` | 1 | Core/Inc/mainSub.h:131 | 4 | 3 | 高濃度側安定判断時間 | 保存済 read_param_1wireinfo+write_param_1wireinfo |
| 259 | `WAG_StableD` | 1 | Core/Inc/mainSub.h:130 | 4 | 3 | 低濃度側安定判断中表示間隔 | 保存済 read_param_1wireinfo+write_param_1wireinfo |
| 260 | `WAG_StableDH` | 1 | Core/Inc/mainSub.h:134 | 4 | 3 | 高濃度側安定判断中表示間隔 | 保存済 read_param_1wireinfo+write_param_1wireinfo |
| 261 | `WAG_StableT` | 1 | Core/Inc/mainSub.h:135 | 4 | 3 | 低濃度安定判断禁止時間 | 保存済 read_param_1wireinfo+write_param_1wireinfo |
| 262 | `WAG_Zero` | 1 | Core/Inc/mainSub.h:119 | 4 | 4 | ゼロ校正判定値% | 保存済 read_param_1wireinfo+write_param_1wireinfo |
| 263 | `Wire_Backup_flag` | 1 | Core/Inc/mainSub.h:76 | 5 | 5 | 1Wireバックアップ有無 1で有り | 保存済 read_param_info+write_param_info |
| 264 | `Wire_ReadError_Count` | 1 | Core/Inc/mainSub.h:77 | 4 | 4 | 1Wire読込連続失敗回数 | 保存済 read_param_info+write_param_info |
| 265 | `Wire_WriteError_Count` | 1 | Core/Inc/mainSub.h:78 | 4 | 4 | 1Wire書込連続失敗回数 | 保存済 read_param_info+write_param_info |
| 266 | `day` | 1 | Core/Inc/mainSub.h:201 | 6 | 32 | 現在日 | 保存済 HIST |
| 267 | `hour` | 1 | Core/Inc/mainSub.h:200 | 7 | 28 | 0時（日までは波及しない） | 保存済 HIST |
| 268 | `min` | 1 | Core/Inc/mainSub.h:199 | 7 | 28 | 0分 | 保存済 HIST |
| 269 | `month` | 1 | Core/Inc/mainSub.h:202 | 6 | 33 | 現在月 | 保存済 HIST |
| 270 | `SYS_ErrorNo` | 4 | Core/Inc/mainSub.h:71 | 4 | 6 | BLE初期化異常は本体故障No.99 | エラーカウンタ page4 に集約済 or 揮発 |
| 271 | `WIRE_ErrorNo` | 4 | Core/Inc/mainSub.h:72 | 3 | 1 | 1wire, EEP共に記憶はできないので、エラーを表示する | エラーカウンタ page4 に集約済 or 揮発 |
| 272 | `WInit_Err` | 4 | Core/Inc/WireSub.h:20 | 10 | 2 | 初期化失敗回数カウント（電源ON～OFF間に発生した回数追加） | エラーカウンタ page4 に集約済 or 揮発 |
| 273 | `WInit_Retry` | 4 | Core/Inc/WireSub.h:19 | 10 | 2 | 初期化リトライ回数カウント（電源ON～OFF間に発生した回数追加） | エラーカウンタ page4 に集約済 or 揮発 |
| 274 | `WRead_Err` | 4 | Core/Inc/WireSub.h:22 | 8 | 2 | 読み込み失敗回数カウント（電源ON～OFF間に発生した回数追加） | エラーカウンタ page4 に集約済 or 揮発 |
| 275 | `WRead_Retry` | 4 | Core/Inc/WireSub.h:21 | 8 | 2 | 読み込みリトライ回数カウント（電源ON～OFF間に発生した回数追加） | エラーカウンタ page4 に集約済 or 揮発 |
| 276 | `WWrite_Err` | 4 | Core/Inc/WireSub.h:24 | 6 | 2 | 書き込み失敗回数カウント（電源ON～OFF間に発生した回数追加） | エラーカウンタ page4 に集約済 or 揮発 |
| 277 | `WWrite_Retry` | 4 | Core/Inc/WireSub.h:23 | 6 | 2 | 書き込みリトライ回数カウント（電源ON～OFF間に発生した回数追加） | エラーカウンタ page4 に集約済 or 揮発 |
| 278 | `WAG_HstErr_Date` | 3 | Core/Inc/mainSub.h:188 | 1 | 2 | 発生日(年) | エラーカウンタ page4 に集約済 or 揮発 |
| 279 | `EComm_Err` | 2 | Core/Inc/mainSub.h:108 | 2 | 2 | 初期化失敗回数カウント（電源ON～OFF間に発生した回数追加） | エラーカウンタ page4 に集約済 or 揮発 |
| 280 | `EComm_Retry` | 2 | Core/Inc/mainSub.h:107 | 2 | 2 | 初期化リトライ回数カウント（電源ON～OFF間に発生した回数追加） | エラーカウンタ page4 に集約済 or 揮発 |
| 281 | `EEP_Comm_Err` | 2 | Core/Inc/mainSub.h:102 | 1 | 2 | EEPROM通信NG エラー回数 (総計) | エラーカウンタ page4 に集約済 or 揮発 |
| 282 | `EEP_Comm_Retry` | 2 | Core/Inc/mainSub.h:101 | 1 | 2 | EEPROM通信NG リトライ回数 (総計) | エラーカウンタ page4 に集約済 or 揮発 |
| 283 | `ERead_Err` | 2 | Core/Inc/mainSub.h:110 | 6 | 2 | 読み込み失敗回数カウント（電源ON～OFF間に発生した回数追加） | エラーカウンタ page4 に集約済 or 揮発 |
| 284 | `ERead_Retry` | 2 | Core/Inc/mainSub.h:109 | 6 | 2 | 読み込みリトライ回数カウント（電源ON～OFF間に発生した回数追加） | エラーカウンタ page4 に集約済 or 揮発 |
| 285 | `EWrite_Err` | 2 | Core/Inc/mainSub.h:112 | 7 | 2 | 書き込み失敗回数カウント（電源ON～OFF間に発生した回数追加） | エラーカウンタ page4 に集約済 or 揮発 |
| 286 | `EWrite_Retry` | 2 | Core/Inc/mainSub.h:111 | 7 | 2 | 書き込みリトライ回数カウント（電源ON～OFF間に発生した回数追加） | エラーカウンタ page4 に集約済 or 揮発 |
| 287 | `RComm_Err` | 2 | Core/Inc/mainSub.h:90 | 2 | 2 | 初期化失敗回数カウント（電源ON～OFF間に発生した回数追加） | エラーカウンタ page4 に集約済 or 揮発 |
| 288 | `RComm_Retry` | 2 | Core/Inc/mainSub.h:89 | 2 | 2 | 初期化リトライ回数カウント（電源ON～OFF間に発生した回数追加） | エラーカウンタ page4 に集約済 or 揮発 |
| 289 | `RRead_Err` | 2 | Core/Inc/mainSub.h:92 | 2 | 2 | 読み込み失敗回数カウント（電源ON～OFF間に発生した回数追加） | エラーカウンタ page4 に集約済 or 揮発 |
| 290 | `RRead_Retry` | 2 | Core/Inc/mainSub.h:91 | 2 | 2 | 読み込みリトライ回数カウント（電源ON～OFF間に発生した回数追加） | エラーカウンタ page4 に集約済 or 揮発 |
| 291 | `RTC_Comm_Err` | 2 | Core/Inc/mainSub.h:84 | 1 | 2 | RTC通信NG エラー回数 (総計) | エラーカウンタ page4 に集約済 or 揮発 |
| 292 | `RTC_Comm_Retry` | 2 | Core/Inc/mainSub.h:83 | 1 | 2 | RTC通信NG リトライ回数 (総計) | エラーカウンタ page4 に集約済 or 揮発 |
| 293 | `RWrite_Err` | 2 | Core/Inc/mainSub.h:94 | 2 | 2 | 書き込み失敗回数カウント（電源ON～OFF間に発生した回数追加） | エラーカウンタ page4 に集約済 or 揮発 |
| 294 | `RWrite_Retry` | 2 | Core/Inc/mainSub.h:93 | 2 | 2 | 書き込みリトライ回数カウント（電源ON～OFF間に発生した回数追加） | エラーカウンタ page4 に集約済 or 揮発 |
| 295 | `ERROR_CLOCK_PON_Flag` | 1 | Core/Inc/mainSub.h:208 | 2 | 2 | PONエラー判別 | エラーカウンタ page4 に集約済 or 揮発 |
| 296 | `ERROR_CLOCK_PON_Flag_sub` | 1 | Core/Inc/mainSub.h:211 | 1 | 2 | PONエラー判別 | エラーカウンタ page4 に集約済 or 揮発 |
| 297 | `ERROR_CLOCK_VDET_Flag` | 1 | Core/Inc/mainSub.h:206 | 2 | 2 | VDETエラー判別 | エラーカウンタ page4 に集約済 or 揮発 |
| 298 | `ERROR_CLOCK_VDET_Flag_sub` | 1 | Core/Inc/mainSub.h:209 | 1 | 2 | VDETエラー判別 | エラーカウンタ page4 に集約済 or 揮発 |
| 299 | `ERROR_CLOCK_XST_Flag` | 1 | Core/Inc/mainSub.h:207 | 2 | 2 | XSTエラー判別 | エラーカウンタ page4 に集約済 or 揮発 |
| 300 | `ERROR_CLOCK_XST_Flag_sub` | 1 | Core/Inc/mainSub.h:210 | 1 | 2 | XSTエラー判別 | エラーカウンタ page4 に集約済 or 揮発 |
| 301 | `ERROR_REG` | 1 | Core/Inc/mainSub.h:212 | 2 | 2 | RTCエラーフラグ | エラーカウンタ page4 に集約済 or 揮発 |
| 302 | `WAG_HstErr_Type` | 1 | Core/Inc/mainSub.h:189 | 1 | 2 | エラー番号 | エラーカウンタ page4 に集約済 or 揮発 |
| 303 | `f_IC_err` | 1 | Core/Inc/mainSub.h:68 | 24 | 12 | ICのエラーフラグ | エラーカウンタ page4 に集約済 or 揮発 |
| 304 | `Probe_MD_Timer` | 4 | Core/Inc/Timer.h:92 | 2 | 2 | 起動後2秒待ってからMD送信開始(Probe起動文字列を待つ) | タイマ/カウンタ 揮発で正しい |
| 305 | `Probe_Stream_Timer` | 4 | Core/Inc/Timer.h:93 | 2 | 2 | Probe測定ストリーム鮮度タイマ (0=鮮度切れ→"----") | タイマ/カウンタ 揮発で正しい |
| 306 | `Timer_Stable` | 4 | Core/Inc/AutoStable.h:21 | 3 | 3 | 所要時間リセット | タイマ/カウンタ 揮発で正しい |
| 307 | `adc_dis_timer` | 4 | Core/Inc/Timer.h:49 | 2 | 3 | レンジ切替後A/DC変換禁止タイマー | タイマ/カウンタ 揮発で正しい |
| 308 | `auto_power_off_timer` | 4 | Core/Inc/Timer.h:64 | 7 | 3 | オートパワーオフタイマー | タイマ/カウンタ 揮発で正しい |
| 309 | `batt_timer` | 4 | Core/Inc/Timer.h:55 | 4 | 3 | 電池電圧取得タイマー | タイマ/カウンタ 揮発で正しい |
| 310 | `ble_recv_timer` | 4 | Core/Inc/Timer.h:71 | 4 | 4 | BLE受信待ちタイマー | タイマ/カウンタ 揮発で正しい |
| 311 | `bt_first_off_timer` | 4 | Core/Inc/Timer.h:74 | 2 | 3 | BT電源ON直後タイマー | タイマ/カウンタ 揮発で正しい |
| 312 | `bt_off_timer` | 4 | Core/Inc/Timer.h:73 | 3 | 3 | BT OFFタイマー | タイマ/カウンタ 揮発で正しい |
| 313 | `bt_on_timer` | 4 | Core/Inc/Timer.h:72 | 3 | 3 | BT ONタイマー | タイマ/カウンタ 揮発で正しい |
| 314 | `cal_timer` | 4 | Core/Inc/Timer.h:51 | 3 | 5 | 校正用タイマー（センサー不安定） | タイマ/カウンタ 揮発で正しい |
| 315 | `check_wag_timer` | 4 | Core/Inc/Timer.h:89 | 2 | 2 | センサー挿抜識別タイマー | タイマ/カウンタ 揮発で正しい |
| 316 | `disp_hold_timer` | 4 | Core/Inc/Timer.h:65 | 2 | 2 | ホールド表示タイマー | タイマ/カウンタ 揮発で正しい |
| 317 | `disp_timer` | 4 | Core/Inc/Timer.h:56 | 30 | 27 | LCD表示更新タイマー | タイマ/カウンタ 揮発で正しい |
| 318 | `ecomode_timer` | 4 | Core/Inc/Timer.h:78 | 3 | 3 | 省電力測定中表示タイマー | タイマ/カウンタ 揮発で正しい |
| 319 | `first_LOW_timer` | 4 | Core/Inc/Timer.h:68 | 1 | 2 | 切替直後の低レンジ変換処理タイマー | タイマ/カウンタ 揮発で正しい |
| 320 | `flash_timer` | 4 | Core/Inc/Timer.h:52 | 15 | 14 | 数値、電池残量点滅タイマー | タイマ/カウンタ 揮発で正しい |
| 321 | `hold_update_timer` | 4 | Core/Inc/Timer.h:66 | 3 | 3 | DO値表示アップデートタイマー | タイマ/カウンタ 揮発で正しい |
| 322 | `ident_wag_timer` | 4 | Core/Inc/Timer.h:90 | 3 | 2 | センサー識別タイマー | タイマ/カウンタ 揮発で正しい |
| 323 | `lcd_off_timer` | 4 | Core/Inc/Timer.h:50 | 17 | 17 | 画面表示OFFタイマー | タイマ/カウンタ 揮発で正しい |
| 324 | `lcd_power_off_timer` | 4 | Core/Inc/Timer.h:75 | 3 | 3 | LCD電源OFFタイマー | タイマ/カウンタ 揮発で正しい |
| 325 | `logger_rtc_timer` | 4 | Core/Inc/Timer.h:80 | 3 | 3 | ロガー測定日時更新タイマー | タイマ/カウンタ 揮発で正しい |
| 326 | `logger_sw_timer` | 4 | Core/Inc/Timer.h:81 | 3 | 3 | 省電力測定復帰後SW無効タイマー | タイマ/カウンタ 揮発で正しい |
| 327 | `logger_wake_timer` | 4 | Core/Inc/Timer.h:60 | 3 | 3 | ロガー測定用タイマー | タイマ/カウンタ 揮発で正しい |
| 328 | `power_on_wait_timer` | 4 | Core/Inc/Timer.h:79 | 3 | 3 | 電源ON後測定待ちタイマー | タイマ/カウンタ 揮発で正しい |
| 329 | `power_sw_timer` | 4 | Core/Inc/Timer.h:57 | 5 | 4 | 判断禁止時間を 50 -> 400 (mS)に変更 | タイマ/カウンタ 揮発で正しい |
| 330 | `qr_update_cnt` | 4 | Core/Inc/mainSub.h:302 | 2 | 2 | LCD表示更新用カウント | タイマ/カウンタ 揮発で正しい |
| 331 | `range_hold_timer` | 4 | Core/Inc/Timer.h:69 | 3 | 4 | 電源ON直後強制レンジ固定タイマー | タイマ/カウンタ 揮発で正しい |
| 332 | `setting_fl_timer` | 4 | Core/Inc/Timer.h:53 | 2 | 2 | メニューカーソル点滅タイマー | タイマ/カウンタ 揮発で正しい |
| 333 | `stable_cnt` | 4 | Core/Src/AutoStable.c:28 | 3 | 2 | 安定状態カウント用 | タイマ/カウンタ 揮発で正しい |
| 334 | `stbblank_timer` | 4 | Core/Inc/Timer.h:59 | 2 | 2 | 安定判断禁止タイマー | タイマ/カウンタ 揮発で正しい |
| 335 | `stbdisp_timer` | 4 | Core/Inc/Timer.h:58 | 3 | 3 | 安定時表示更新間隔タイマー | タイマ/カウンタ 揮発で正しい |
| 336 | `timer1sec` | 4 | Core/Inc/Timer.h:48 | 4 | 3 | 1secタイマーカウント | タイマ/カウンタ 揮発で正しい |
| 337 | `timer_EXTCOMIN` | 4 | Core/Inc/Timer.h:62 | 3 | 3 | LCD EXTCOMIN信号タイマー | タイマ/カウンタ 揮発で正しい |
| 338 | `timer_errdisp` | 4 | Core/Inc/Timer.h:63 | 3 | 3 | エラー自動解除タイマー | タイマ/カウンタ 揮発で正しい |
| 339 | `v33_power_off_timer` | 4 | Core/Inc/Timer.h:77 | 3 | 3 | 3.3V電源OFFタイマー | タイマ/カウンタ 揮発で正しい |
| 340 | `v33_power_on_timer` | 4 | Core/Inc/Timer.h:76 | 3 | 3 | 3.3V電源ONタイマー | タイマ/カウンタ 揮発で正しい |
| 341 | `log_start_time` | 2 | Core/Inc/mainSub.h:345 | 3 | 7 | ロガー開始時 | タイマ/カウンタ 揮発で正しい |
| 342 | `AutoCal_time_sel` | 1 | Core/Inc/mainSub.h:293 | 4 | 2 | デフォルトで6AM | タイマ/カウンタ 揮発で正しい |
| 343 | `DISP_l_sw_cnt` | 1 | Core/Inc/mainSub.h:322 | 1 | 2 | DISP SWの長押し判定カウント | タイマ/カウンタ 揮発で正しい |
| 344 | `MEM_l_sw_cnt` | 1 | Core/Inc/mainSub.h:321 | 2 | 2 | MEM SWの長押し判定カウント | タイマ/カウンタ 揮発で正しい |
| 345 | `POW_l_sw_cnt` | 1 | Core/Inc/mainSub.h:323 | 1 | 2 | POW SWの長押し判定カウント | タイマ/カウンタ 揮発で正しい |
| 346 | `log_timer_flag` | 1 | Core/Inc/mainSub.h:343 | 3 | 3 | ロガー測定タイマー有無 | タイマ/カウンタ 揮発で正しい |
| 347 | `sec` | 1 | Core/Inc/mainSub.h:198 | 2 | 6 | RTC 秒 | タイマ/カウンタ 揮発で正しい |
| 348 | `disp_buf` | 12482 | Core/Inc/Lcd.h:36 | 4 | 3 | (cmd + adr + 50バイト) x 240ライン + dummyx2=12482バイト | UI状態 揮発で正しい |
| 349 | `guide_disp_flag` | 5 | Core/Inc/mainSub.h:63 | 6 | 5 | 1日1回表示済み | UI状態 揮発で正しい |
| 350 | `DISP_CYCLE` | 4 | Core/Inc/mainSub.h:357 | 1 | 31 | 測定中LCD表示間隔 Nx50mS = 2s | UI状態 揮発で正しい |
| 351 | `cal_hsel_cursor` | 2 | Core/Src/Normal.c:72 | 2 | 1 | 選択中の履歴番号 (0..count-1) | UI状態 揮発で正しい |
| 352 | `App_DL_disp_flag` | 1 | Core/Inc/mainSub.h:292 | 3 | 3 | アプリDL先表示フラグ | UI状態 揮発で正しい |
| 353 | `DISP_sw_mem` | 1 | Core/Inc/mainSub.h:325 | 13 | 10 | DISP短押し判別のための記憶 | UI状態 揮発で正しい |
| 354 | `DISP_sw_step` | 1 | Core/Inc/mainSub.h:319 | 33 | 3 | DISP SWの判定用 | UI状態 揮発で正しい |
| 355 | `MEM_sw_mem` | 1 | Core/Inc/mainSub.h:324 | 6 | 7 | MEM短押し判別のための記憶 | UI状態 揮発で正しい |
| 356 | `MEM_sw_step` | 1 | Core/Inc/mainSub.h:318 | 32 | 3 | MEM SWの判定用 | UI状態 揮発で正しい |
| 357 | `POW_sw_step` | 1 | Core/Inc/mainSub.h:320 | 5 | 3 | POW SWの判定用 | UI状態 揮発で正しい |
| 358 | `WAG_flash_flag` | 1 | Core/Inc/mainSub.h:407 | 6 | 2 | センサー名称表示フラグ（0:表示無し, 表示有り） | UI状態 揮発で正しい |
| 359 | `adj_buf_mode` | 1 | Core/Src/IM_110.c:1646 | 3 | 4 | Mode_CF 捕捉のモード (混在検出用) | UI状態 揮発で正しい |
| 360 | `bar_flag` | 1 | Core/Inc/Display.h:37 | 3 | 30 | 電池アイコン表示用 | UI状態 揮発で正しい |
| 361 | `batt_flash_flag` | 1 | Core/Src/Normal.c:36 | 1 | 2 | W:batt_check R:,flash_calc | UI状態 揮発で正しい |
| 362 | `ble_bar_flag` | 1 | Core/Inc/mainSub.h:355 | 1 | 2 | BLE転送用バッテリー残量 | UI状態 揮発で正しい |
| 363 | `cur_sel_item` | 1 | Core/Src/Setting.c:24 | 2 | 1 | W:,set_menu R:set_menu | UI状態 揮発で正しい |
| 364 | `dispACAL_flag` | 1 | Core/Inc/mainSub.h:289 | 2 | 2 | 自動校正エラー表示フラグ | UI状態 揮発で正しい |
| 365 | `disp_colon_flag` | 1 | Core/Inc/mainSub.h:327 | 2 | 2 | 時計：表示フラグ | UI状態 揮発で正しい |
| 366 | `disp_lr_yajirushi` | 1 | Core/Inc/Display.h:38 | 2 | 2 | 矢印アイコン（左右） | UI状態 揮発で正しい |
| 367 | `log_disp_flag` | 1 | Core/Inc/mainSub.h:307 | 1 | 1 | ロガー測定時に数値表示するためのフラグ | UI状態 揮発で正しい |
| 368 | `Probe_RecvData` | 128 | Core/Src/IM_110.c:306 | 2 | 8 | Probe受信バッファ | 通信バッファ/状態 揮発で正しい |
| 369 | `RecvData` | 128 | Core/Inc/LinkSerial.h:17 | 3 | 16 | 受信バッファ | 通信バッファ/状態 揮発で正しい |
| 370 | `UART1_BPS` | 4 | Core/Inc/mainSub.h:340 | 4 | 2 | UART通信速度 | 通信バッファ/状態 揮発で正しい |
| 371 | `stable_now` | 4 | Core/Src/AutoStable.c:30 | 3 | 1 | 安定状態カウント用 | 通信バッファ/状態 揮発で正しい |
| 372 | `stable_old` | 4 | Core/Src/AutoStable.c:29 | 3 | 1 | 安定状態カウント用 | 通信バッファ/状態 揮発で正しい |
| 373 | `Probe_RecvDataP` | 1 | Core/Src/IM_110.c:307 | 2 | 3 | W:Probe_RxCallback,Probe_clear_RecvData R:(宣言のみ),Probe_RxCallback,update_main_ave | 通信バッファ/状態 揮発で正しい |
| 374 | `Probe_RxData` | 1 | Core/Src/IM_110.c:308 | 3 | 3 | 先にローカルへ退避 | 通信バッファ/状態 揮発で正しい |
| 375 | `Probe_cmd_pending` | 1 | Core/Src/IM_110.c:310 | 2 | 1 | P:コマンド応答待ちフラグ | 通信バッファ/状態 揮発で正しい |
| 376 | `Probe_uart_end` | 1 | Core/Src/IM_110.c:309 | 4 | 4 | Probe受信完了フラグ | 通信バッファ/状態 揮発で正しい |
| 377 | `RecvDataP` | 1 | Core/Inc/LinkSerial.h:18 | 3 | 3 | W:HAL_UART_RxCpltCallback,clear_RecvData,Check_BLE_Status R:(宣言のみ),,HAL_UART_RxCpltCallback | 通信バッファ/状態 揮発で正しい |
| 378 | `RxData` | 1 | Core/Inc/LinkSerial.h:19 | 4 | 3 | W:HAL_UART_ErrorCallback,HAL_UART_RxCpltCallback,uart_init,mai R:(宣言のみ),,HAL_UART_RxCpltCallback | 通信バッファ/状態 揮発で正しい |
| 379 | `bt_off_cmd` | 1 | Core/Inc/mainSub.h:367 | 4 | 1 | BT電源OFF処理フラグ | 通信バッファ/状態 揮発で正しい |
| 380 | `bt_on_cmd` | 1 | Core/Inc/mainSub.h:366 | 2 | 1 | BT電源ON処理フラグ | 通信バッファ/状態 揮発で正しい |
| 381 | `bt_on_flag` | 1 | Core/Inc/mainSub.h:359 | 5 | 3 | BT電源ONフラグ | 通信バッファ/状態 揮発で正しい |
| 382 | `lcd_off_cmd` | 1 | Core/Inc/mainSub.h:365 | 2 | 1 | LCD電源OFF処理フラグ | 通信バッファ/状態 揮発で正しい |
| 383 | `lcd_on_cmd` | 1 | Core/Inc/mainSub.h:364 | 4 | 2 | LCD OFFなら表示させる | 通信バッファ/状態 揮発で正しい |
| 384 | `uart_end` | 1 | Core/Inc/LinkSerial.h:20 | 3 | 2 | UART文字列受信フラグ | 通信バッファ/状態 揮発で正しい |
| 385 | `v33_off_cmd` | 1 | Core/Inc/mainSub.h:363 | 2 | 1 | 3.3V電源OFF処理フラグ | 通信バッファ/状態 揮発で正しい |
| 386 | `v33_on_cmd` | 1 | Core/Inc/mainSub.h:362 | 2 | 1 | 3.3V電源ON処理フラグ | 通信バッファ/状態 揮発で正しい |
| 387 | `num.0` | 5 | Core/Src/Display.c:479 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 388 | `num.21` | 5 | Core/Src/Normal.c:623 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 389 | `num.25` | 5 | Core/Src/Normal.c:412 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 390 | `num.29` | 5 | Core/Src/Normal.c:231 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 391 | `num.4` | 5 | Core/Src/Display.c:282 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 392 | `Cal_SetVal_1_tmp.30` | 4 | Core/Src/Normal.c:232 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 393 | `Cal_SetVal_2_tmp.26` | 4 | Core/Src/Normal.c:413 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 394 | `Interface_Threshold_tmp.22` | 4 | Core/Src/Normal.c:624 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 395 | `calexec_time.16` | 4 | Core/Src/Normal.c:1128 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 396 | `EEP_Tadrs.5` | 2 | Core/Src/Adjust.c:565 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 397 | `EEP_Tadrs.7` | 2 | Core/Src/Adjust.c:197 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 398 | `index.13` | 2 | Core/Src/Normal.c:2413 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 399 | `index.32` | 2 | Core/Src/Normal.c:2627 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 400 | `last_idx.3` | 2 | Core/Src/Adjust.c:849 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 401 | `num.12` | 2 | Core/Src/Normal.c:1899 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 402 | `tim1_counter.0` | 2 | Core/Src/Timer.c:85 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 403 | `cal_menu_sel.19` | 1 | Core/Src/Normal.c:837 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 404 | `corr_num_sel.11` | 1 | Core/Src/Normal.c:1900 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 405 | `cur_sel_item2.0` | 1 | Core/Src/Adjust.c:1436 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 406 | `cur_sel_item2.1` | 1 | Core/Src/Adjust.c:1224 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 407 | `cur_sel_item2.2` | 1 | Core/Src/Adjust.c:1086 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 408 | `disp_sw_mem.1` | 1 | Core/Src/Setting.c:235 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 409 | `disp_sw_mem.10` | 1 | Core/Src/Normal.c:1901 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 410 | `disp_sw_mem.18` | 1 | Core/Src/Normal.c:838 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 411 | `disp_sw_mem.20` | 1 | Core/Src/Normal.c:625 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 412 | `disp_sw_mem.24` | 1 | Core/Src/Normal.c:414 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 413 | `disp_sw_mem.28` | 1 | Core/Src/Normal.c:233 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 414 | `empty_bat.2` | 1 | Core/Src/Normal.c:3833 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 415 | `first_check.3` | 1 | Core/Src/Normal.c:3799 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 416 | `fl_flag.17` | 1 | Core/Src/Normal.c:1129 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 417 | `initialized.1` | 1 | Core/Src/IM_110.c:1115 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 418 | `meas_menu_sel.9` | 1 | Core/Src/Normal.c:2022 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 419 | `prev_above.0` | 1 | Core/Src/IM_110.c:1114 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 420 | `result.4` | 1 | Core/Src/Adjust.c:566 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 421 | `result.6` | 1 | Core/Src/Adjust.c:198 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 422 | `span_num_sel.23` | 1 | Core/Src/Normal.c:622 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 423 | `span_num_sel.27` | 1 | Core/Src/Normal.c:411 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 424 | `span_num_sel.31` | 1 | Core/Src/Normal.c:230 | 0 | 0 | W:- R:- | 関数内static 揮発で正しい |
| 425 | `__global_locale` | 364 |  | 0 | 0 | W:- R:- | HAL/libc 対象外 |
| 426 | `__sf` | 312 |  | 0 | 0 | W:- R:- | HAL/libc 対象外 |
| 427 | `hlpuart1` | 136 | Core/Src/main.c:69 | 7 | 6 | W:Probe_RxCallback,Probe_SendSerialData,Probe_uart_init,MX_LPU R:,,MX_LPUART1_UART_Init,ProbeFupBridge_Run,ProbeRelay_Ru | HAL/libc 対象外 |
| 428 | `huart1` | 136 | Core/Src/main.c:70 | 12 | 14 | W:HAL_UART_RxCpltCallback,SendSerialData,uart_init,Change_uart R:,,,,Change_uart_bps,,MX_USART1_UART_Init,ProbeFupBridge | HAL/libc 対象外 |
| 429 | `hadc1` | 104 | Core/Src/main.c:64 | 5 | 5 | W:HAL_ADC_ConvCpltCallback,adc_conv_start,MX_ADC1_Init,v33_pow R:,,,MX_ADC1_Init, | HAL/libc 対象外 |
| 430 | `hspi1` | 100 | Core/Src/main.c:74 | 4 | 4 | spiハンドル | HAL/libc 対象外 |
| 431 | `hspi2` | 100 | Core/Src/main.c:75 | 13 | 7 | spiハンドル | HAL/libc 対象外 |
| 432 | `_impure_data` | 76 |  | 0 | 0 | W:- R:- | HAL/libc 対象外 |
| 433 | `htim1` | 76 | Core/Src/main.c:78 | 7 | 8 | W:timer_set,timer_start,MX_TIM1_Init,SystemClock_16MHz,SystemC R:,,MX_TIM1_Init,,SystemClock_16MHz,SystemClock_4MHz,Syst | HAL/libc 対象外 |
| 434 | `hdma_adc1` | 72 | Core/Src/main.c:65 | 2 | 4 | W:HAL_ADC_MspInit,DMA1_Channel1_IRQHandler R:,,HAL_ADC_MspInit, | HAL/libc 対象外 |
| 435 | `hiwdg` | 16 | Core/Src/main.c:67 | 2 | 3 | W:MX_IWDG_Init,WatchDog_reset R:,MX_IWDG_Init, | HAL/libc 対象外 |
| 436 | `__sglue` | 12 |  | 0 | 0 | W:- R:- | HAL/libc 対象外 |
| 437 | `__tzname_dst` | 12 |  | 0 | 0 | W:- R:- | HAL/libc 対象外 |
| 438 | `__tzname_std` | 12 |  | 0 | 0 | W:- R:- | HAL/libc 対象外 |
| 439 | `SystemCoreClock` | 4 | Core/Src/system_stm32l4xx.c:172 | 2 | 3 | W:,SystemCoreClockUpdate R:,SystemCoreClockUpdate,SystemInit | HAL/libc 対象外 |
| 440 | `__env` | 4 | Core/Src/syscalls.c:39 | 1 | 1 | W:- R:- | HAL/libc 対象外 |
| 441 | `__malloc_free_list` | 4 |  | 0 | 0 | W:- R:- | HAL/libc 対象外 |
| 442 | `__malloc_sbrk_start` | 4 |  | 0 | 0 | W:- R:- | HAL/libc 対象外 |
| 443 | `__sbrk_heap_end` | 4 | Core/Src/sysmem.c:31 | 2 | 1 | W:,_sbrk R:_sbrk | HAL/libc 対象外 |
| 444 | `__stdio_exit_handler` | 4 |  | 0 | 0 | W:- R:- | HAL/libc 対象外 |
| 445 | `_impure_ptr` | 4 |  | 0 | 0 | W:- R:- | HAL/libc 対象外 |
| 446 | `environ` | 4 | Core/Src/syscalls.c:40 | 1 | 0 | W:- R:- | HAL/libc 対象外 |
| 447 | `errno` | 4 |  | 7 | 2 | W:_execve,_fork,_kill,_link,_unlink,_wait,_sbrk R:, | HAL/libc 対象外 |
| 448 | `uwTick` | 4 | Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal.c:90 | 0 | 0 | W:- R:- | HAL/libc 対象外 |
| 449 | `uwTickPrio` | 4 | Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal.c:91 | 0 | 0 | W:- R:- | HAL/libc 対象外 |
| 450 | `__lock___env_recursive_mutex` | 1 |  | 0 | 0 | W:- R:- | HAL/libc 対象外 |
| 451 | `__lock___malloc_recursive_mutex` | 1 |  | 0 | 0 | W:- R:- | HAL/libc 対象外 |
| 452 | `__lock___sfp_recursive_mutex` | 1 |  | 0 | 0 | W:- R:- | HAL/libc 対象外 |
| 453 | `__lock___tz_mutex` | 1 |  | 0 | 0 | W:- R:- | HAL/libc 対象外 |
| 454 | `uwTickFreq` | 1 | Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal.c:92 | 0 | 0 | W:- R:- | HAL/libc 対象外 |


## 5. プローブ 全50件 一覧

| 変数 | B | 宣言 | 書込 | 読出 | 用途 |
|---|---|---|---|---|---|
| `ADC_Span` | 20 | Core/Src/IM_110.c:37 | 3 | 6 | 空中の生mVを傾き基準に採用 → 空中出力=1700 |
| `ADC_Span_S` | 20 | Core/Src/IM_110.c:39 | 1 | 4 | 出荷時ADCゼロ点補正値 |
| `ADC_Zero` | 20 | Core/Src/IM_110.c:36 | 2 | 4 | ゼロ点を記録(未使用) |
| `AD_AVE_Count` | 1 | Core/Src/IM_110.c:22 | 2 | 2 | 移動平均件数 (0=無し) |
| `AD_AVE_mV` | 20 | Core/Src/IM_110.c:21 | 1 | 3 | 移動平均値 |
| `FLASH_Program_Fast` | 60 | Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_flash.c:678 | 0 | 0 | W:- R:- |
| `HSPPAD143_Pressure_hPa` | 4 | Core/Src/IM_110.c:30 | 3 | 2 | 気圧値 [hPa] |
| `LED_Out` | 20 | Core/Src/IM_110.c:38 | 3 | 7 | LED出力値 |
| `MCP3424_AD_Digit` | 20 | Core/Src/IM_110.c:19 | 1 | 5 | W:MCP3424_AD_Read R:(宣言のみ),,uart_Put_Measure_Data_Raw,uart_Set |
| `MCP3424_AD_mV` | 20 | Core/Src/IM_110.c:20 | 1 | 3 | W:MCP3424_AD_Read R:(宣言のみ),,AD_AVE_Calc |
| `MS_flag` | 1 | Core/Src/IM_110.c:16 | 2 | 2 | 冪等セット |
| `Probe_ID` | 4 | Core/Src/IM_110.c:35 | 3 | 4 | プローブID格納変数 |
| `Product_Name` | 7 | Core/Src/IM_110.c:34 | 0 | 5 | 製品名称格納配列 |
| `RecvData` | 512 | Core/Src/IIJIMA_Templete.c:13 | 4 | 16 | 単一ページ RPG,<N> |
| `RecvDataP` | 4 | Core/Src/IIJIMA_Templete.c:14 | 3 | 2 | W:,clear_RecvData,HAL_UART_RxCpltCallback R:(宣言のみ),HAL_UART_RxCpltCallback |
| `RxData` | 1 | Core/Src/IIJIMA_Templete.c:15 | 4 | 3 | W:HAL_UART_ErrorCallback,HAL_UART_RxCpltCallback,fla R:(宣言のみ),,HAL_UART_RxCpltCallback |
| `SystemCoreClock` | 4 | Core/Src/system_stm32g0xx.c:159 | 2 | 3 | W:,SystemCoreClockUpdate R:,SystemCoreClockUpdate,SystemInit |
| `__global_locale` | 364 |  | 0 | 0 | W:- R:- |
| `__lock___malloc_recursive_mutex` | 1 |  | 0 | 0 | W:- R:- |
| `__lock___sfp_recursive_mutex` | 1 |  | 0 | 0 | W:- R:- |
| `__malloc_free_list` | 4 |  | 0 | 0 | W:- R:- |
| `__malloc_sbrk_start` | 4 |  | 0 | 0 | W:- R:- |
| `__sbrk_heap_end` | 4 | Core/Src/sysmem.c:31 | 2 | 1 | W:,_sbrk R:_sbrk |
| `__sf` | 312 |  | 0 | 0 | W:- R:- |
| `__sglue` | 12 |  | 0 | 0 | W:- R:- |
| `__stdio_exit_handler` | 4 |  | 0 | 0 | W:- R:- |
| `_impure_data` | 76 |  | 0 | 0 | W:- R:- |
| `_impure_ptr` | 4 |  | 0 | 0 | W:- R:- |
| `ad_ave_buf` | 5100 | Core/Src/IM_110.c:25 | 0 | 3 | W:- R:,AD_AVE_Calc,MCP3424_Init |
| `ad_ave_idx` | 5 | Core/Src/IM_110.c:26 | 1 | 3 | W:AD_AVE_Calc R:,AD_AVE_Calc,MCP3424_Init |
| `ad_ave_num` | 5 | Core/Src/IM_110.c:27 | 1 | 3 | W:AD_AVE_Calc R:,AD_AVE_Calc,MCP3424_Init |
| `errno` | 4 |  | 7 | 2 | W:_execve,_fork,_kill,_link,_unlink,_wait,_sbrk R:, |
| `g_store` | 512 | Core/Src/IM_110.c:1048 | 7 | 3 | W:store_commit,store_load,store_new_probe_init,store R:uart_Read_Pages,uart_Read_Store,uart_Reset_Probe_F |
| `g_store_valid` | 1 | Core/Src/IM_110.c:1049 | 6 | 1 | 既定値を live へ反映 |
| `hdma_usart2_rx` | 92 | Core/Src/main.c:56 | 2 | 4 | W:HAL_UART_MspInit,DMA1_Channel1_IRQHandler R:,,HAL_UART_MspInit, |
| `hi2c1` | 84 | Core/Src/main.c:46 | 3 | 3 | W:MCP3424_AD_Read,MCP3424_Init,MX_I2C1_Init R:(宣言のみ),,MX_I2C1_Init |
| `hi2c2` | 84 | Core/Src/main.c:47 | 3 | 3 | W:HSPPAD143_ReadRegs,HSPPAD143_WriteReg,MX_I2C2_Init R:(宣言のみ),,MX_I2C2_Init |
| `hiwdg` | 16 | Core/Src/main.c:49 | 2 | 2 | W:MX_IWDG_Init,main R:,MX_IWDG_Init |
| `htim1` | 76 | Core/Src/main.c:51 | 4 | 4 | W:timer_set,timer_start,MX_TIM1_Init,TIM1_BRK_UP_TRG R:(宣言のみ),,MX_TIM1_Init, |
| `htim16` | 76 | Core/Src/main.c:53 | 5 | 4 | W:Set_SEL,uart_Set_SEL_Freq,MX_TIM16_Init,main,TIM16 R:(宣言のみ),,MX_TIM16_Init, |
| `htim3` | 76 | Core/Src/main.c:52 | 4 | 4 | W:Set_PWM_Duty,MX_TIM3_Init,main,TIM3_IRQHandler R:(宣言のみ),,MX_TIM3_Init, |
| `huart2` | 148 | Core/Src/main.c:55 | 7 | 4 | W:HAL_UART_ErrorCallback,HAL_UART_RxCpltCallback,fla R:(宣言のみ),,MX_USART2_UART_Init, |
| `mcp3424_ch_mask` | 1 | Core/Src/IM_110.c:23 | 2 | 2 | W:,MCP3424_Init R:MCP3424_AD_Read,MCP3424_Init |
| `mcp3424_current_ch` | 1 | Core/Src/IM_110.c:24 | 3 | 2 | W:,MCP3424_AD_Read,MCP3424_Init R:MCP3424_AD_Read,MCP3424_Init |
| `pFlash` | 28 | Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_flash.c:109 | 0 | 0 | W:- R:- |
| `timer_uart` | 4 | Core/Src/IM_110.c:42 | 4 | 3 | UART出力タイマー |
| `uart_end` | 1 | Core/Src/IIJIMA_Templete.c:19 | 3 | 2 | 1行受信完了 (ISR が立て main loop が処理) |
| `uwTick` | 4 | Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal.c:80 | 0 | 0 | W:- R:- |
| `uwTickFreq` | 1 | Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal.c:82 | 0 | 0 | W:- R:- |
| `uwTickPrio` | 4 | Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal.c:81 | 0 | 0 | W:- R:- |
