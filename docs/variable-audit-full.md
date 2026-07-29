# 全変数 総ざらい — 保存先・用途・参照箇所（ELF 起点・DO 撤去後 全面再生成）

> **生成日: 2026-07-27。ID-200T DO / 1Wire 撤去後のツリーから全面再取得したもの**（前版の手直し版は破棄）。
> 対象コミット: 本体 `f7fa7c4`（DO 計算・1Wire センサー管理 全撤去）/ プローブ `81ceb30`（STORE_VER=2）。
> **本体 361 変数・プローブ 52 変数を 1 件も省略せず掲載する**（§6 / §7）。
> 母数は ELF のシンボルテーブル。ソースの正規表現抽出は母数に使っていない。
>
> **追記 (同日・第2次)**: DO 残渣に続き、**1Wire / WAG レガシー一式を全撤去**した（本体のみ、`make clean && make -j` 通過）。
> これにより §2-1 の page5 衝突と §2-2 の「書くだけで読まれない保存」は**消滅**した（§2-9）。
> 本体変数は **448 → 384** に減った。§1 / §6 の数値と表は撤去後のもの。
>
> **追記 (2026-07-28・第3次)**: 自動校正 / 自動校正履歴 / 校正履歴データ `cal[]` を全撤去（§2-10）。
> 本体変数は **384 → 377**。§1 / §6 の数値と表は撤去後のもの。**実機未検証。**
>
> **追記 (2026-07-28・第4次)**: データロガー機能一式と BLE サブシステムを全撤去（§2-11）。
> 本体変数は **377 → 361**。§1 / §6 の数値と表は撤去後のもの。**実機未検証。**

---

## 0. 生成手順と判定ロジック

### 0-1. 母数の取り方

```bash
cd ../IM-110       && make clean && make -j
cd ../IM-110_Probe && make -j
arm-none-eabi-nm --defined-only -S -l build/IM-110.elf       | grep -E '^20[0-9a-f]{6} '   # 370 行
arm-none-eabi-nm --defined-only -S -l build/IM-110_Probe.elf | grep -E '^20[0-9a-f]{6} '   # 61 行
```

RAM 帯 (`0x20xxxxxx`) に実体を持つ定義済みシンボルが母数。ここから
**リンカスクリプト由来の 9 個**（`_estack` `_sdata` `_edata` `_sbss` `_ebss` `_end`
`__bss_start__` `__bss_end__` `__TMC_END__` — アドレス目印であって変数ではない）を除外する。

| | ELF 行数 | リンカ記号 | **変数として掲載** |
|---|---|---|---|
| 本体 IM-110 | 370 | −9 | **361** |
| プローブ IM-110_Probe | 61 | −9 | **52** |

`nm` がサイズを出さない 2 個（`completed.1` / `object.0` = newlib の crtstuff 内部）も
変数として残し、サイズ欄は `-` と表記した。関数内 static はコンパイラのマングル名
（`名前.N`）のまま掲載している。

### 0-2. 参照の数え方（W / R / & 列の定義）

Python で対象ツリーの `.c` / `.h` を全走査し、文字列リテラルとコメントを潰したうえで
識別子ごとに出現位置を分類した。走査対象は **本体 = `Core/` + `qrencode/`、プローブ = `Core/`**。
HAL ドライバ (`Drivers/`) は走査対象外（CMSIS 配布物に無関係な同名変数が大量にあり、
数値が汚染されるため）。したがって `uwTick` などドライバ内変数の W/R は 0 と出る。

- **W（書込）** = `x = …` / `x[i] = …` / `x.m = …` / `+=` 等の複合代入 / `++` `--` /
  `memset`/`memcpy`/`sprintf` 等の第 1 引数。**宣言に初期化子が付く場合も W に数える。**
- **R（読出）** = 上記以外の値としての参照。**宣言・extern 宣言は W にも R にも数えない。**
- **&（アドレス渡し）** = `&x` の形で関数へ渡された回数。呼び先が書くか読むかは
  呼び先次第なので W/R とは別枠にした。**W=0 かつ &>0 なら「ポインタ経由で書かれている」**と読む。

関数の帰属は波括弧の深さを追って決めている（0→1 の遷移直前の関数定義行を採用）。
**static 変数は宣言ファイル内の参照だけを帰属させる**（`initialized` のように
`qrencode/rsecc.c:41` と `Core/Src/IM_110.c:1115` に同名が存在するため）。

### 0-3. 判定（最右列）の決め方

上から順に評価し、最初に一致したものを採用する。

1. **HAL/libc 対象外** — 宣言が `Drivers/` / `syscalls.c` / `sysmem.c` / `system_*.c` にある、
   または newlib 内部名（`__*` `_impure*` `errno` 等）、または `main.c` の HAL ハンドル (`hxxx` / `hdma_*`)
2. **関数内 static** — nm 名にマングル `.N` が付くもの
3. **保存済** — 不揮発への往復経路がある。方向まで判定する
   - **書出** = 保存関数がその変数を *読む*（`write_param_*` / `eep_write_history` / `store_pack_from_globals` …）
   - **復元** = 復元関数がその変数に *書く*（`read_param_*` / `eep_read_history` / `store_unpack_to_globals` / `Apply_Coef_To_Live` …）
   - 表記は `保存済 <保存先> [書出+復元 / 書出のみ / 復元のみ]`
4. **★未参照** — W=R=&=0
5. **★ポインタ出力のみ・読出0** — W=0, &>0, R=0（＝出力引数として書かれるだけで誰も読まない）
6. **ポインタ経由で書込** — W=0, &>0, R>0
7. **FW固定テーブル/読出専用** — W=0, &=0, R>0
8. 以下は名前パターンによる分類 — **エラーカウンタ** → **タイマ/カウンタ** → **通信バッファ/状態** → **UI状態**
9. どれにも当たらなければ **★要判断**

> `eep_read_calhis` / `eep_write_calhis` / `eep_read_precal` / `eep_write_precal` は
> **中身が `return EEP_OK;` だけの no-op スタブ**（Eeprom.c:319/337/285/302）なので保存関数から外した。
> `Backup_WAGData_Read` も呼び出し元が 1 つも無いので外した（§2-2）。

### 0-4. 限界（先に明示する）

- 行番号は正確だが、**関数名の帰属には誤りがありうる**。断定する前に行番号を開くこと。
- **同一ファイル内に同名の関数内 static が複数ある場合**（`num.12` `num.21` `num.25` `num.29` など）は
  互いを区別できず、同じ参照リストを共有する。
- `&x` の検出は `&` が識別子に直接前置される形のみ。`& x` のように空白を挟む書き方は拾えない。
- DMA が書く配列（`adc_data`）はキャスト越しに渡されるため W にも & にも出ない（§2-4）。

---

## 1. 結論サマリ

### 1-1. 本体 IM-110 — 448 件

| 分類 | 件数 | 判定 |
|---|---|---|
| 保存済（不揮発への経路あり） | 62 | 内訳は下表 |
| **★要判断** | 92 | §4 で全件裁定 |
| タイマ/カウンタ | 51 | 揮発で正しい |
| HAL/libc | 40 | 対象外 |
| 関数内 static | 38 | 関数スコープ。揮発で正しい |
| UI 状態 | 31 | 揮発で正しい |
| エラーカウンタ | 19 | 揮発 or page4 集約 |
| 通信バッファ/状態 | 17 | 揮発で正しい |
| ポインタ経由で書込 | 6 | 揮発で正しい |
| **★ポインタ出力のみ・読出0** | 3 | §2-5 |
| FW固定テーブル/読出専用 | 2 | `adc_data`(DMA書込) / `Cal_Type`(§2-4) |
| **★未参照（デッド）** | **0** | ― |
| **合計** | **361** | |

保存済 62 件の保存先内訳（**legacy 1Wire の 41 件は撤去済みで消滅**）:

| 保存先 | 件数 |
|---|---|
| 統合ストア512B [書出+復元] | 25 |
| EEP page4 (本体情報/エラー) [書出+復元] | 13 |
| EEP page81-125 (測定履歴) [書出+復元] | 6 |
| EEP page12 (共通設定) + 統合ストア512B [書出+復元] | 6 |
| EEP page81-125 (測定履歴) [書出のみ] | 5 |
| EEP page12 (共通設定) [書出+復元] | 3 |
| EEP page12 + page4 + page5 [書出のみ] (= `eep_wdata`) | 1 |
| EEP page5 (界面設定) [書出+復元] | 1 |
| EEP page12 (共通設定) + EEP page81-125 (測定履歴) [書出+復元] | 1 |
| 統合ストア512B [復元のみ] (= `g_hstore`) | 1 |
| **計** | **62** |

### 1-2. プローブ IM-110_Probe — 52 件

| 分類 | 件数 | 判定 |
|---|---|---|
| HAL/libc | 28 | 対象外 |
| **★要判断** | 12 | §4-8 |
| 保存済（統合ストア512B / Flash page63） | 6 | `g_store` `g_store_valid` `ADC_Span` `LED_Out` `Product_Name` `Probe_ID` |
| 通信バッファ/状態 | 3 | 揮発で正しい |
| タイマ/カウンタ | 2 | 揮発で正しい |
| ポインタ経由で書込 | 1 | `RxData` |
| **合計** | **52** | |

> 前版は「プローブ 50 件」としていたが、リンカ記号の扱いを揃えると **52 件**が正しい。

---

## 2. 不具合・要処置の棚卸し

すべて実コードを開いて確認した。行番号は削除作業**前**のコミット時点のもの。
**§2-1〜§2-4 は既出**（§2-1 = 1Wire レガシー撤去の動機そのもの、§2-2〜§2-4 は `pending-decisions.md` で裁定済）。
新規に判明したのは §2-5 / §2-6 と、§2-7 の残渣の全量。

### 2-1. 【重大・解決済】EEPROM page5 の二重使用 — `Interface_Threshold` が電源 OFF ごとに破壊されていた

> **§2-9 の 1Wire 撤去により解消。** page5 に書き込むのは `write_param_interface()` だけになった。以下は撤去前の記録。

**事実**（すべて `IM-110/Core/Src` 配下）:

| # | 根拠 | 内容 |
|---|---|---|
| 1 | `Eeprom.h:111` | `#define EEP_INTERFACE_PAGE 5` — page5 は **界面測定設定 (`Interface_Threshold`)** |
| 2 | `Eeprom.c:2264-2268` | `write_param_interface()` が page5 に マジック `0x3C` + `Interface_Threshold` を書く |
| 3 | `Eeprom.c:961` | `write_param_1wireinfo()` が **`eep_write_verify_page(5)`** で page5 を WAG_* データで上書きする（先頭は `WAG_Header` の生バイト） |
| 4 | `mainSub.c:515` | `shutt_down()` 内で **無条件に** `Wire_Backup_flag = 2;`（「特に理由が無ければ1Wireデータは毎回バックアップ」） |
| 5 | `mainSub.c:824-830` | 同じ `shutt_down()` の後半で `if (Wire_Backup_flag == 2)` → `eep_write_1wireinfo()` / `eep_write_1wirebk()` |
| 6 | `mainSub.c:741` | 同じ `shutt_down()` の **前半**で `eep_write_setting()`（→ `write_param_interface()` = page5 書込） |
| 7 | `Eeprom.c:2238` | `read_param_interface()` は先頭バイトが `0x3C` でなければ `EEP_CS` を返す |
| 8 | `Eeprom.c:1233-1241` | 起動時 `eep_init_param()` は `EEP_CS` を受けると `Interface_Threshold = INI_INTERFACE_THRESHOLD` に戻して page5 を書き直す |

**帰結**: `shutt_down()` は 741 行（page5 = 界面設定を書く）→ 829 行（page5 = WAG データで上書き）の順に走る。
`WAG_Header[0]` が `0x3C` になることはまず無いので、**次回起動時 page5 のマジック判定は必ず失敗し、
`Interface_Threshold` は毎回 FW 既定値へ戻る**。ユーザーが設定した界面判定しきい値は 1 電源サイクルも保たない。

**処置の選択肢**: (a) `eep_write_1wireinfo()` / `eep_write_1wirebk()` の呼び出しを撤去する
（読み側は既に死んでいる = 下記 2-2）、(b) legacy 1Wire の書込先を page5-9 から未使用ページへ退避する。
**(a) が妥当**。呼び出し元は `mainSub.c:829` / `Adjust.c:530` / `Adjust.c:775` / `LinkSerial.c:1105` の 4 箇所。

### 2-2. 【解決済】legacy 1Wire (page5-8 / page9) は「書くだけで二度と読まれない」状態だった

> **§2-9 の 1Wire 撤去により解消。** 以下は撤去前の記録。

- 読み出し側の入口は `Backup_WAGData_Read()`（`Eeprom.c:1295`）だけで、
  **呼び出し元が 1 つも存在しない**（宣言 `Eeprom.h:182` と定義のみ）。
- `eep_read_1wireinfo()` / `eep_read_1wirebk()` も `Backup_WAGData_Read()` からしか呼ばれない
  （`Eeprom.c:1304` / `1318`）。
- 一方で書込側 `eep_write_1wireinfo()` / `eep_write_1wirebk()` は電源 OFF ごとに走る。

→ **保存済 111 件のうち 41 件（page5-8 が 30 件、page9 が 16 件、うち重複あり）は、
実質「電源 OFF のたびに EEPROM を消耗させるだけ」の書込**になっている。
撤去するか、正式な保存先へ移すかの決めが要る。

### 2-3. 【解決済】`cal` / `pre_cal` / `WAG_SNo_CAL` — 一度も書かれず、校正履歴が常に空だった

> **§2-10 の自動校正撤去で `cal[]` ごと消滅。** 以下は撤去前の記録。

- `cal[CAL_NUM]`（`mainSub.h:276`）: W=0 / R=28。`pre_cal`（`mainSub.h:277`）: W=0 / R=10。
- 書き手であるはずの `eep_read_calhis()` / `eep_write_calhis()` / `eep_read_precal()` / `eep_write_precal()` は
  **すべて no-op スタブ**（`Eeprom.c:319` `337` `285` `302`。コメントに「IM-110 では未使用、
  レイアウト v2.0 で page 削除済み」と明記）。
- ソース全体で `cal[...] = ` / `memcpy(&cal…` 相当の書込は 0 件。

→ **校正履歴の表示は常にゼロ埋めの初期値を出している**。`WAG_SNo_CAL`（`mainSub.h:283`, W=0/R=1）も同様で、
`LinkSerial.c:524` の応答電文に常に 0 が乗る。UI/電文ごと撤去するか、書込を実装するかの決めが要る。

### 2-4. `Cal_Type` / `Stbl_sel` — 一度も書かれないまま分岐条件に使われている

| 変数 | 宣言 | 読出箇所 | 帰結 |
|---|---|---|---|
| `Cal_Type` | `IM_110.c:88`（校正種別 0:ゼロ 1:2pスパン 2:3p中間） | `Normal.c:746` / `Normal.c:2677` の `switch(Cal_Type)` | 代入が 1 箇所も無い → **常に 0（ゼロ校正）の分岐しか通らない** |
| `Stbl_sel` | `AutoStable.h:15`（自動安定判断条件の選択） | `AutoStable.c:80` の `if (Stbl_sel)` | 同上 → 常に偽。もう一方の安定判断条件は到達不能だった → **本セッションで変数ごと削除**（§2-8） |

`adc_data`（`Adc.h:27`）も W=0 と出るが、これは誤検知ではなく仕様で、
`Adc.c:28` の `HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_data, ADC_CH_NUM)` により
**DMA が直接書く**（キャストを挟むため静的解析には出ない）。こちらは正常。

### 2-5. `MLSS_ABSS` / `SS_ABSS` / `TR_ABSS` — ポインタ出力のみ・読出 0

3 件とも「W=0 / R=0 / &=1」。唯一の参照は出力引数としての受け渡しで、値を読む箇所が無い。

| 変数 | 宣言 | 唯一の参照 |
|---|---|---|
| `MLSS_ABSS` | `IM_110.c:96` | `IM_110.c:1244` `MLSS_FABSS_from_raw_mv(…, &MLSS_ADC_mV, &MLSS_ABSS)` |
| `SS_ABSS` | `IM_110.c:192` | `IM_110.c:1287` `SS_FABSS_from_raw_mv(…, &SS_ADC_mV, &SS_ABSS)` |
| `TR_ABSS` | `IM_110.c:245` | `IM_110.c:1342` `TR_FABSS_from_raw_mv(…, &TR_ADC_mV, &TR_ABSS)` |

対の `*_ADC_mV` は `IM_110.c:2200` / `2270` / `2316` で読まれている（デバッグ表示用）ので生きている。
`*_ABSS` は **デバッグ用の観測点として置いたまま使われていない**。残すなら表示に繋ぐ、
使わないなら出力引数ごと落とす、の決めが要る。

### 2-6. 「代入はされるが、誰もその値を読まない」変数 25 件

**この節の意味**: 変数に値を書くコードは存在するのに、**その変数を読むコードがソース上に 1 つも無い**もの。
書いた値は誰にも使われず捨てられる（＝デッドストア）。`x = 5;` と書いた行が全部無駄になっている状態。
HAL/libc と関数内 static を除いた、**W>0 かつ R=0 かつ &=0** の全件が下記。

`Stbl_STD` `log_interval_setting` `WAG_HstCal_Date` `WAG_HstMeas_Date` `Probe_cmd_pending`
`RTC_ResetOver` `RTC_ResetYear` `select_JIS` `v33_off_cmd` `v33_on_cmd` `WAFG_Flag`
`WAG_HstCal_Type` `ZEROCAL_flag` `WAG_HstErr_Date` `WAG_HstErr_Type` `WAG_HstCal_Time`
`WAG_HstMeas_Time` `AutoCal_time_sel` `RTC_ResetCount` `lcd_off_cmd` `log_disp_flag`
`WAG_HstCal_Stable` `WAG_HstMeas_Stable` `bt_off_cmd` `bt_on_cmd`

内訳の要点:

- **`Stbl_STD`（`mainSub.h:60`, 自動安定判断条件の切替値）** — 参照は全部で 3 箇所しかない:
  宣言（`mainSub.h:60`）、`Eeprom.c:609` の `Stbl_STD = WAG_StbChange;`（= `read_param_1wireinfo` 内）、
  `mainSub.c:1312` の `Stbl_STD = WAG_StbChange;`（= `Default_InfoData_Set` 内）。**読む側が 1 つも無い。**
  前版は「`normal_disp` と `start_measure` で現役」と書いていたが、**DO 撤去でその読出は消えている**。
- `WAG_Hst*` 系（校正/測定/エラー履歴の日付・種別・所要時間・ふらつき）は
  DO 撤去で表示側が消え、書込だけが残った。
- `v33_on_cmd` / `v33_off_cmd` / `lcd_off_cmd` / `bt_on_cmd` / `bt_off_cmd` は
  電源系コマンドフラグ。立てるコードはあるが見るコードが無い。
- `log_interval_setting`（`Timer.h:67`, W=7/R=0）/ `log_disp_flag` はロガー未実装に伴う残置。

いずれも「代入行ごと消す」か「読む側を実装する」かの二択。`Stbl_STD` `stbwidthA/B` `stbtime` `stbsize` `Stbl_sel` は
`pending-decisions.md` §0 で **「変数は残す・保存しない・`#define` 初期値」** と裁定済みなので、
残り（`WAG_Hst*` / `v33_*_cmd` / `lcd_off_cmd` / `bt_*_cmd` / ロガー系）が未裁定。今回は削除していない。

### 2-7. DO 撤去の残渣 — **本セッションで削除済み**

> **(c) のロガー機能は §2-11 で撤去済み。**

§5 の通り、DO 撤去で RAM から消えたのは 67 変数。うち 15 件はソースに宣言が残っていた。
**訂正**: 初版でこの 15 件を一律「使用箇所ゼロ」と書いたのは誤り。実際は 3 種類に分かれる。

#### (a) 完全に未参照だった 7 件 → **削除した**

| 変数 | 削除した宣言 |
|---|---|
| `WAG_HstCal_Amp` | `mainSub.h:164` |
| `WAG_HstCal_Temp` | `mainSub.h:165` |
| `WAG_HstMeas_Amp` | `mainSub.h:171` |
| `WAG_HstMeas_Temp` | `mainSub.h:172` |
| `WAG_HstMeas_Meas` | `mainSub.h:173` |
| `force_range_set_flag` | `mainSub.h:320` |
| `force_range` | `mainSub.h:321` |

#### (b) 到達しない DO 表示コードからのみ参照されていたもの → **コードごと削除した**

- `flash_calc()` の **`FLASH_VAL` ブロック**（酸素濃度点滅）と **`FLASH_TEMP` ブロック**（水温点滅）、
  および空気飽和率描画ブロック。全 49 呼出が `pdspdo = pdspAcc = pdspTemp = pdspair = NULL` だったため到達不能だった。
- 引数を 4 本落として `uint8_t flash_calc(flash_item itembit, uint8_t disp, uint8_t u_colon)` に変更。**呼出 49 箇所を追従**。
- 上記ブロック内だけで使われていた `do_flash_flag`（`Normal.c:36`）/ `temp_flash_flag`（`Normal.c:37`, `Setting.c:25`）/
  `empty_do` / `empty_temp` の宣言。
- 呼び先の `LS027_disp_change_do()` / `LS027_disp_change_temp()` / `LS027_disp_change_air()` の定義と宣言
  （`IIJIMA_Templete.c` / `.h`）。他に呼び手は無かった。
- `enum flash_item` から `FLASH_VAL` / `FLASH_TEMP` / `FLASH_ALL` を削除し `FLASH_BAT` のみに。
- `Normal.h:74` の `EXTERN void do_calc(void);`（**定義が存在しない宙ぶらりん宣言**）。
- `nrm_measure()` 内のデッドローカル `dspdo` / `dspTemp` / `dspair`（`Normal.c:2228-2230`）。

**結果**: `make -j` 通過。`text` 448,240 → **446,264 B（−1,976 B）**。`bss` は 21,928 で不変
（削除対象の変数は元々リンカが落としていたため RAM は変わらない）。
削除後にソース全体を再走査し、上記 18 個の識別子が **1 件も残っていない**ことを確認済み。

#### (c) 「死んだロガー機能」から参照されていたもの → **削除していない（DO ではない）**

初版で「使用箇所ゼロ」と書いた `log_dat` / `log_head` は誤りで、実際は参照が 15 / 77 箇所ある。
ただしその参照元がすべて到達不能である。

| 事実 | 根拠 |
|---|---|
| `nrm_data_logger()` は **ELF に存在しない**（プロトタイプ以外に呼び出し元が無く gc-sections が除去） | `Normal.c:2834` 定義 / 呼出 0 |
| `log_dat` / `log_head` の参照元は `nrm_data_logger` と、同じく除去された `Qrcode.c` / `Display.c:4445-4526` / `ble.c` のロガー画面群のみ | ― |
| `set_disp_log()` も `defined but not used` 警告が出る | ビルドログ |

**ロガーは未実装のまま「将来実装する」と `Eeprom.h:12-14, 44-46` に明記されている機能**であり、
DO 撤去の残渣ではない。撤去するか作り直すかは別途の判断。

#### (d) 触っていないもの

- **1Wire レガシー（page5-9）一式** — `pending-decisions.md` §0 で「変数・関数・呼び出しごと削除」と裁定済みだが、
  §2-1 のバグ潰しとして**現在進行中の作業**のため手を出していない。
- `Eeprom.h:70` / `AllDef.h:304`(`ERR1W`) / `Normal.h:54`(`test_WIRE`) / `mainSub.h:24,27` の 1Wire 由来の定義・コメントも同様。
- `Display.c` の `disp_do_temp_data()` / `disp_st_sp_temp_data()` は**名前が DO 由来なだけで履歴表示に使われており生きている**。消してはいけない。

### 2-8. `calcu_stable_CAL()` の 1Wire レガシー依存 — **本セッションで撤去済み**

**撤去前**: 校正時の安定判断条件だけが legacy 1Wire の値を使っていた（測定時は既に `#define` 化済み）。

| 経路 | 使っていた値 |
|---|---|
| 測定時 `calcu_stable()` | `MLSS/SS/TR_StableA-D` = **すべて `#define`**（`IM_110.h:232-235` ほか） |
| 校正時 `calcu_stable_CAL()` | `Stbl_sel` + `WAG_StableB/C`（`Stbl_sel`≠0 なら `BH/CH`）+ `WAG_Type` |

`calcu_stable_CAL()` は `start_measure()`（`mainSub.c:2018`）から `operation_mode & 0xFF00 == ADCAL` のとき呼ばれる**生きた経路**。

**撤去後**: FW 固定値へ置換した（`AutoStable.c:22-25` に `#define`）。

| 新 `#define` | 値 | 根拠 |
|---|---|---|
| `CAL_STB_TIME` | `5`（秒） | 旧コメント「安定判断時間は5秒で決め打ち」。旧実装の `5 * 100` は **100Hz 機 (ID-350T) のサンプルレート由来**で、IM-110 はプローブ約 2Hz。`calcu_stable()` と同じ「秒 × 2」に揃えた |
| `CAL_STB_WIDTH_A` | `0.02f` | 旧 `WAG_StableB` の既定値（`mainSub.c:1298`） |
| `CAL_STB_WIDTH_B` | `0.10f` | 旧 `WAG_StableC` の既定値（`mainSub.c:1299`） |

- `Stbl_sel` は代入が 1 箇所も無く常に 0 だったため、高濃度側 (`BH`/`CH`) の枝は到達していなかった。
  低濃度側の値だけを引き継ぎ、**`Stbl_sel` の宣言（`AutoStable.h:15`）は削除**した。
- `WAG_Type` は `Judge_Wagnit()` が `WAG_Header` から判定するが、`WAG_Header` を埋めるのは
  `Default_InfoData_Set()`（起動時に呼ばれない）と死んだ `read_param_1wireinfo()` だけなので、
  実行時は常に 0 = WA-TL 相当だった。20 秒枝も到達していない。
- **副次的に潰したバグ**: 旧実装は `stbsize = 5 * 100 = 500` のまま `data_stable[STB_TBL_SIZE=255]` を
  参照しており、`cnt_OverWrite` が立つまで（約 127 秒）**配列範囲外アクセス**になっていた。
  `calcu_stable()` と同じクランプを追加した。

**動作差分**: 校正時の安定判断サンプル数が `500`（実際は範囲外参照込み）→ `10`（5 秒 × 2Hz）になる。
判定幅は `stbwidthA/B` が下限フロアで、実効幅は「測定値の ±10%」が支配的（`AutoStable.c:263-267`）なので
フロア側の影響は小さい。**実機での校正動作確認が要る。**

**残っている `WAG_Stable*` / `WAG_Type`**: 参照元は `Eeprom.c` の `read/write_param_1wireinfo`（page5-8）、
`Default_InfoData_Set()`、`LinkSerial.c` のデバッグダンプのみ。**すべて 1Wire レガシーブロックの内側**で、
そのブロックごとの撤去（§2-1 / §2-2）で消える。EEPROM レイアウトに触るため単独では切り離さない。

### 2-9. 1Wire / WAG レガシー 全撤去 — **本セッションで実施**

`pending-decisions.md` §0 の裁定「1Wire レガシー（page5-9）は変数・関数・呼び出しごと削除」を実行した。

**削除した関数**

| ファイル | 関数 |
|---|---|
| `Eeprom.c` | `read_param_1wireinfo` / `write_param_1wireinfo`（page5-8）、`read_param_1wirebk` / `write_param_1wirebk`（page9）、`eep_read_1wireinfo` / `eep_write_1wireinfo` / `eep_read_1wirebk` / `eep_write_1wirebk`、`Backup_WAGData_Read` |
| `mainSub.c` | `Judge_Wagnit`、`Ident_Wagnit_Err`、`WAG_ExpireCheck`、`Default_InfoData_Set`（中身が全て 1Wire 既定値だった） |
| `LinkSerial.c` | `Send_1wire_Data`（`R1D`）、`Write_1wire_Data`（`W1D`）、`Clear_1Wire_Info`（`CWI`）と各コマンド分岐、`#if WIRE_ENABLE` ブロック 2 箇所 |

**削除したコードブロック**

- `shutt_down()`: 1Wire 測定履歴の記録ブロック（`WAG_Size == 20` 配下）、センサー入替判定、`Wire_Backup_flag == 2` のバックアップ書込。
  併せて未使用になった局所変数 `res` / `nd` / `ld` / `wrk` / `m_month` とラベル `ShuttDown_JP1` も削除。
- `start_measure()`: 1Wire テスト結果表示、保証期限 / センサー寿命 / 使用不可の各エラー判定。
- `Normal.c`: 校正回数カウント（`WAG_Total_Span/Zero`）、校正履歴・エラー履歴の WAG 部分、到達しない `case ERR1W:`、未使用になった `extime`。
- `Qrcode.c`: 問い合わせ QR の `sNo=` / `sDate=`（1Wire センサー由来）。
- 定義: `WIRE_ENABLE` / `ERR1W`（`AllDef.h`）、`ERR_1WIRE` / `INIT_1WIRE`（`mainSub.h`）、`test_WIRE`（`Normal.h` 列挙子）。

**リネーム**: `WAG_flash_flag` → **`meas_flash_flag`**。1Wire とは無関係な測定値表示の点滅フラグで、名前だけが紛らわしかった。

**副次的に直った箇所**

- **`main.c` の EEPROM 復旧が塞がっていた**: `if (f_IC_err & ERR_EEPROM) { if (f_IC_err & ERR_1WIRE) { init_param_set(); SYS_ErrorNo = 90; } }` の内側が
  **一度も真にならないビット**（`ERR_1WIRE` を立てるコードは 1Wire 撤去前から存在しなかった）で塞がれており、
  EEPROM 読込失敗時のパラメータ再初期化が走らなかった。内側の判定を外し、`ERR_EEPROM` だけで復旧するようにした。
- `mainSub.c` の電源 OFF 時 `SYS_ErrorNo = 91` も同じ `ERR_1WIRE` ゲートで塞がれていたため、`Eres` のみの判定にした。

**撤去で消えた変数 65 件**（`meas_flash_flag` は改名により +1）

`Stbl_STD` `Stbl_sel` `WAG_AveSize` `WAG_Base` `WAG_Date` `WAG_Date_Meas1` `WAG_Date_SEnd` `WAG_FairH` `WAG_Fflow` `WAG_Flag_SEnd` `WAG_FtempH` `WAG_Header` `WAG_Header_Back` `WAG_HstCal_Date` `WAG_HstCal_Stable` `WAG_HstCal_Time` `WAG_HstCal_Type` `WAG_HstErr_Date` `WAG_HstErr_Type` `WAG_HstMeas_Date` `WAG_HstMeas_Stable` `WAG_HstMeas_Time` `WAG_Limit` `WAG_OmakeDay` `WAG_OmakeDay2` `WAG_OmakeDay2_2` `WAG_OmakeDay_2` `WAG_Over` `WAG_POS_Cal` `WAG_POS_Err` `WAG_POS_Meas1` `WAG_POS_Meas2` `WAG_RestDay` `WAG_SNo` `WAG_SNo_Back` `WAG_SNo_CAL` `WAG_Size` `WAG_Span` `WAG_StableA` `WAG_StableAH` `WAG_StableB` `WAG_StableBH` `WAG_StableC` `WAG_StableCH` `WAG_StableD` `WAG_StableDH` `WAG_StableT` `WAG_StbChange` `WAG_Total_POn` `WAG_Total_Span` `WAG_Total_Zero` `WAG_Type` `WAG_Zero` `WAG_flash_flag` `WIRE_ErrorNo` `WIRE_Init_Err` `WIRE_Init_Retry` `WIRE_Read_Err` `WIRE_Read_Retry` `WIRE_Write_Err` `WIRE_Write_Retry` `Wire_Backup_flag` `Wire_ReadError_Count` `Wire_WriteError_Count` `wire_test_flag`

**ビルド結果**

| | 撤去前 (DO 撤去後) | 撤去後 |
|---|---|---|
| `text` | 446,064 B | **437,400 B**（−8,664） |
| `bss` | 21,928 B | **21,696 B**（−232） |
| RAM 変数 | 448 | **384** |
| ソース行 | ― | **−2,768 行 / +131 行**（16 ファイル） |

`make clean && make -j` で error 0。実機未検証。

**残っている警告（本撤去とは無関係の既存不具合）**

- `mainSub.c:107` `Check_AutoCal_History()` が **`control reaches end of non-void function`**。
  外側の `if` が偽のとき戻り値が不定のまま `calhisret` として使われる。撤去前から存在。**別途の判断が要る。**
- `Normal.c` の `prog_bar` / `d1` / `set_disp_log` は死んだロガー機能側（§2-7 (c)）。

### 2-10. 自動校正 / 自動校正履歴 / 校正履歴データ 全撤去 — **本セッションで実施**

`Check_AutoCal_History()` の `control reaches end of non-void function` 警告（§2-9 の残課題）を起点に
自動校正まわりを全数調査し、**すべて到達不能または定数畳み込み可能**と確認したうえで撤去した。

**到達不能だった根拠**

| 対象 | 根拠 |
|---|---|
| 自動校正の起動 | `rtc_alarm_check()` の唯一の呼出が `main.c` でコメントアウト済み → `WAFG_Flag` / `DAFG_Flag` は 0 固定 |
| 自動校正時刻設定 `ACALSET_1..6` | `operation_mode = ACALSET_1` を実行するコードが存在しない（`Setting.c` の case 群自体が `/* */` でコメントアウトされていた） |
| 校正履歴表示 `DISPCAL_1..6` | 同様に `operation_mode = DISPCAL_1` が存在しない |
| `err_ACAL1..7` / `gui_ACALCOMP` | `oc_err_num` へ代入するのは `Check_AutoCal_History()` の結果を見る 3 分岐のみ |
| `cal[]` / `pre_cal` | 書込ゼロ（`eep_*_calhis` / `eep_*_precal` が no-op スタブ）。§2-3 |

**削除した関数**

| ファイル | 関数 |
|---|---|
| `Rtc.c` | `rtc_alarm_check` |
| `mainSub.c` | `Check_AutoCal_History` |
| `LinkSerial.c` | `Read_SCAL_Data` |
| `Normal.c` | `set_disp_cal` |
| `Display.c` | `disp_ACALSET` / `disp_AUTOCAL_COMP` / `disp_DISPCAL` / `disp_cal_data` / `disp_cal_datetime` / `disp_ERROR17_1,2` / `disp_ERROR19_1,2` / `disp_st_sp_temp_data` |
| `Eeprom.c` | `eep_read_calhis` / `eep_write_calhis` / `eep_delete_calhis` / `eep_clear_calhis` / `eep_read_precal` / `eep_write_precal` |

**削除したその他**

- operation_mode 定義 `ACALSET_1..6` / `DISPCAL_1..6`（`AllDef.h`）と対応する case 群
- 列挙子 `err_ACAL1..7` / `gui_ACALCOMP`（`Normal.h`）とエラー表示分岐（`Normal.c` ERRDSP1/ERRDSP2、`Qrcode.c` のナレッジ URL）
- 定数 `ACAL_SP_HI` / `ACAL_SP_LO` / `ACAL_T_HI` / `ACAL_T_LO`（参照ゼロ）
- 型 `cal_data` と `CAL_NUM`、変数 `cal[]` / `pre_cal` / `AutoCal_time_sel` / `dispACAL_flag`
- シリアルコマンド **`RCD`（校正履歴取得）/ `CCH`（校正履歴消去）**
- `start_measure()` の自動校正履歴判定 3 分岐と局所変数 `calhisret`

**定数畳み込み（`DAFG_Flag` は常に 0）**

| 箇所 | 変更 |
|---|---|
| `Normal.c` 測定開始NG | 「自動校正時は電源OFF」分岐を落とし else 側のみ残す |
| `Normal.c` ゼロ校正開始 | `(MEM_sw_check() == 4) 論理和 DAFG_Flag` → `MEM_sw_check() == 4` |
| `Normal.c` バッテリーエラー | 同上、else 側のみ残す |
| `mainSub.c` | `if (!DAFG_Flag) { … }` をブロック展開 |
| `main.c` | `(!SW2) 論理和 DAFG_Flag` → `!SW2`（**CALDSP = SW2 押下の校正起動は生きている**） |

**消えた変数 6 件**（増えた変数 0 件）

`AutoCal_time_sel` `DAFG_Flag` `WAFG_Flag` `cal` `dispACAL_flag` `pre_cal`

**残したもの（名前が紛らわしいが自動校正ではない）**

| 残す | 理由 |
|---|---|
| `ADCAL_1..9` | **スパン校正（手動）**。"AD CAL" であって自動校正ではない |
| `CALDSP_1..3` | 校正起動画面。SW2 押下の電源 ON で入る生きた経路 |
| `CAL_HSEL_*` / `MCAL_*` | 後校正の記録選択。`his[]`（測定履歴）を使い `cal[]` は使わない |
| `HCLR` コマンド | `eep_clear_history()` = **測定履歴**消去。校正履歴ではない |
| `icon_l_calhis` | スパン校正画面の「長押しで校正履歴」アイコン。実際の遷移先は `CAL_HSEL_1`（後校正の記録選択）で**機能は生きている**。アイコン名が古いだけ |

**ビルド結果**

| | 1Wire 撤去後 | 自動校正撤去後 |
|---|---|---|
| `text` | 437,400 B | **431,792 B**（−5,608） |
| `bss` | 21,696 B | **21,064 B**（−632） |
| RAM 変数 | 384 | **377** |

`make clean && make -j` で error 0。**実機未検証。**

**解消した既存警告**: `mainSub.c` の `Check_AutoCal_History()` `control reaches end of non-void function`
（§2-9 末尾で「別途の判断が要る」としていたもの）は、関数ごと撤去したため消滅した。

### 2-11. データロガー / BLE 全撤去 — **本セッションで実施**

ELF に存在しない関数（＝リンカが除去した到達不能関数）をソース定義と突き合わせて全数列挙し、
**ロガーと BLE がまるごと到達不能**であることを確認したうえで撤去した。

**到達不能だった根拠**

| 対象 | 根拠 |
|---|---|
| データロガー | `nrm_data_logger()` は定義のみで呼出元ゼロ。`log_dat` / `log_head` を参照するのは、同じく到達不能な `Qrcode.c` / `Display.c` / `ble.c` のロガー画面群だけ |
| BLE | `BleServer()`（状態機械の本体）は呼出元ゼロ。唯一の外部入口 `BleServerInit()` は `Setting.c` の `BT_RESET_4` から呼ばれるが、`BT_RESET` 画面へ遷移するのは `Check_ret_mode()` のみで、その `Check_ret_mode()` 自体が 1Wire 撤去（§2-9）で呼出元を失っていた |

`ble.c` の `BleServer()` は中身がロガーデータ転送そのもの（転送バッファ `LOGDATA` = `8 + LOG_NUM*4`、
WV コマンド `5244`(RD) / `4444`(DD) / `5253`(RS) / `5753`(WS) がロガー履歴の読み書き）で、
ロガーを外すと骨格しか残らないため、**BLE ごと撤去**を選択した（ユーザー裁定）。

**削除したファイル**

- `Core/Src/ble.c` / `Core/Inc/ble.h`（Makefile の `C_SOURCES` からも除去）

**削除した関数**

| ファイル | 関数 |
|---|---|
| `Normal.c` | `nrm_data_logger` / `check_log_start` / `sort_log_index` / `clear_log_data` / `set_disp_log` |
| `Display.c` | `disp_LOG1` / `disp_LOG3` / `disp_LOG4` / `disp_LOG5` / `disp_DISPLOG` / `disp_BT_RESET` |
| `Eeprom.c` | `eep_clear_logh` / `eep_read_logh` / `eep_read_logd` / `eep_write_log` |
| `Qrcode.c` | `qrcode_log_disp` / `qrcode_log_calcp` |
| `LinkSerial.c` | `Set_logger_interval`（`LI` コマンド） |
| `mainSub.c` | `Set_logger_timer` |
| `IIJIMA_Templete.c` | `LS027_G_Write` / `LS027_G_Clear` / `LS027_G_Set`（ロガーのグラフ描画） |
| `main.c` | `bt_power_on` / `bt_power_off` |

**削除したその他**

- operation_mode 定義 `LOGGER` / `LOG_1..LOG_D` / `BT_RESET_1..6`、`MEASMODE` 画面（コメントアウト済みだった）
- 型 `log_data` / `log_header`、変数 `log_dat[2000]` / `log_head[10]` / `LOGDATA[8008]` ほか `log_*` 一式
- 定数 `LOG_NUM` / `LOG_SEL_MAX` / `LOG_MAX_PDATA` / `LOG_DATA_MAXP` / `LOG_DATA_OFFSET` / `LOG_DISP_MAX` / `LOGGER_*`
- `bt_on_flag` / `bt_on_cmd` / `bt_off_cmd` / `ble_bar_flag`、BLE/BT タイマー 4 本
  (`ble_recv_timer` / `bt_on_timer` / `bt_off_timer` / `bt_first_off_timer`)、`Setting.c` の `BT_RESET` 画面
- `measure_mode_flag`（ロガーモード判別）は常に 0 として畳み込み。`auto_power_off()` のロガーモード分岐、
  `shutt_down()` / `start_measure()` のガイダンス条件から `LOGGER` を除去

**移設**

- `Change_uart_bps()` は `main.c` から呼ばれている**生きた関数**だったため、`ble.c` から `LinkSerial.c` へ移設
  （宣言は `LinkSerial.h`）。

**ビルド結果**

| | 自動校正撤去後 | ロガー/BLE 撤去後 |
|---|---|---|
| `text` | 431,792 B | **430,968 B**（−824） |
| `bss` | 21,064 B | **21,024 B**（−40） |
| RAM 変数 | 377 | **361** |

`text` / `bss` の減りが小さいのは、**削除対象がすでにリンカに除去されていた**ため（ソースからの
除去であって、バイナリからの除去ではない）。`make clean && make -j` で error 0。**実機未検証。**

## 3. 保存が必要なのに保存されていないもの

| 変数 | B | 宣言 | 現状 | 必要な対応 |
|---|---|---|---|---|
| `LED_Out[1..4]`（プローブ） | 20 | `IM_110.c:38` | 統合ストアに `[0]` のみ | LED PWM 2 系統化に必須 |
| `Adb_Ref_MLSS2/3` `Adb_Ref_SS2/3` `Adb_Ref_TR2/3` | 各4 | `IM_110.c:291-296` | 保存なし | ADBOAD 出荷時 3 点の基準器値。電源で初期値へ戻る |
| `Depth_offset` | 4 | `IM_110.c:1350` | 保存なし・用途が 3 つ相乗り | §4-1 |
| `AD_AVE_Count`（プローブ） | 1 | `IM_110.c:22` | Page2 に枠のみ未結線 | 本体が毎起動 `SADA,0` を送るため実害は限定的。枠ごと削除が妥当 |
| `mcp3424_ch_mask`（プローブ） | 1 | `IM_110.c:23` | Page2 に枠のみ未結線 | 同上 |
| `stbwidthA` `stbwidthB` `stbtime` `stbsize` | 各1-8 | `AutoStable.c:22-24` | 保存なし | 安定判断の判定パラメータ。FW 固定でよいか決めが要る（§4-4） |
| `Stbl_STD` | 4 | `mainSub.h:60` | legacy 1Wire page5-8 から復元されるのみ・**読出 0** | §2-6。移設先を決めるか撤去する |
| `Interface_Threshold` | 4 | `IM_110.c:36` | page5 に保存されるが**毎回破壊される** | §2-1 |

---

## 4. ★要判断 103 件（本体）+ 12 件（プローブ）の裁定

### 4-1. `Depth_offset` / `Depth_offset_set` — 保存が要る（用途分離が前提）

`Depth_offset`（`IM_110.c:1350`）の書込は 3 箇所: 宣言初期化、`Calc_Depth:1363`（電源 ON 初回の自動ゼロ）、
`Depth_Calib_Zero:1380`。読出は `Calc_Depth:1367` と `Depth_Calib_Span6m:1393`。
`Depth_Calib_Zero()` は **現場の界面ゼロ校正と ADBOAD 出荷時ゼロの両方から呼ばれる**ため、
3 用途が 1 変数を共有している。→ 出荷時ゼロを別変数に分離してそちらを保存する。
電源 ON 大気圧スナップショットは揮発のままでよい。

### 4-2. 測定・校正の中間計算値（保存不要・揮発で正しい）

`MLSS/SS/TR_FABSS`（モード変換後）、`MLSS/SS/TR_1A`・`1B`（FABSS フル/半スパン校正値）、
`MLSS/SS/TR_CAL_REF`（校正実行中の一時値）、`MLSS/SS/TR_Hold`（安定ホールド値。ただし R=0 → §2-6 と同様の要確認）、
`MLSS_inst`（瞬時値）、`Interface_Hold`、`ADC_mV` / `ADC_mV_ave`（プローブ受信値と本体移動平均）、
`ad_ave_buf` / `ad_ave_idx`、`f_ad` / `f_ad_data`（電池電圧移動平均）、`Y2`（電池 mV）。

いずれも毎測定サイクルで再計算される。確定値は統合ストア側に入っているため保存不要。
`MLSS/SS/TR_ADC_mV` は §2-5 の通りポインタ経由で書かれる（正常）。

### 4-3. 調整・校正の作業領域（保存不要。確定時にストアへ入る）

`adj_x[48B]` / `adj_y[48B]`（Mode_CF 捕捉点）、`adj_n`（捕捉点数）、`adj_tc_ref` / `adj_tc_juko`（温度補正捕捉）、
`adj_tc_set`（5/20/35℃ 捕捉フラグ）、`adj_progress_cb`、
`cal_hsel_cursor` / `cal_hsel_purpose`、`cal_from_powerOn`、
`span_setting_target` / `span_setting_return` / `span_setting_chain_mid` / `span_setting_kind`、
`adb_busy_scr` / `adb_busy_val` / `adb_busy_digit` / `adb_busy_unit` / `adb_busy_frame`（ADBOAD 待機表示）、
`g_hmirror`（EEPROM ミラー RAM 像）。

`AWC` 等の確定操作でストアへ pack されるため、作業領域自体は揮発で正しい。

### 4-4. 安定判断（AutoStable）— 設計判断が要る

`data_stable[1020B]`（サンプルバッファ）`cnt_stable` `cnt_OverWrite` `chk_stable` `f_stable`
`Stagger` / `OldStagger`（ふらつき幅）`TimeStable` / `CalTimeStable` / `CalStagger`（所要時間）は
実行時状態＝揮発でよい。
一方 `stbwidthA` / `stbwidthB`（安定判断幅）`stbtime`（表示更新間隔）`stbsize`（サンプル数）は
**判定パラメータ**であり、FW 固定にするか保存して調整可能にするかの決めが要る
（handoff の T4「低値域フロア調整」は `stbwidthA/B` が対象）。
`Stbl_sel` は §2-4 の通り **書込 0 で分岐が死んでいる**。

### 4-5. プローブ起動・同期の実行時状態（揮発で正しい）

`probe_hs` / `probe_hs_t0`（起動ハンドシェイク）、`Probe_MS_started`、`probe_ms_on`、
`Probe_Store_BootResult` / `Probe_Store_L2Applied`（AMIR 観測用）、`Probe_cmd_pending`（ただし R=0 → §2-6）。

### 4-6. 電源・UI の実行時フラグ（揮発で正しい）

`power_off_flag` `wakeup_flag` `ena_pow` `Comm_PowerOff_flag` `wn_poff_flag`
`v33_on_flag`（`v33_on_cmd` / `v33_off_cmd` は R=0 → §2-6）、
`fl_flag` / `fl_flag2`（カーソル点滅）、`bar_flag`（電池アイコン）、`MEM_lp`、
`auto_adjust_flag`、`req_auto_stbl_flag`、`req_hst`、`Req_SetupData_Write`。

### 4-7. legacy 1Wire 由来で残っているもの（撤去候補）

`WAFG_Flag` `DAFG_Flag` `wire_test_flag` `WAG_Type` `WAG_RestDay`
`WAG_HstCal_Date` `WAG_HstCal_Type` `WAG_HstMeas_Date` `WAG_HstErr_Date` `WAG_HstErr_Type`
`WAG_HstCal_Time` `WAG_HstMeas_Time` `WAG_HstCal_Stable` `WAG_HstMeas_Stable`
`ZEROCAL_flag` `select_JIS` `RTC_ResetYear` `RTC_ResetOver` `RTC_ResetCount` `RTC_ResetDate`
`AutoCal_time_sel` `log_start_date` `log_sampling_gap` `log_interval_setting` `log_disp_flag`。

多くが §2-6 の「書込のみ」に該当する。§2-1 / §2-2 の処置とまとめて整理するのが妥当。

### 4-8. プローブ側 ★要判断 12 件

| 変数 | 宣言 | 判断 |
|---|---|---|
| `ad_ave_buf` (5100B) | `IM_110.c:25` | 移動平均バッファ。揮発で正しい |
| `ad_ave_idx` / `ad_ave_num` | `IM_110.c:26,27` | 同上 |
| `AD_AVE_mV` | `IM_110.c:21` | 移動平均値。揮発で正しい |
| `MCP3424_AD_Digit` / `MCP3424_AD_mV` | `IM_110.c:19,20` | 生 AD 値。揮発で正しい |
| `mcp3424_current_ch` | `IM_110.c:24` | 実行時状態。揮発で正しい |
| `HSPPAD143_Pressure_hPa` | `IM_110.c:30` | 気圧生値。揮発で正しい |
| `MS_flag` | `IM_110.c:16` | 測定開始状態。揮発で正しい |
| `ADC_Zero` | `IM_110.c:36` | 統合ストア未結線。ゼロ点を記録するが復元経路なし → 要決定 |
| `ADC_Span_S` | `IM_110.c:39` | 出荷時 ADC ゼロ点補正値。同上 → 要決定 |
| **`AD_AVE_Count`** | `IM_110.c:22` | Page2 に枠のみ・未結線（§3） |
| **`mcp3424_ch_mask`** | `IM_110.c:23` | Page2 に枠のみ・未結線（§3） |

---

## 5. DO / 1Wire 撤去で RAM から消えた変数 — 67 件（基底名）

前版は「54 個」としていたが**過少だった**。撤去前コミット `1d37c3f` を
`git archive` で取り出して同条件でビルドし直し、ELF シンボルを突き合わせた結果が下記。
関数内 static のマングル番号（`num.12` → `num.9` 等）の振り直しは差分から除外してある。

```bash
git archive 1d37c3f | tar -x -C <tmp> && cd <tmp> && make -j
arm-none-eabi-nm --defined-only -S -l build/IM-110.elf | grep -E '^20[0-9a-f]{6} '
# 撤去前 515 変数 (524 行 − リンカ記号 9) と 現行 448 変数を、マングル番号を落とした基底名で比較
```

**消えた 67 件**（増えた変数は 0 件）:

`AD` `AD20` `ADH` `ADzr` `AIR_PER` `Cnt_After1wire` `DO` `DOAcc` `DOH` `DO_Hold` `DO_buf` `DOsp` `DOzr`
`FCL` `FDOH` `FH` `FTH` `FTL` `PowerOn_Amp` `PowerOn_Temp` `R` `SETmid` `SETsp` `SETzr` `THsp` `THzr`
`TLsp` `TLzr` `TOffset` `TOsp` `TOzr` `TPmid` `TPsp` `TPzr` `WAG_DOsp` `WAG_DOzr` `WAG_HstCal_Amp`
`WAG_HstCal_Temp` `WAG_HstMeas_Amp` `WAG_HstMeas_Meas` `WAG_HstMeas_Temp` `WA_F` `WA_T` `WInit_Err`
`WInit_Retry` `WRead_Err` `WRead_Retry` `WTemp` `WWrite_Err` `WWrite_Retry` `Y0` `Y0mid` `Y0over` `Y1`
`Y1_` `do_flash_flag` `do_stop` `dspTemp` `dspdo` `empty_do` `empty_temp` `force_range`
`force_range_set_flag` `log_dat` `log_head` `now_range` `temp_flash_flag`

- うち **52 件はソースからも消えていた**（`Calc.c` / `Calc.h` / `Wire.c` / `Wire.h` / `WireSub.c` / `WireSub.h` は削除済み、
  Makefile からも除去済み）。
- 残り **15 件は宣言がソースに残っていた** → **本セッションで 13 件を削除**（§2-7 (a)(b)）。
  未削除は `log_dat` / `log_head` の 2 件で、これは DO ではなく**死んだロガー機能**に属する（§2-7 (c)）。
- `Y2` / `Y2_`（電池電圧、ADBOAD 3 で使用）は残置で正しい。

---

## 6. 本体 IM-110 — 全 361 件 一覧

判定分類順 → サイズ降順 → 名前順。`B` 欄の `-` は nm がサイズを出さないシンボル。
`W`=書込回数 / `R`=読出回数 / `&`=アドレス渡し回数（定義は §0-2）。

| # | 変数 | B | 宣言 | W | R | & | 用途 (宣言コメント / 参照関数) | 判定 |
|---|---|---|---|---|---|---|---|---|
| 1 | `MLSS_Mode_CF` | 840 | Core/Src/IM_110.c:114 | 9 | 9 | 0 | / | 保存済 統合ストア512B [書出+復元] |
| 2 | `SS_Mode_CF` | 840 | Core/Src/IM_110.c:151 | 9 | 7 | 0 | SS 相関式テーブル ([] と No. の対応は MLSS_Mode_CF と同じ規則) | 保存済 統合ストア512B [書出+復元] |
| 3 | `his` | 600 | Core/Inc/mainSub.h:158 | 13 | 38 | 4 | 測定履歴データ (現在選択中の種別バンクを保持) | 保存済 EEP page81-125(測定履歴) [書出+復元] |
| 4 | `g_hstore` | 512 | Core/Src/IM_110.c:551 | 2 | 12 | 18 | 本体側 統合ストア像 (RPG受信/WPG送信/ミラー) | 保存済 統合ストア512B [復元のみ] |
| 5 | `MLSS_Coef_ModeCF` | 280 | Core/Src/IM_110.c:262 | 4 | 5 | 0 | Mode_CF (吸光度→FABSS 多項式) No.21-30 | 保存済 統合ストア512B [書出+復元] |
| 6 | `SS_Coef_ModeCF` | 280 | Core/Src/IM_110.c:276 | 4 | 5 | 0 | W:(ファイルスコープ),store_unpack_to_globals,adj_copy_base_to_slots / R:store_pack_f… | 保存済 統合ストア512B [書出+復元] |
| 7 | `eep_wdata` | 32 | Core/Inc/Eeprom.h:153 | 56 | 48 | 0 | EEPROM書込ページデータ | 保存済 EEP page12(共通設定) + EEP page4(本体情報/エラー) + EEP page5(界面設定) [書出のみ] |
| 8 | `MLSS_Coef_TempC` | 28 | Core/Src/IM_110.c:302 | 3 | 5 | 0 | 温度補正 (Ref による受光補正) | 保存済 統合ストア512B [書出+復元] |
| 9 | `SS_Coef_TempC` | 28 | Core/Src/IM_110.c:303 | 3 | 6 | 0 | TR は SS を参照 (実体持たず) | 保存済 統合ストア512B [書出+復元] |
| 10 | `TR_Coef_ModeCF` | 28 | Core/Src/IM_110.c:298 | 6 | 6 | 0 | TR 累乗 a,b (校正後 = ストア Page14) | 保存済 統合ストア512B [書出+復元] |
| 11 | `TR_Coef_ModeCF_Ship` | 28 | Core/Src/IM_110.c:299 | 3 | 4 | 0 | TR 累乗 a,b (出荷時ベース = ストア Page13, §12 表) | 保存済 統合ストア512B [書出+復元] |
| 12 | `ADC_mV_ave` | 24 | Core/Src/IM_110.c:39 | 3 | 33 | 0 | 本体側 MAIN_AD_AVE_COUNT 件移動平均後の値 | 保存済 EEP page81-125(測定履歴) [書出のみ] |
| 13 | `MLSS_Coef_ADZR` | 8 | Core/Src/IM_110.c:300 | 2 | 5 | 0 | [0]=受光ADZR [1]=RefADZR (per-mode 単一ゼロ) | 保存済 統合ストア512B [書出+復元] |
| 14 | `SS_Coef_ADZR` | 8 | Core/Src/IM_110.c:301 | 2 | 8 | 0 | W:store_unpack_to_globals / R:store_pack_from_globals,SS_FABSS_from_raw_mv,T… | 保存済 統合ストア512B [書出+復元] |
| 15 | `Depth` | 4 | Core/Src/IM_110.c:35 | 14 | 44 | 0 | W:disp_MLSS_SCal_1,disp_MLSS_SCal_3,disp_MLSS_MCal_3 / R:adj_probe,disp_MLSS… | 保存済 EEP page81-125(測定履歴) [書出のみ] |
| 16 | `Depth_k` | 4 | Core/Src/IM_110.c:1354 | 3 | 4 | 0 | ADBOAD/UART の 6m 校正 (Depth_Calib_Span6m) で上書き。1点スロープ校正 (mlss-calc-reference … | 保存済 統合ストア512B [書出+復元] |
| 17 | `Interface_Threshold` | 4 | Core/Src/IM_110.c:36 | 4 | 9 | 0 | 界面検知しきい値 | 保存済 EEP page5(界面設定) [書出+復元] |
| 18 | `MLSS` | 4 | Core/Src/IM_110.c:31 | 25 | 86 | 0 | W:disp_MLSS_SCal_1,disp_MLSS_SCal_2,disp_MLSS_SCal_3 / R:adj_probe,check_sta… | 保存済 EEP page81-125(測定履歴) [書出のみ] |
| 19 | `MLSS_Cal_SetVal_1` | 4 | Core/Src/IM_110.c:104 | 7 | 10 | 0 | MLSS 校正設定値1 (2点校正時のスパン濃度) | 保存済 EEP page12(共通設定) + 統合ストア512B [書出+復元] |
| 20 | `MLSS_Cal_SetVal_2` | 4 | Core/Src/IM_110.c:105 | 9 | 8 | 0 | MLSS 校正設定値2 (3点校正時の中間濃度) | 保存済 EEP page12(共通設定) + 統合ストア512B [書出+復元] |
| 21 | `MLSS_SP_A` | 4 | Core/Src/IM_110.c:98 | 4 | 5 | 1 | MLSS スパン校正係数A (y=Ax^2+Bx+C) | 保存済 統合ストア512B [書出+復元] |
| 22 | `MLSS_SP_B` | 4 | Core/Src/IM_110.c:99 | 4 | 6 | 1 | MLSS スパン校正係数B | 保存済 統合ストア512B [書出+復元] |
| 23 | `MLSS_SP_C` | 4 | Core/Src/IM_110.c:100 | 4 | 5 | 1 | MLSS スパン校正係数C | 保存済 統合ストア512B [書出+復元] |
| 24 | `MLSS_ZR` | 4 | Core/Src/IM_110.c:91 | 7 | 5 | 0 | MLSSゼロ校正係数 (mV) = 現場枠 zr_field | 保存済 統合ストア512B [書出+復元] |
| 25 | `MLSS_ZR_Ship` | 4 | Core/Src/IM_110.c:94 | 3 | 3 | 0 | W:(ファイルスコープ),store_unpack_to_globals,Adj_CaptureZR_Ship / R:store_pack_from_… | 保存済 統合ストア512B [書出+復元] |
| 26 | `SS` | 4 | Core/Src/IM_110.c:32 | 15 | 54 | 0 | W:disp_MLSS_SCal_1,disp_MLSS_SCal_2,disp_MLSS_SCal_3 / R:adj_probe,check_sta… | 保存済 EEP page81-125(測定履歴) [書出のみ] |
| 27 | `SS_Cal_SetVal_1` | 4 | Core/Src/IM_110.c:200 | 7 | 8 | 0 | SS 校正設定値1 (2点校正時のスパン濃度) | 保存済 EEP page12(共通設定) + 統合ストア512B [書出+復元] |
| 28 | `SS_Cal_SetVal_2` | 4 | Core/Src/IM_110.c:201 | 8 | 7 | 0 | SS 校正設定値2 (3点校正時の中間濃度) | 保存済 EEP page12(共通設定) + 統合ストア512B [書出+復元] |
| 29 | `SS_SP_A` | 4 | Core/Src/IM_110.c:194 | 4 | 4 | 1 | SS スパン校正係数A (y=Ax^2+Bx+C) | 保存済 統合ストア512B [書出+復元] |
| 30 | `SS_SP_B` | 4 | Core/Src/IM_110.c:195 | 4 | 4 | 1 | SS スパン校正係数B | 保存済 統合ストア512B [書出+復元] |
| 31 | `SS_SP_C` | 4 | Core/Src/IM_110.c:196 | 4 | 4 | 1 | SS スパン校正係数C | 保存済 統合ストア512B [書出+復元] |
| 32 | `SS_ZR` | 4 | Core/Src/IM_110.c:189 | 7 | 4 | 0 | SSゼロ校正係数 (mV) = 現場枠 zr_field | 保存済 統合ストア512B [書出+復元] |
| 33 | `SS_ZR_Ship` | 4 | Core/Src/IM_110.c:190 | 3 | 3 | 0 | 出荷時枠 zr_ship (§191/§193)。MLSS_ZR_Ship と同じ役割 | 保存済 統合ストア512B [書出+復元] |
| 34 | `TR_Cal_SetVal_1` | 4 | Core/Src/IM_110.c:253 | 7 | 8 | 0 | TR 校正設定値1 (2点校正時のスパン濃度) | 保存済 EEP page12(共通設定) + 統合ストア512B [書出+復元] |
| 35 | `TR_Cal_SetVal_2` | 4 | Core/Src/IM_110.c:254 | 8 | 7 | 0 | TR 校正設定値2 (3点校正時の中間濃度) | 保存済 EEP page12(共通設定) + 統合ストア512B [書出+復元] |
| 36 | `TR_SP_A` | 4 | Core/Src/IM_110.c:247 | 4 | 4 | 1 | TR スパン校正係数A (y=Ax^2+Bx+C) | 保存済 統合ストア512B [書出+復元] |
| 37 | `TR_SP_B` | 4 | Core/Src/IM_110.c:248 | 4 | 4 | 1 | TR スパン校正係数B | 保存済 統合ストア512B [書出+復元] |
| 38 | `TR_SP_C` | 4 | Core/Src/IM_110.c:249 | 4 | 4 | 1 | TR スパン校正係数C | 保存済 統合ストア512B [書出+復元] |
| 39 | `TR_ZR` | 4 | Core/Src/IM_110.c:243 | 6 | 3 | 0 | TRゼロ校正係数 (mV) | 保存済 統合ストア512B [書出+復元] |
| 40 | `Transparency` | 4 | Core/Src/IM_110.c:34 | 15 | 54 | 0 | W:disp_MLSS_SCal_1,disp_MLSS_SCal_2,disp_MLSS_SCal_3 / R:adj_probe,check_sta… | 保存済 EEP page81-125(測定履歴) [書出のみ] |
| 41 | `ErrDate` | 3 | Core/Inc/mainSub.h:74 | 12 | 6 | 0 | 最新エラー発生日時 | 保存済 EEP page4(本体情報/エラー) [書出+復元] |
| 42 | `LastDate` | 3 | Core/Inc/mainSub.h:56 | 9 | 9 | 0 | 最新電源ON日 | 保存済 EEP page4(本体情報/エラー) [書出+復元] |
| 43 | `EEP_Read_Err` | 2 | Core/Inc/mainSub.h:83 | 6 | 5 | 0 | EEPROM読込エラー回数 (総計) | 保存済 EEP page4(本体情報/エラー) [書出+復元] |
| 44 | `EEP_Read_Retry` | 2 | Core/Inc/mainSub.h:82 | 6 | 5 | 0 | EEPROM読込リトライ回数 (総計) | 保存済 EEP page4(本体情報/エラー) [書出+復元] |
| 45 | `EEP_Write_Err` | 2 | Core/Inc/mainSub.h:85 | 6 | 5 | 0 | EEPROM書込エラー回数 (総計) | 保存済 EEP page4(本体情報/エラー) [書出+復元] |
| 46 | `EEP_Write_Retry` | 2 | Core/Inc/mainSub.h:84 | 6 | 5 | 0 | EEPROM書込リトライ回数 (総計) | 保存済 EEP page4(本体情報/エラー) [書出+復元] |
| 47 | `RTC_Read_Err` | 2 | Core/Inc/mainSub.h:65 | 6 | 5 | 0 | RTC読込エラー回数 (総計) | 保存済 EEP page4(本体情報/エラー) [書出+復元] |
| 48 | `RTC_Read_Retry` | 2 | Core/Inc/mainSub.h:64 | 6 | 5 | 0 | RTC読込リトライ回数 (総計) | 保存済 EEP page4(本体情報/エラー) [書出+復元] |
| 49 | `RTC_TSet` | 2 | Core/Inc/mainSub.h:61 | 3 | 2 | 0 | RTC時計設定回数（総計） | 保存済 EEP page4(本体情報/エラー) [書出+復元] |
| 50 | `RTC_Write_Err` | 2 | Core/Inc/mainSub.h:67 | 6 | 5 | 0 | RTC書込エラー回数 (総計) | 保存済 EEP page4(本体情報/エラー) [書出+復元] |
| 51 | `RTC_Write_Retry` | 2 | Core/Inc/mainSub.h:66 | 6 | 5 | 0 | RTC書込リトライ回数 (総計) | 保存済 EEP page4(本体情報/エラー) [書出+復元] |
| 52 | `year` | 2 | Core/Inc/mainSub.h:123 | 8 | 28 | 0 | RTC 年 | 保存済 EEP page81-125(測定履歴) [書出+復元] |
| 53 | `day` | 1 | Core/Inc/mainSub.h:120 | 14 | 32 | 0 | RTC 日 | 保存済 EEP page81-125(測定履歴) [書出+復元] |
| 54 | `EEP_Info_flag` | 1 | Core/Inc/mainSub.h:60 | 2 | 1 | 0 | EEPROMの寿命検知パラメーター強制使用フラグ | 保存済 EEP page4(本体情報/エラー) [書出+復元] |
| 55 | `ErrNo` | 1 | Core/Inc/mainSub.h:75 | 4 | 3 | 0 | 最新エラーNo. | 保存済 EEP page4(本体情報/エラー) [書出+復元] |
| 56 | `hour` | 1 | Core/Inc/mainSub.h:119 | 20 | 30 | 0 | RTC 時 | 保存済 EEP page81-125(測定履歴) [書出+復元] |
| 57 | `Meas_Mode` | 1 | Core/Src/IM_110.c:30 | 6 | 55 | 0 | Measurement data variables | 保存済 EEP page12(共通設定) + EEP page81-125(測定履歴) [書出+復元] |
| 58 | `min` | 1 | Core/Inc/mainSub.h:118 | 21 | 30 | 0 | RTC 分 | 保存済 EEP page81-125(測定履歴) [書出+復元] |
| 59 | `MLSS_MODE` | 1 | Core/Src/IM_110.c:90 | 7 | 27 | 0 | MLSSモード | 保存済 EEP page12(共通設定) [書出+復元] |
| 60 | `month` | 1 | Core/Inc/mainSub.h:121 | 12 | 37 | 0 | RTC 月 | 保存済 EEP page81-125(測定履歴) [書出+復元] |
| 61 | `SS_MODE` | 1 | Core/Src/IM_110.c:188 | 7 | 22 | 0 | SSモード | 保存済 EEP page12(共通設定) [書出+復元] |
| 62 | `TR_MODE` | 1 | Core/Src/IM_110.c:242 | 7 | 20 | 0 | TRモード | 保存済 EEP page12(共通設定) [書出+復元] |
| 63 | `generator` | 899 | qrencode/rsecc.c:54 | 1 | 1 | 0 | W:generator_init / R:RSECC_encode | ★要判断 |
| 64 | `aindex` | 256 | qrencode/rsecc.c:53 | 2 | 4 | 0 | W:RSECC_initLookupTable / R:generator_init,RSECC_encode | ★要判断 |
| 65 | `alpha` | 256 | qrencode/rsecc.c:52 | 2 | 4 | 0 | W:RSECC_initLookupTable / R:generator_init,RSECC_encode | ★要判断 |
| 66 | `ad_ave_buf` | 240 | Core/Src/IM_110.c:42 | 2 | 1 | 0 | 本体側 ADC 移動平均バッファ (旧 Probe AD_AVE_Calc と同等のロジック、件数は MAIN_AD_AVE_COUNT) | ★要判断 |
| 67 | `adj_x` | 48 | Core/Src/IM_110.c:1643 | 1 | 7 | 0 | Mode_CF 捕捉: x = ABS (対数中間値) | ★要判断 |
| 68 | `adj_y` | 48 | Core/Src/IM_110.c:1644 | 1 | 7 | 0 | Mode_CF 捕捉: y = 基準器 mg/L (透視度は cm) | ★要判断 |
| 69 | `eep_rdata` | 32 | Core/Inc/Eeprom.h:152 | 1 | 44 | 0 | EEPROM読込ページデータ | ★要判断 |
| 70 | `generatorInitialized` | 29 | qrencode/rsecc.c:55 | 2 | 1 | 0 | W:RSECC_init,generator_init / R:RSECC_encode | ★要判断 |
| 71 | `ADC_mV` | 24 | Core/Src/IM_110.c:38 | 1 | 17 | 0 | Probe 受信値 (瞬時、SADA,0 で平均 OFF 前提) | ★要判断 |
| 72 | `adj_tc_juko` | 12 | Core/Src/IM_110.c:1648 | 1 | 3 | 0 | 温度補正 捕捉 受光 | ★要判断 |
| 73 | `adj_tc_ref` | 12 | Core/Src/IM_110.c:1647 | 1 | 3 | 0 | 温度補正 捕捉 Ref  (0=5℃ 1=20℃ 2=35℃) | ★要判断 |
| 74 | `ad_ave_idx` | 6 | Core/Src/IM_110.c:43 | 3 | 2 | 0 | W:update_main_ave / R:update_main_ave | ★要判断 |
| 75 | `adb_busy_val` | 4 | Core/Src/Adjust.c:910 | 1 | 1 | 0 | 表示中の値 | ★要判断 |
| 76 | `Adb_Ref_MLSS2` | 4 | Core/Src/IM_110.c:291 | 1 | 3 | 0 | 09-11 MLSS 出荷時3点 2点目 | ★要判断 |
| 77 | `Adb_Ref_MLSS3` | 4 | Core/Src/IM_110.c:292 | 1 | 3 | 0 | 3点目 | ★要判断 |
| 78 | `Adb_Ref_SS2` | 4 | Core/Src/IM_110.c:293 | 1 | 3 | 0 | 12-14 SS   出荷時3点 2点目 | ★要判断 |
| 79 | `Adb_Ref_SS3` | 4 | Core/Src/IM_110.c:294 | 1 | 3 | 0 | 3点目 | ★要判断 |
| 80 | `Adb_Ref_TR2` | 4 | Core/Src/IM_110.c:295 | 1 | 3 | 0 | 15-17 透視度 出荷時3点 2点目 [cm] | ★要判断 |
| 81 | `Adb_Ref_TR3` | 4 | Core/Src/IM_110.c:296 | 1 | 3 | 0 | 3点目 [cm] | ★要判断 |
| 82 | `adj_n` | 4 | Core/Src/IM_110.c:1645 | 4 | 11 | 0 | Mode_CF 捕捉点数 | ★要判断 |
| 83 | `adj_progress_cb` | 4 | Core/Src/IM_110.c:1788 | 1 | 3 | 0 | W:Adj_SetProgressCallback / R:(ファイルスコープ),adj_delay_progress | ★要判断 |
| 84 | `CalStagger` | 4 | Core/Inc/AutoStable.h:18 | 2 | 0 | 0 | 校正案定時のふらつき幅 | ★要判断 |
| 85 | `Depth_offset` | 4 | Core/Src/IM_110.c:1350 | 3 | 2 | 0 | Depth_Calib_Zero() で任意のタイミングに現在 ADC_mV[5] を新基準に上書き可能。 | ★要判断 |
| 86 | `f_ad` | 4 | Core/Inc/Adc.h:28 | 2 | 2 | 0 | 移動平均計算用 | ★要判断 |
| 87 | `f_ad_data` | 4 | Core/Inc/Adc.h:29 | 1 | 1 | 0 | 移動平均計算用 | ★要判断 |
| 88 | `initialized` | 4 | qrencode/rsecc.c:41 | 2 | 1 | 0 | W:(ファイルスコープ),RSECC_init / R:RSECC_encode | ★要判断 |
| 89 | `Interface_Hold` | 4 | Core/Src/IM_110.c:37 | 2 | 2 | 0 | 界面深度ホールド (初期値は FLT_MAX = 未捕捉、表示側で flash 白塗りされる) | ★要判断 |
| 90 | `MLSS_1A` | 4 | Core/Src/IM_110.c:101 | 3 | 6 | 0 | MLSS FABSSフルスパン校正値 | ★要判断 |
| 91 | `MLSS_1B` | 4 | Core/Src/IM_110.c:102 | 4 | 2 | 0 | MLSS FABSS 1/2スパン校正値 | ★要判断 |
| 92 | `MLSS_CAL_REF` | 4 | Core/Src/IM_110.c:103 | 2 | 2 | 0 | MLSS スパン校正基準値 (設定濃度) | ★要判断 |
| 93 | `MLSS_FABSS` | 4 | Core/Src/IM_110.c:97 | 1 | 4 | 0 | MLSS モード変換後の値 | ★要判断 |
| 94 | `MLSS_Hold` | 4 | Core/Src/IM_110.c:83 | 3 | 0 | 1 | Hold values (stable) | ★要判断 |
| 95 | `MLSS_inst` | 4 | Core/Src/IM_110.c:33 | 1 | 2 | 0 | 移動平均無しの瞬時 MLSS (界面判断バー用) | ★要判断 |
| 96 | `OldStagger` | 4 | Core/Inc/AutoStable.h:17 | 4 | 1 | 0 | 安定時，安定直前のふらつき幅 | ★要判断 |
| 97 | `probe_hs_t0` | 4 | Core/Src/IM_110.c:988 | 3 | 2 | 0 | W:(ファイルスコープ),Probe_Boot_Start,Probe_Boot_Tick / R:Probe_Boot_Tick | ★要判断 |
| 98 | `span_setting_target` | 4 | Core/Src/Normal.c:75 | 5 | 12 | 0 | W:(ファイルスコープ),nrm_span_setting_begin,nrm_span_setting / R:nrm_span_setting | ★要判断 |
| 99 | `SS_1A` | 4 | Core/Src/IM_110.c:197 | 2 | 6 | 0 | SS FABSSフルスパン校正値 | ★要判断 |
| 100 | `SS_1B` | 4 | Core/Src/IM_110.c:198 | 3 | 2 | 0 | SS FABSS 1/2スパン校正値 | ★要判断 |
| 101 | `SS_CAL_REF` | 4 | Core/Src/IM_110.c:199 | 1 | 2 | 0 | SS スパン校正基準値 (設定濃度) | ★要判断 |
| 102 | `SS_FABSS` | 4 | Core/Src/IM_110.c:193 | 1 | 3 | 0 | SS モード変換後の値 | ★要判断 |
| 103 | `SS_Hold` | 4 | Core/Src/IM_110.c:84 | 2 | 0 | 1 | W:Calib_StoreHoldMeasured_Current,Calib_StoreHoldCalibrated_Current / R:- | ★要判断 |
| 104 | `Stagger` | 4 | Core/Inc/AutoStable.h:17 | 2 | 1 | 0 | 安定時，安定直前のふらつき幅 | ★要判断 |
| 105 | `stbsize` | 4 | Core/Src/AutoStable.c:27 | 6 | 15 | 0 | 安定判断サンプル数 | ★要判断 |
| 106 | `stbwidthA` | 4 | Core/Src/AutoStable.c:28 | 4 | 2 | 0 | 安定判断値 | ★要判断 |
| 107 | `stbwidthB` | 4 | Core/Src/AutoStable.c:28 | 4 | 3 | 0 | 安定判断値 | ★要判断 |
| 108 | `TR_1A` | 4 | Core/Src/IM_110.c:250 | 2 | 6 | 0 | TR FABSSフルスパン校正値 | ★要判断 |
| 109 | `TR_1B` | 4 | Core/Src/IM_110.c:251 | 3 | 2 | 0 | TR FABSS 1/2スパン校正値 | ★要判断 |
| 110 | `TR_CAL_REF` | 4 | Core/Src/IM_110.c:252 | 1 | 2 | 0 | TR スパン校正基準値 (設定濃度) | ★要判断 |
| 111 | `TR_FABSS` | 4 | Core/Src/IM_110.c:246 | 1 | 3 | 0 | TR モード変換後の値 | ★要判断 |
| 112 | `TR_Hold` | 4 | Core/Src/IM_110.c:85 | 2 | 0 | 1 | W:Calib_StoreHoldMeasured_Current,Calib_StoreHoldCalibrated_Current / R:- | ★要判断 |
| 113 | `Y2` | 4 | Core/Inc/Adc.h:25 | 1 | 6 | 0 | アナログ電圧値mV Y2:電池 (Y0:水温/Y1 は撤去で削除) | ★要判断 |
| 114 | `RTC_ResetDate` | 3 | Core/Inc/mainSub.h:115 | 7 | 10 | 0 | RTCチェック用の製造日 | ★要判断 |
| 115 | `cal_hsel_cursor` | 2 | Core/Src/Normal.c:65 | 5 | 4 | 0 | 選択中の履歴番号 (0..count-1) | ★要判断 |
| 116 | `span_setting_return` | 2 | Core/Src/Normal.c:70 | 6 | 2 | 0 | スパン校正値設定 (C_S_SET) から戻る先と、3点時に中間濃度設定へ連鎖するか | ★要判断 |
| 117 | `adb_busy_digit` | 1 | Core/Src/Adjust.c:911 | 1 | 1 | 0 | W:adj_probe / R:adj_probe_progress | ★要判断 |
| 118 | `adb_busy_frame` | 1 | Core/Src/Adjust.c:913 | 2 | 1 | 0 | 0/1 トグル | ★要判断 |
| 119 | `adb_busy_scr` | 1 | Core/Src/Adjust.c:909 | 1 | 1 | 0 | 表示中の画面番号 | ★要判断 |
| 120 | `adb_busy_unit` | 1 | Core/Src/Adjust.c:912 | 1 | 1 | 0 | W:adj_probe / R:adj_probe_progress | ★要判断 |
| 121 | `adj_tc_set` | 1 | Core/Src/IM_110.c:1649 | 4 | 2 | 0 | 温度補正 捕捉フラグ (bit0/1/2 = 5/20/35℃) | ★要判断 |
| 122 | `auto_adjust_flag` | 1 | Core/Inc/mainSub.h:196 | 15 | 1 | 0 | 基板自動調整フラグ	2019/12/17追加　三浦 | ★要判断 |
| 123 | `bar_flag` | 1 | Core/Inc/Display.h:36 | 7 | 70 | 0 | 電池アイコン表示用 | ★要判断 |
| 124 | `cal_from_powerOn` | 1 | Core/Inc/Normal.h:21 | 1 | 4 | 0 | 電源 ON 時 MEM+POW で校正モード起動した場合 1。校正メニューに「ゼロ校正」を含めた 4 ボタン配置に切替え、現校正モードをカーソル初期位置… | ★要判断 |
| 125 | `cal_hsel_purpose` | 1 | Core/Src/Normal.c:64 | 6 | 3 | 0 | cal_hsel_purpose: 0=2点フルスパン点 / 1=3点中間点 / 2=3点フルスパン点 | ★要判断 |
| 126 | `Comm_PowerOff_flag` | 1 | Core/Inc/mainSub.h:197 | 2 | 1 | 0 | 通信コマンドでの電源ＯＦＦ要求フラグ	2019/12/17追加　三浦 | ★要判断 |
| 127 | `Depth_offset_set` | 1 | Core/Src/IM_110.c:1351 | 3 | 1 | 0 | W:(ファイルスコープ),Calc_Depth,Depth_Calib_Zero / R:Calc_Depth | ★要判断 |
| 128 | `ena_pow` | 1 | Core/Src/mainSub.c:193 | 2 | 1 | 0 | Powerボタン有効フラグ | ★要判断 |
| 129 | `fl_flag` | 1 | Core/Inc/Setting.h:21 | 57 | 28 | 0 | 選択カーソル点滅フラグ（設定画面用） | ★要判断 |
| 130 | `fl_flag2` | 1 | Core/Inc/Adjust.h:20 | 40 | 9 | 0 | 選択カーソル点滅フラグ（調整画面用） | ★要判断 |
| 131 | `MEM_lp` | 1 | Core/Inc/Setting.h:22 | 3 | 1 | 0 | MEM長押し中フラグ | ★要判断 |
| 132 | `power_off_flag` | 1 | Core/Inc/mainSub.h:192 | 2 | 1 | 0 | 電源OFFフラグ | ★要判断 |
| 133 | `Probe_cmd_pending` | 1 | Core/Src/IM_110.c:310 | 2 | 0 | 0 | W:(ファイルスコープ),Probe_uart_init / R:- | ★要判断 |
| 134 | `probe_hs` | 1 | Core/Src/IM_110.c:987 | 10 | 2 | 0 | 未起動時は DONE 扱い | ★要判断 |
| 135 | `probe_ms_on` | 1 | Core/Src/IM_110.c:321 | 5 | 1 | 0 | 0=OFF(電源ONベースライン) / 1=ON | ★要判断 |
| 136 | `Probe_MS_started` | 1 | Core/Src/IM_110.c:312 | 4 | 1 | 0 | W:(ファイルスコープ),Probe_Request_MD,Probe_ResumeMS / R:Probe_Request_MD | ★要判断 |
| 137 | `Probe_Store_BootResult` | 1 | Core/Src/IM_110.c:728 | 6 | 1 | 0 | 起動時3層ロードの結果 (AMIR で観測)。0xFF=未実行 | ★要判断 |
| 138 | `Probe_Store_L2Applied` | 1 | Core/Src/IM_110.c:729 | 2 | 1 | 0 | 起動時に層2(ミラー)を live へ適用したか (AMIR で観測) | ★要判断 |
| 139 | `req_auto_stbl_flag` | 1 | Core/Inc/AutoStable.h:21 | 6 | 5 | 0 | 判定要求フラグ | ★要判断 |
| 140 | `req_hst` | 1 | Core/Inc/mainSub.h:58 | 2 | 0 | 0 | 測定履歴の記憶許可フラグ | ★要判断 |
| 141 | `Req_SetupData_Write` | 1 | Core/Inc/mainSub.h:59 | 14 | 1 | 0 | 設定情報の記憶要求フラグ（電源OFF時記憶） | ★要判断 |
| 142 | `RTC_ResetOver` | 1 | Core/Inc/mainSub.h:114 | 1 | 0 | 0 | RTCリセット回数ｵｰﾊﾞｰ有無フラグ | ★要判断 |
| 143 | `RTC_ResetYear` | 1 | Core/Inc/mainSub.h:113 | 1 | 0 | 0 | RTCチェック用の年カウント | ★要判断 |
| 144 | `sec` | 1 | Core/Inc/mainSub.h:117 | 2 | 4 | 0 | RTC 秒 | ★要判断 |
| 145 | `select_JIS` | 1 | Core/Inc/mainSub.h:48 | 1 | 0 | 0 | 新旧JIS選択 (0/1: 旧JIS/新JIS) | ★要判断 |
| 146 | `span_setting_chain_mid` | 1 | Core/Src/Normal.c:71 | 6 | 1 | 0 | W:(ファイルスコープ),nrm_span_setting_begin,nrm_adjust_span_m / R:nrm_span_setting | ★要判断 |
| 147 | `span_setting_kind` | 1 | Core/Src/Normal.c:76 | 2 | 2 | 0 | W:(ファイルスコープ),nrm_span_setting_begin / R:nrm_span_setting | ★要判断 |
| 148 | `stbtime` | 1 | Core/Src/AutoStable.c:29 | 3 | 1 | 0 | 安定中の表示更新間隔 | ★要判断 |
| 149 | `v33_off_cmd` | 1 | Core/Inc/mainSub.h:211 | 1 | 0 | 0 | 3.3V電源OFF処理フラグ | ★要判断 |
| 150 | `v33_on_cmd` | 1 | Core/Inc/mainSub.h:210 | 1 | 0 | 0 | 3.3V電源ON処理フラグ | ★要判断 |
| 151 | `v33_on_flag` | 1 | Core/Inc/mainSub.h:209 | 3 | 2 | 0 | 3.3V電源 ONフラグ | ★要判断 |
| 152 | `wakeup_flag` | 1 | Core/Inc/mainSub.h:214 | 1 | 0 | 0 | 測定前準備フラグ | ★要判断 |
| 153 | `wn_poff_flag` | 1 | Core/Inc/mainSub.h:215 | 1 | 0 | 0 | WN受信時の定期監視フラグ 0:定期監視外 1:定期監視中 | ★要判断 |
| 154 | `ZEROCAL_flag` | 1 | Core/Inc/mainSub.h:162 | 1 | 0 | 0 | ゼロ校正係数更新フラグ | ★要判断 |
| 155 | `MLSS_ABSS` | 4 | Core/Src/IM_110.c:96 | 0 | 0 | 1 | MLSS 対数中間値 | ★ポインタ出力のみ・読出0 (デッドストア) |
| 156 | `SS_ABSS` | 4 | Core/Src/IM_110.c:192 | 0 | 0 | 1 | SS 対数中間値 | ★ポインタ出力のみ・読出0 (デッドストア) |
| 157 | `TR_ABSS` | 4 | Core/Src/IM_110.c:245 | 0 | 0 | 1 | TR 対数中間値 | ★ポインタ出力のみ・読出0 (デッドストア) |
| 158 | `g_hmirror` | 512 | Core/Src/IM_110.c:726 | 0 | 7 | 11 | 調停用ミラー像 (g_hstore=プローブ像 と別に保持) | ポインタ経由で書込 (&渡し) 揮発で正しい |
| 159 | `MLSS_ADC_mV` | 4 | Core/Src/IM_110.c:95 | 0 | 1 | 1 | MLSS A/D補正後の値 (mV) | ポインタ経由で書込 (&渡し) 揮発で正しい |
| 160 | `SS_ADC_mV` | 4 | Core/Src/IM_110.c:191 | 0 | 1 | 1 | SS A/D補正後の値 (mV) | ポインタ経由で書込 (&渡し) 揮発で正しい |
| 161 | `TR_ADC_mV` | 4 | Core/Src/IM_110.c:244 | 0 | 1 | 1 | TR A/D補正後の値 (mV) | ポインタ経由で書込 (&渡し) 揮発で正しい |
| 162 | `Probe_RxData` | 1 | Core/Src/IM_110.c:308 | 0 | 1 | 3 | W:- / R:Probe_RxCallback | ポインタ経由で書込 (&渡し) 揮発で正しい |
| 163 | `RxData` | 1 | Core/Inc/LinkSerial.h:19 | 0 | 8 | 5 | W:- / R:uart_init,HAL_UART_RxCpltCallback,HAL_UART_ErrorCallback | ポインタ経由で書込 (&渡し) 揮発で正しい |
| 164 | `adc_data` | 2 | Core/Inc/Adc.h:27 | 0 | 2 | 0 | A/Dデータ格納用 0:電池 | FW固定テーブル/読出専用 書込0 → 保存不要 |
| 165 | `Cal_Type` | 1 | Core/Src/IM_110.c:88 | 0 | 1 | 0 | 校正種別(0:ゼロ, 1:2pスパン, 2:3p中間) | FW固定テーブル/読出専用 書込0 → 保存不要 |
| 166 | `SYS_ErrorNo` | 4 | Core/Inc/mainSub.h:57 | 7 | 4 | 0 | IC操作エラー番号 | エラーカウンタ 揮発 or page4集約 |
| 167 | `timer_errdisp` | 4 | Core/Inc/Timer.h:59 | 2 | 2 | 3 | エラー自動解除タイマー | エラーカウンタ 揮発 or page4集約 |
| 168 | `EComm_Err` | 2 | Core/Inc/mainSub.h:87 | 2 | 2 | 0 | EEPROM通信初期化エラー回数 (電源OFFまでの累計) | エラーカウンタ 揮発 or page4集約 |
| 169 | `EEP_Comm_Err` | 2 | Core/Inc/mainSub.h:81 | 4 | 4 | 0 | EEPROM通信NG エラー回数 (総計) | エラーカウンタ 揮発 or page4集約 |
| 170 | `ERead_Err` | 2 | Core/Inc/mainSub.h:89 | 6 | 2 | 0 | EEPROM読込エラー回数 (電源OFFまでの累計) | エラーカウンタ 揮発 or page4集約 |
| 171 | `EWrite_Err` | 2 | Core/Inc/mainSub.h:91 | 14 | 2 | 0 | EEPROM書込エラー回数 (電源OFFまでの累計) | エラーカウンタ 揮発 or page4集約 |
| 172 | `RComm_Err` | 2 | Core/Inc/mainSub.h:69 | 2 | 2 | 0 | RTC通信初期化エラー回数 (電源OFFまでの累計) | エラーカウンタ 揮発 or page4集約 |
| 173 | `RRead_Err` | 2 | Core/Inc/mainSub.h:71 | 2 | 2 | 0 | RTC読込エラー回数 (電源OFFまでの累計) | エラーカウンタ 揮発 or page4集約 |
| 174 | `RTC_Comm_Err` | 2 | Core/Inc/mainSub.h:63 | 4 | 4 | 0 | RTC通信NG エラー回数 (総計) | エラーカウンタ 揮発 or page4集約 |
| 175 | `RWrite_Err` | 2 | Core/Inc/mainSub.h:73 | 2 | 2 | 0 | RTC書込エラー回数 (電源OFFまでの累計) | エラーカウンタ 揮発 or page4集約 |
| 176 | `ERROR_CLOCK_PON_Flag` | 1 | Core/Inc/mainSub.h:127 | 1 | 1 | 0 | PONエラー判別 | エラーカウンタ 揮発 or page4集約 |
| 177 | `ERROR_CLOCK_PON_Flag_sub` | 1 | Core/Inc/mainSub.h:130 | 1 | 1 | 0 | PONエラー判別 | エラーカウンタ 揮発 or page4集約 |
| 178 | `ERROR_CLOCK_VDET_Flag` | 1 | Core/Inc/mainSub.h:125 | 1 | 1 | 0 | VDETエラー判別 | エラーカウンタ 揮発 or page4集約 |
| 179 | `ERROR_CLOCK_VDET_Flag_sub` | 1 | Core/Inc/mainSub.h:128 | 1 | 1 | 0 | VDETエラー判別 | エラーカウンタ 揮発 or page4集約 |
| 180 | `ERROR_CLOCK_XST_Flag` | 1 | Core/Inc/mainSub.h:126 | 1 | 1 | 0 | XSTエラー判別 | エラーカウンタ 揮発 or page4集約 |
| 181 | `ERROR_CLOCK_XST_Flag_sub` | 1 | Core/Inc/mainSub.h:129 | 1 | 1 | 0 | XSTエラー判別 | エラーカウンタ 揮発 or page4集約 |
| 182 | `ERROR_REG` | 1 | Core/Inc/mainSub.h:131 | 1 | 1 | 0 | RTCエラーフラグ | エラーカウンタ 揮発 or page4集約 |
| 183 | `f_IC_err` | 1 | Core/Inc/mainSub.h:55 | 23 | 2 | 0 | ICのエラーフラグ | エラーカウンタ 揮発 or page4集約 |
| 184 | `oc_err_num` | 1 | Core/Inc/Normal.h:51 | 21 | 2 | 0 | 発生エラー番号 | エラーカウンタ 揮発 or page4集約 |
| 185 | `ad_ave_num` | 6 | Core/Src/IM_110.c:44 | 1 | 3 | 0 | バッファに格納済みのサンプル数 (0..MAIN_AD_AVE_COUNT) | タイマ/カウンタ 揮発で正しい |
| 186 | `adc_dis_timer` | 4 | Core/Inc/Timer.h:45 | 2 | 2 | 0 | レンジ切替後A/DC変換禁止タイマー | タイマ/カウンタ 揮発で正しい |
| 187 | `auto_power_off_timer` | 4 | Core/Inc/Timer.h:60 | 2 | 2 | 4 | オートパワーオフタイマー | タイマ/カウンタ 揮発で正しい |
| 188 | `batt_timer` | 4 | Core/Inc/Timer.h:51 | 2 | 2 | 3 | 電池電圧取得タイマー | タイマ/カウンタ 揮発で正しい |
| 189 | `cal_timer` | 4 | Core/Inc/Timer.h:47 | 2 | 6 | 2 | 校正用タイマー（センサー不安定） | タイマ/カウンタ 揮発で正しい |
| 190 | `check_wag_timer` | 4 | Core/Inc/Timer.h:80 | 2 | 1 | 0 | センサー挿抜識別タイマー | タイマ/カウンタ 揮発で正しい |
| 191 | `cnt_stable` | 4 | Core/Src/AutoStable.c:31 | 3 | 9 | 0 | 安定判断のデータ位置 | タイマ/カウンタ 揮発で正しい |
| 192 | `disp_hold_timer` | 4 | Core/Inc/Timer.h:61 | 2 | 1 | 0 | ホールド表示タイマー | タイマ/カウンタ 揮発で正しい |
| 193 | `disp_timer` | 4 | Core/Inc/Timer.h:52 | 51 | 32 | 32 | LCD表示更新タイマー | タイマ/カウンタ 揮発で正しい |
| 194 | `ecomode_timer` | 4 | Core/Inc/Timer.h:69 | 2 | 1 | 0 | 省電力測定中表示タイマー | タイマ/カウンタ 揮発で正しい |
| 195 | `first_LOW_timer` | 4 | Core/Inc/Timer.h:63 | 1 | 1 | 0 | 切替直後の低レンジ変換処理タイマー | タイマ/カウンタ 揮発で正しい |
| 196 | `flash_timer` | 4 | Core/Inc/Timer.h:48 | 9 | 18 | 26 | 数値、電池残量点滅タイマー | タイマ/カウンタ 揮発で正しい |
| 197 | `hold_update_timer` | 4 | Core/Inc/Timer.h:62 | 2 | 2 | 1 | 測定値表示アップデートタイマー | タイマ/カウンタ 揮発で正しい |
| 198 | `ident_wag_timer` | 4 | Core/Inc/Timer.h:81 | 2 | 1 | 0 | センサー識別タイマー | タイマ/カウンタ 揮発で正しい |
| 199 | `lcd_off_timer` | 4 | Core/Inc/Timer.h:46 | 2 | 15 | 14 | 画面表示OFFタイマー | タイマ/カウンタ 揮発で正しい |
| 200 | `lcd_power_off_timer` | 4 | Core/Inc/Timer.h:66 | 2 | 1 | 0 | LCD電源OFFタイマー | タイマ/カウンタ 揮発で正しい |
| 201 | `logger_rtc_timer` | 4 | Core/Inc/Timer.h:71 | 2 | 1 | 0 | ロガー測定日時更新タイマー | タイマ/カウンタ 揮発で正しい |
| 202 | `logger_sw_timer` | 4 | Core/Inc/Timer.h:72 | 2 | 2 | 0 | 省電力測定復帰後SW無効タイマー | タイマ/カウンタ 揮発で正しい |
| 203 | `logger_wake_timer` | 4 | Core/Inc/Timer.h:56 | 2 | 1 | 0 | ロガー測定用タイマー | タイマ/カウンタ 揮発で正しい |
| 204 | `power_on_wait_timer` | 4 | Core/Inc/Timer.h:70 | 2 | 1 | 1 | 電源ON後測定待ちタイマー | タイマ/カウンタ 揮発で正しい |
| 205 | `power_sw_timer` | 4 | Core/Inc/Timer.h:53 | 4 | 3 | 1 | POWER SW判定禁止タイマー | タイマ/カウンタ 揮発で正しい |
| 206 | `Probe_MD_Timer` | 4 | Core/Inc/Timer.h:83 | 2 | 1 | 0 | ProbeコマンドMD送信間隔タイマー | タイマ/カウンタ 揮発で正しい |
| 207 | `Probe_Stream_Timer` | 4 | Core/Inc/Timer.h:84 | 1 | 1 | 1 | Probe測定ストリーム鮮度タイマ (0=鮮度切れ→"----") | タイマ/カウンタ 揮発で正しい |
| 208 | `qr_update_cnt` | 4 | Core/Inc/mainSub.h:167 | 3 | 1 | 0 | LCD表示更新用カウント | タイマ/カウンタ 揮発で正しい |
| 209 | `range_hold_timer` | 4 | Core/Inc/Timer.h:64 | 2 | 4 | 1 | 電源ON直後強制レンジ固定タイマー | タイマ/カウンタ 揮発で正しい |
| 210 | `setting_fl_timer` | 4 | Core/Inc/Timer.h:49 | 2 | 1 | 0 | メニューカーソル点滅タイマー | タイマ/カウンタ 揮発で正しい |
| 211 | `stable_cnt` | 4 | Core/Src/AutoStable.c:33 | 4 | 2 | 0 | 安定状態カウント用 | タイマ/カウンタ 揮発で正しい |
| 212 | `stbblank_timer` | 4 | Core/Inc/Timer.h:55 | 2 | 1 | 0 | 安定判断禁止タイマー | タイマ/カウンタ 揮発で正しい |
| 213 | `stbdisp_timer` | 4 | Core/Inc/Timer.h:54 | 2 | 2 | 1 | 安定時表示更新間隔タイマー | タイマ/カウンタ 揮発で正しい |
| 214 | `timer1sec` | 4 | Core/Inc/Timer.h:44 | 5 | 2 | 0 | 1secカウントアップタイマー | タイマ/カウンタ 揮発で正しい |
| 215 | `timer_EXTCOMIN` | 4 | Core/Inc/Timer.h:58 | 2 | 2 | 2 | LCD EXTCOMIN信号タイマー | タイマ/カウンタ 揮発で正しい |
| 216 | `Timer_Stable` | 4 | Core/Inc/AutoStable.h:20 | 3 | 2 | 0 | 安定までかかった時間（測定用変数） | タイマ/カウンタ 揮発で正しい |
| 217 | `v33_power_off_timer` | 4 | Core/Inc/Timer.h:68 | 2 | 1 | 0 | 3.3V電源OFFタイマー | タイマ/カウンタ 揮発で正しい |
| 218 | `v33_power_on_timer` | 4 | Core/Inc/Timer.h:67 | 2 | 1 | 0 | 3.3V電源ONタイマー | タイマ/カウンタ 揮発で正しい |
| 219 | `cal_hsel_count` | 2 | Core/Src/Normal.c:66 | 3 | 5 | 0 | 有効な履歴件数 | タイマ/カウンタ 揮発で正しい |
| 220 | `EComm_Retry` | 2 | Core/Inc/mainSub.h:86 | 2 | 2 | 0 | EEPROM通信初期化リトライ回数 (電源OFFまでの累計) | タイマ/カウンタ 揮発で正しい |
| 221 | `EEP_Comm_Retry` | 2 | Core/Inc/mainSub.h:80 | 4 | 4 | 0 | EEPROM通信NG リトライ回数 (総計) | タイマ/カウンタ 揮発で正しい |
| 222 | `ERead_Retry` | 2 | Core/Inc/mainSub.h:88 | 6 | 2 | 0 | EEPROM読込リトライ回数 (電源OFFまでの累計) | タイマ/カウンタ 揮発で正しい |
| 223 | `EWrite_Retry` | 2 | Core/Inc/mainSub.h:90 | 14 | 2 | 0 | EEPROM書込リトライ回数 (電源OFFまでの累計) | タイマ/カウンタ 揮発で正しい |
| 224 | `RComm_Retry` | 2 | Core/Inc/mainSub.h:68 | 2 | 2 | 0 | RTC通信初期化リトライ回数 (電源OFFまでの累計) | タイマ/カウンタ 揮発で正しい |
| 225 | `RRead_Retry` | 2 | Core/Inc/mainSub.h:70 | 2 | 2 | 0 | RTC読込リトライ回数 (電源OFFまでの累計) | タイマ/カウンタ 揮発で正しい |
| 226 | `RTC_Comm_Retry` | 2 | Core/Inc/mainSub.h:62 | 4 | 4 | 0 | RTC通信NG リトライ回数 (総計) | タイマ/カウンタ 揮発で正しい |
| 227 | `RWrite_Retry` | 2 | Core/Inc/mainSub.h:72 | 2 | 2 | 0 | RTC書込リトライ回数 (電源OFFまでの累計) | タイマ/カウンタ 揮発で正しい |
| 228 | `cnt_OverWrite` | 1 | Core/Src/AutoStable.c:32 | 2 | 9 | 0 | cnt_stableのリセット有無 | タイマ/カウンタ 揮発で正しい |
| 229 | `DISP_l_sw_cnt` | 1 | Core/Inc/mainSub.h:186 | 2 | 1 | 0 | DISP SWの長押し判定カウント | タイマ/カウンタ 揮発で正しい |
| 230 | `EEP_RetryCount_Read` | 1 | Core/Inc/mainSub.h:77 | 6 | 8 | 0 | 読み込みリトライの発生回数 | タイマ/カウンタ 揮発で正しい |
| 231 | `EEP_RetryCount_Write` | 1 | Core/Inc/mainSub.h:78 | 5 | 24 | 0 | 書き込みリトライの発生回数 | タイマ/カウンタ 揮発で正しい |
| 232 | `MEM_l_sw_cnt` | 1 | Core/Inc/mainSub.h:185 | 3 | 1 | 0 | MEM SWの長押し判定カウント | タイマ/カウンタ 揮発で正しい |
| 233 | `POW_l_sw_cnt` | 1 | Core/Inc/mainSub.h:187 | 2 | 1 | 0 | POW SWの長押し判定カウント | タイマ/カウンタ 揮発で正しい |
| 234 | `probe_hs_retry` | 1 | Core/Src/IM_110.c:989 | 3 | 1 | 0 | W:(ファイルスコープ),Probe_Boot_Start,Probe_Boot_Tick / R:Probe_Boot_Tick | タイマ/カウンタ 揮発で正しい |
| 235 | `RTC_ResetCount` | 1 | Core/Inc/mainSub.h:112 | 1 | 0 | 0 | RTCリセット回数 | タイマ/カウンタ 揮発で正しい |
| 236 | `disp_buf` | 12482 | Core/Inc/Lcd.h:36 | 6 | 3 | 3 | (cmd + adr + 50バイト) x 240ライン + dummyx2=12482バイト | UI状態 揮発で正しい |
| 237 | `TR_Mode_CF` | 840 | Core/Src/IM_110.c:204 | 1 | 1 | 0 | TR 相関式テーブル ([] と No. の対応は MLSS_Mode_CF と同じ規則) | UI状態 揮発で正しい |
| 238 | `guide_disp_flag` | 5 | Core/Inc/mainSub.h:51 | 7 | 7 | 0 | ガイダンス表示フラグ（0:表示 1:非表示） | UI状態 揮発で正しい |
| 239 | `cal_sel_raw_full` | 4 | Core/Src/Normal.c:67 | 3 | 2 | 0 | 選択したフルスパン点 raw mV (ゼロ校正前) | UI状態 揮発で正しい |
| 240 | `cal_sel_raw_mid` | 4 | Core/Src/Normal.c:68 | 2 | 1 | 0 | 選択した中間点 raw mV (ゼロ校正前) | UI状態 揮発で正しい |
| 241 | `DISP_CYCLE` | 4 | Core/Inc/mainSub.h:206 | 1 | 56 | 0 | 測定中LCD表示間隔 Nx50mS = 2s | UI状態 揮発で正しい |
| 242 | `adjust_mode` | 2 | Core/Inc/mainSub.h:198 | 28 | 2 | 0 | 基板自動調整時の設定モード (内容はAllDef.h参照(operation_mode準拠))	2019/12/17追加　三浦 | UI状態 揮発で正しい |
| 243 | `operation_mode` | 2 | Core/Inc/mainSub.h:53 | 326 | 50 | 0 | 操作表示モード（内容はAllDef.h参照） | UI状態 揮発で正しい |
| 244 | `ret_mode` | 2 | Core/Inc/Normal.h:52 | 16 | 6 | 0 | エラー解除後表示モード | UI状態 揮発で正しい |
| 245 | `adj_buf_mode` | 1 | Core/Src/IM_110.c:1646 | 3 | 5 | 0 | Mode_CF 捕捉のモード (混在検出用) | UI状態 揮発で正しい |
| 246 | `adj_tc_mode` | 1 | Core/Src/IM_110.c:1650 | 3 | 3 | 0 | 温度補正 捕捉のモード | UI状態 揮発で正しい |
| 247 | `App_DL_disp_flag` | 1 | Core/Inc/mainSub.h:163 | 2 | 0 | 0 | アプリDL表示フラグ（0:表示 1:表示無し） | UI状態 揮発で正しい |
| 248 | `batt_flash_flag` | 1 | Core/Src/Normal.c:34 | 5 | 1 | 0 | 変数 | UI状態 揮発で正しい |
| 249 | `cal_setting_sel` | 1 | Core/Src/Normal.c:766 | 13 | 8 | 0 | 校正モード選択: 0=ZCAL, 1=ADCAL(2点), 2=MCAL(3点) | UI状態 揮発で正しい |
| 250 | `corr_sel` | 1 | Core/Src/Normal.c:35 | 3 | 7 | 0 | 相関式選択: 0〜9 | UI状態 揮発で正しい |
| 251 | `cur_sel_item` | 1 | Core/Src/Setting.c:23 | 3 | 3 | 0 | W:(ファイルスコープ),set_menu / R:set_menu | UI状態 揮発で正しい |
| 252 | `disp_colon_flag` | 1 | Core/Inc/mainSub.h:191 | 1 | 1 | 0 | 時計：表示フラグ | UI状態 揮発で正しい |
| 253 | `disp_lr_yajirushi` | 1 | Core/Inc/Display.h:37 | 13 | 12 | 0 | 矢印アイコン（左右） | UI状態 揮発で正しい |
| 254 | `DISP_sw_mem` | 1 | Core/Inc/mainSub.h:189 | 41 | 10 | 0 | DISP短押し判別のための記憶 | UI状態 揮発で正しい |
| 255 | `DISP_sw_step` | 1 | Core/Inc/mainSub.h:183 | 65 | 3 | 0 | DISP SWの判定用 | UI状態 揮発で正しい |
| 256 | `lcd_extcomin` | 1 | Core/Inc/mainSub.h:171 | 3 | 2 | 0 | LCD交流化信号用フラグ | UI状態 揮発で正しい |
| 257 | `lcd_off_cmd` | 1 | Core/Inc/mainSub.h:213 | 1 | 0 | 0 | LCD電源OFF処理フラグ | UI状態 揮発で正しい |
| 258 | `lcd_on_cmd` | 1 | Core/Inc/mainSub.h:212 | 2 | 0 | 0 | LCD電源ON処理フラグ | UI状態 揮発で正しい |
| 259 | `lcd_on_flag` | 1 | Core/Inc/mainSub.h:208 | 3 | 4 | 0 | LCD電源 ONフラグ | UI状態 揮発で正しい |
| 260 | `logger_sw_stop_flag` | 1 | Core/Inc/mainSub.h:216 | 1 | 0 | 0 | SW操作でのロガー測定終了フラグ 0:2000点終了 1:SW終了 | UI状態 揮発で正しい |
| 261 | `meas_flash_flag` | 1 | Core/Inc/mainSub.h:244 | 6 | 3 | 0 | センサー名称表示フラグ（0:表示無し, 表示有り） | UI状態 揮発で正しい |
| 262 | `MEM_sw_mem` | 1 | Core/Inc/mainSub.h:188 | 22 | 9 | 0 | MEM短押し判別のための記憶 | UI状態 揮発で正しい |
| 263 | `MEM_sw_step` | 1 | Core/Inc/mainSub.h:182 | 63 | 3 | 0 | MEM SWの判定用 | UI状態 揮発で正しい |
| 264 | `POW_sw_step` | 1 | Core/Inc/mainSub.h:184 | 10 | 3 | 0 | POW SWの判定用 | UI状態 揮発で正しい |
| 265 | `qr_update_flag` | 1 | Core/Inc/mainSub.h:166 | 2 | 1 | 0 | LCD表示更新用フラグ（2sec） | UI状態 揮発で正しい |
| 266 | `tansui_sw_flag` | 1 | Core/Inc/mainSub.h:47 | 2 | 4 | 0 | 淡水／海水設定（0:淡水 1:海水） | UI状態 揮発で正しい |
| 267 | `data_stable` | 1020 | Core/Src/AutoStable.c:30 | 2 | 14 | 4 | 安定判断サンプル ("MAX: 60秒×2(件／秒)" ×2倍 ＋15件) | 通信バッファ/状態 揮発で正しい |
| 268 | `Probe_RecvData` | 128 | Core/Src/IM_110.c:306 | 3 | 10 | 0 | Probe communication variables | 通信バッファ/状態 揮発で正しい |
| 269 | `RecvData` | 128 | Core/Inc/LinkSerial.h:17 | 6 | 85 | 1 | 受信バッファ | 通信バッファ/状態 揮発で正しい |
| 270 | `CalTimeStable` | 4 | Core/Inc/AutoStable.h:19 | 3 | 0 | 0 | 安定までかかった時間 | 通信バッファ/状態 揮発で正しい |
| 271 | `stable_now` | 4 | Core/Src/AutoStable.c:35 | 3 | 2 | 0 | 安定状態カウント用 | 通信バッファ/状態 揮発で正しい |
| 272 | `stable_old` | 4 | Core/Src/AutoStable.c:34 | 4 | 1 | 0 | 安定状態カウント用 | 通信バッファ/状態 揮発で正しい |
| 273 | `TimeStable` | 4 | Core/Inc/AutoStable.h:19 | 2 | 1 | 0 | 安定までかかった時間 | 通信バッファ/状態 揮発で正しい |
| 274 | `UART1_BPS` | 4 | Core/Inc/mainSub.h:201 | 2 | 1 | 0 | UART通信速度 | 通信バッファ/状態 揮発で正しい |
| 275 | `chk_stable` | 1 | Core/Inc/AutoStable.h:16 | 2 | 0 | 0 | 安定データの有無 | 通信バッファ/状態 揮発で正しい |
| 276 | `f_stable` | 1 | Core/Inc/AutoStable.h:15 | 5 | 4 | 0 | 安定状態の指示 (0/1： 不安定／安定） | 通信バッファ/状態 揮発で正しい |
| 277 | `Probe_Conn_Status` | 1 | Core/Src/IM_110.c:315 | 4 | 2 | 0 | 0=unknown / 1=connected / 2=NG(未応答) | 通信バッファ/状態 揮発で正しい |
| 278 | `Probe_Data_Valid` | 1 | Core/Src/IM_110.c:316 | 2 | 1 | 0 | 1=測定値有効 / 0=未受信 or 鮮度切れ(→ "----" 表示) | 通信バッファ/状態 揮発で正しい |
| 279 | `Probe_RecvDataP` | 1 | Core/Src/IM_110.c:307 | 2 | 2 | 0 | W:Probe_clear_RecvData,Probe_RxCallback / R:Probe_RxCallback | 通信バッファ/状態 揮発で正しい |
| 280 | `Probe_uart_end` | 1 | Core/Src/IM_110.c:309 | 4 | 3 | 0 | W:(ファイルスコープ),Probe_clear_RecvData,Probe_RxCallback / R:Probe_WaitLine,Probe_… | 通信バッファ/状態 揮発で正しい |
| 281 | `RecvDataP` | 1 | Core/Inc/LinkSerial.h:18 | 3 | 5 | 0 | W:clear_RecvData,HAL_UART_RxCpltCallback / R:HAL_UART_RxCpltCallback | 通信バッファ/状態 揮発で正しい |
| 282 | `uart_end` | 1 | Core/Inc/LinkSerial.h:20 | 4 | 1 | 0 | UART文字列受信フラグ | 通信バッファ/状態 揮発で正しい |
| 283 | `use_UART_flag` | 1 | Core/Inc/mainSub.h:195 | 3 | 6 | 0 | UART使用可能フラグ | 通信バッファ/状態 揮発で正しい |
| 284 | `num.0` | 5 | Core/Src/Display.c:475 | 24 | 75 | 0 | 表示中の数値(整数部5桁) | 関数内static 揮発で正しい |
| 285 | `num.14` | 5 | Core/Src/Normal.c:612 | 56 | 110 | 0 | 表示中の数値(整数部5桁) | 関数内static 揮発で正しい |
| 286 | `num.18` | 5 | Core/Src/Normal.c:401 | 56 | 110 | 0 | W:nrm_span_setting,nrm_span_setting_mid,nrm_depth_setting / R:nrm_span_setti… | 関数内static 揮発で正しい |
| 287 | `num.22` | 5 | Core/Src/Normal.c:220 | 56 | 110 | 0 | W:nrm_span_setting,nrm_span_setting_mid,nrm_depth_setting / R:nrm_span_setti… | 関数内static 揮発で正しい |
| 288 | `num.4` | 5 | Core/Src/Display.c:278 | 24 | 75 | 0 | TR: 4桁 + 小数点 (100/10/1/0.1 の位、num[1..4] 使用、x=29/32/35/41 に配置し x=38 に "." アイコン) | 関数内static 揮発で正しい |
| 289 | `Cal_SetVal_1_tmp.23` | 4 | Core/Src/Normal.c:221 | 8 | 17 | 0 | W:nrm_span_setting / R:nrm_span_setting | 関数内static 揮発で正しい |
| 290 | `Cal_SetVal_2_tmp.19` | 4 | Core/Src/Normal.c:402 | 2 | 7 | 0 | W:nrm_span_setting_mid / R:nrm_span_setting_mid | 関数内static 揮発で正しい |
| 291 | `calexec_time.9` | 4 | Core/Src/Normal.c:1111 | 5 | 5 | 0 | W:nrm_adjust_zero,nrm_adjust_span_m,nrm_adjust_span / R:nrm_adjust_zero,nrm_… | 関数内static 揮発で正しい |
| 292 | `Interface_Threshold_tmp.15` | 4 | Core/Src/Normal.c:613 | 2 | 7 | 0 | W:nrm_depth_setting / R:nrm_depth_setting | 関数内static 揮発で正しい |
| 293 | `EEP_Tadrs.5` | 2 | Core/Src/Adjust.c:562 | 24 | 32 | 0 | W:adj_eep_test,adj_eep_test_serial / R:adj_eep_test,adj_eep_test_serial | 関数内static 揮発で正しい |
| 294 | `EEP_Tadrs.7` | 2 | Core/Src/Adjust.c:197 | 24 | 32 | 0 | W:adj_eep_test,adj_eep_test_serial / R:adj_eep_test,adj_eep_test_serial | 関数内static 揮発で正しい |
| 295 | `index.6` | 2 | Core/Src/Normal.c:2393 | 4 | 4 | 0 | W:set_disp_his / R:set_disp_his | 関数内static 揮発で正しい |
| 296 | `last_idx.3` | 2 | Core/Src/Adjust.c:843 | 2 | 1 | 0 | W:adb_apply_adc_mask / R:adb_apply_adc_mask | 関数内static 揮発で正しい |
| 297 | `num.5` | 2 | Core/Src/Normal.c:1882 | 56 | 110 | 0 | 確定時 00 → 01 にクランプ、表示値 (1..30) - 1 で内部値 (0..29) に変換して MODE へ保存。 | 関数内static 揮発で正しい |
| 298 | `tim1_counter.0` | 2 | Core/Src/Timer.c:81 | 3 | 4 | 0 | W:HAL_TIM_PeriodElapsedCallback / R:HAL_TIM_PeriodElapsedCallback | 関数内static 揮発で正しい |
| 299 | `cal_menu_sel.12` | 1 | Core/Src/Normal.c:826 | 8 | 9 | 0 | W:nrm_cal_menu / R:nrm_cal_menu | 関数内static 揮発で正しい |
| 300 | `corr_num_sel.4` | 1 | Core/Src/Normal.c:1883 | 2 | 3 | 0 | W:nrm_corr_setting / R:nrm_corr_setting | 関数内static 揮発で正しい |
| 301 | `cur_sel_item2.0` | 1 | Core/Src/Setting.c:330 | 4 | 3 | 0 | 選択中のアイテム 0...[+], 1...[-], 2...[セット] | 関数内static 揮発で正しい |
| 302 | `cur_sel_item2.0` | 1 | Core/Src/Adjust.c:1430 | 12 | 9 | 0 | 選択中のアイテム 0...[+], 1...[-], 2...[セット] | 関数内static 揮発で正しい |
| 303 | `cur_sel_item2.1` | 1 | Core/Src/Adjust.c:1218 | 12 | 9 | 0 | 選択中のアイテム 0...[+], 1...[-], 2...[セット] | 関数内static 揮発で正しい |
| 304 | `cur_sel_item2.2` | 1 | Core/Src/Adjust.c:1080 | 12 | 9 | 0 | 選択中のアイテム 0...[+], 1...[-], 2...[セット] | 関数内static 揮発で正しい |
| 305 | `disp_sw_mem.1` | 1 | Core/Src/Setting.c:230 | 4 | 1 | 0 | W:set_menu / R:set_menu | 関数内static 揮発で正しい |
| 306 | `disp_sw_mem.11` | 1 | Core/Src/Normal.c:827 | 20 | 5 | 0 | W:nrm_span_setting,nrm_span_setting_mid,nrm_depth_setting / R:nrm_span_setti… | 関数内static 揮発で正しい |
| 307 | `disp_sw_mem.13` | 1 | Core/Src/Normal.c:614 | 20 | 5 | 0 | W:nrm_span_setting,nrm_span_setting_mid,nrm_depth_setting / R:nrm_span_setti… | 関数内static 揮発で正しい |
| 308 | `disp_sw_mem.17` | 1 | Core/Src/Normal.c:403 | 20 | 5 | 0 | W:nrm_span_setting,nrm_span_setting_mid,nrm_depth_setting / R:nrm_span_setti… | 関数内static 揮発で正しい |
| 309 | `disp_sw_mem.21` | 1 | Core/Src/Normal.c:222 | 20 | 5 | 0 | W:nrm_span_setting,nrm_span_setting_mid,nrm_depth_setting / R:nrm_span_setti… | 関数内static 揮発で正しい |
| 310 | `disp_sw_mem.3` | 1 | Core/Src/Normal.c:1884 | 20 | 5 | 0 | W:nrm_span_setting,nrm_span_setting_mid,nrm_depth_setting / R:nrm_span_setti… | 関数内static 揮発で正しい |
| 311 | `empty_bat.0` | 1 | Core/Src/Normal.c:2929 | 2 | 2 | 0 | 電池点滅用 | 関数内static 揮発で正しい |
| 312 | `first_check.1` | 1 | Core/Src/Normal.c:2899 | 2 | 1 | 0 | W:battery_check / R:battery_check | 関数内static 揮発で正しい |
| 313 | `fl_flag.10` | 1 | Core/Src/Normal.c:1112 | 43 | 24 | 0 | W:nrm_span_setting,nrm_span_setting_mid,set_cal_his_select / R:nrm_span_sett… | 関数内static 揮発で正しい |
| 314 | `initialized.1` | 1 | Core/Src/IM_110.c:1115 | 2 | 1 | 0 | W:Update_Interface_Hold / R:Update_Interface_Hold | 関数内static 揮発で正しい |
| 315 | `meas_menu_sel.2` | 1 | Core/Src/Normal.c:2005 | 4 | 4 | 0 | W:nrm_meas_menu / R:nrm_meas_menu | 関数内static 揮発で正しい |
| 316 | `prev_above.0` | 1 | Core/Src/IM_110.c:1114 | 3 | 1 | 0 | W:Update_Interface_Hold / R:Update_Interface_Hold | 関数内static 揮発で正しい |
| 317 | `result.4` | 1 | Core/Src/Adjust.c:563 | 16 | 2 | 0 | W:adj_eep_test,adj_eep_test_serial / R:adj_eep_test,adj_eep_test_serial | 関数内static 揮発で正しい |
| 318 | `result.6` | 1 | Core/Src/Adjust.c:198 | 16 | 2 | 0 | W:adj_eep_test,adj_eep_test_serial / R:adj_eep_test,adj_eep_test_serial | 関数内static 揮発で正しい |
| 319 | `span_num_sel.16` | 1 | Core/Src/Normal.c:611 | 6 | 23 | 0 | W:nrm_span_setting,nrm_span_setting_mid,nrm_depth_setting / R:nrm_span_setti… | 関数内static 揮発で正しい |
| 320 | `span_num_sel.20` | 1 | Core/Src/Normal.c:400 | 6 | 23 | 0 | MLSS: 5桁 (10000/1000/100/10/1)、num[0..4] 全使用、span_num_sel 0..4 (5 で確定) | 関数内static 揮発で正しい |
| 321 | `span_num_sel.24` | 1 | Core/Src/Normal.c:219 | 6 | 23 | 0 | "." は固定表示 (x=38) で num_sel は飛ばす、value = num[1]*100 + num[2]*10 + num[3] + nu… | 関数内static 揮発で正しい |
| 322 | `__global_locale` | 364 | (libc) | 0 | 0 | 0 | W:- / R:- | HAL/libc 対象外 |
| 323 | `__sf` | 312 | (libc) | 0 | 0 | 0 | W:- / R:- | HAL/libc 対象外 |
| 324 | `hlpuart1` | 136 | Core/Src/main.c:66 | 15 | 10 | 11 | W:ProbeRelay_Run,ProbeFupBridge_Run,MX_LPUART1_UART_Init / R:(ファイルスコープ),Prob… | HAL/libc 対象外 |
| 325 | `huart1` | 136 | Core/Src/main.c:67 | 65 | 13 | 21 | W:Change_uart_bps,ProbeRelay_Run,ProbeFupBridge_Run / R:(ファイルスコープ),ProbeRela… | HAL/libc 対象外 |
| 326 | `hadc1` | 104 | Core/Src/main.c:61 | 15 | 3 | 6 | Private variables --------------------------------------------------------- | HAL/libc 対象外 |
| 327 | `hspi1` | 100 | Core/Src/main.c:71 | 14 | 3 | 4 | W:MX_SPI1_Init / R:(ファイルスコープ) | HAL/libc 対象外 |
| 328 | `hspi2` | 100 | Core/Src/main.c:72 | 42 | 4 | 23 | W:eep_SPI_Configuration,rtc_SPI_Configuration,MX_SPI2_Init / R:(ファイルスコープ) | HAL/libc 対象外 |
| 329 | `tzinfo` | 88 | (libc) | 0 | 0 | 0 | W:- / R:- | HAL/libc 対象外 |
| 330 | `_impure_data` | 76 | (libc) | 0 | 0 | 0 | W:- / R:- | HAL/libc 対象外 |
| 331 | `htim1` | 76 | Core/Src/main.c:75 | 28 | 4 | 16 | W:MX_TIM1_Init,SystemClock_SLEEP,SystemClock_4MHz / R:(ファイルスコープ) | HAL/libc 対象外 |
| 332 | `hdma_adc1` | 72 | Core/Src/main.c:62 | 9 | 4 | 2 | W:HAL_ADC_MspInit / R:(ファイルスコープ),HAL_ADC_MspInit | HAL/libc 対象外 |
| 333 | `hdma_lpuart_rx` | 72 | Core/Src/main.c:68 | 9 | 4 | 2 | W:HAL_UART_MspInit / R:(ファイルスコープ),HAL_UART_MspInit | HAL/libc 対象外 |
| 334 | `hdma_spi1_tx` | 72 | Core/Src/main.c:73 | 9 | 4 | 2 | W:HAL_SPI_MspInit / R:(ファイルスコープ),HAL_SPI_MspInit | HAL/libc 対象外 |
| 335 | `hdma_usart1_rx` | 72 | Core/Src/main.c:69 | 9 | 4 | 2 | W:HAL_UART_MspInit / R:(ファイルスコープ),HAL_UART_MspInit | HAL/libc 対象外 |
| 336 | `hiwdg` | 16 | Core/Src/main.c:64 | 4 | 2 | 2 | W:MX_IWDG_Init / R:(ファイルスコープ) | HAL/libc 対象外 |
| 337 | `__sglue` | 12 | (libc) | 0 | 0 | 0 | W:- / R:- | HAL/libc 対象外 |
| 338 | `__tzname_dst` | 12 | (libc) | 0 | 0 | 0 | W:- / R:- | HAL/libc 対象外 |
| 339 | `__tzname_std` | 12 | (libc) | 0 | 0 | 0 | W:- / R:- | HAL/libc 対象外 |
| 340 | `_tzname` | 8 | (libc) | 0 | 0 | 0 | W:- / R:- | HAL/libc 対象外 |
| 341 | `__env` | 4 | Core/Src/syscalls.c:39 | 1 | 1 | 0 | W:(ファイルスコープ) / R:(ファイルスコープ) | HAL/libc 対象外 |
| 342 | `__malloc_free_list` | 4 | (libc) | 0 | 0 | 0 | W:- / R:- | HAL/libc 対象外 |
| 343 | `__malloc_sbrk_start` | 4 | (libc) | 0 | 0 | 0 | W:- / R:- | HAL/libc 対象外 |
| 344 | `__sbrk_heap_end` | 4 | Core/Src/sysmem.c:31 | 3 | 3 | 0 | / | HAL/libc 対象外 |
| 345 | `__stdio_exit_handler` | 4 | (libc) | 0 | 0 | 0 | W:- / R:- | HAL/libc 対象外 |
| 346 | `_daylight` | 4 | (libc) | 0 | 0 | 0 | W:- / R:- | HAL/libc 対象外 |
| 347 | `_impure_ptr` | 4 | (libc) | 0 | 0 | 0 | W:- / R:- | HAL/libc 対象外 |
| 348 | `_timezone` | 4 | (libc) | 0 | 0 | 0 | W:- / R:- | HAL/libc 対象外 |
| 349 | `environ` | 4 | Core/Src/syscalls.c:40 | 1 | 0 | 0 | W:(ファイルスコープ) / R:- | HAL/libc 対象外 |
| 350 | `errno` | 4 | (libc) | 45 | 9 | 0 | W:_kill,_wait,_unlink / R:(ファイルスコープ) | HAL/libc 対象外 |
| 351 | `prev_tzenv` | 4 | (libc) | 0 | 0 | 0 | W:- / R:- | HAL/libc 対象外 |
| 352 | `SystemCoreClock` | 4 | Core/Src/system_stm32l4xx.c:172 | 7 | 0 | 0 | / | HAL/libc 対象外 |
| 353 | `uwTick` | 4 | Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal.c:90 | 0 | 0 | 0 | / | HAL/libc 対象外 |
| 354 | `uwTickPrio` | 4 | Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal.c:91 | 0 | 0 | 0 | Invalid priority | HAL/libc 対象外 |
| 355 | `__lock___env_recursive_mutex` | 1 | (libc) | 0 | 0 | 0 | W:- / R:- | HAL/libc 対象外 |
| 356 | `__lock___malloc_recursive_mutex` | 1 | (libc) | 0 | 0 | 0 | W:- / R:- | HAL/libc 対象外 |
| 357 | `__lock___sfp_recursive_mutex` | 1 | (libc) | 0 | 0 | 0 | W:- / R:- | HAL/libc 対象外 |
| 358 | `__lock___tz_mutex` | 1 | (libc) | 0 | 0 | 0 | W:- / R:- | HAL/libc 対象外 |
| 359 | `uwTickFreq` | 1 | Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal.c:92 | 0 | 0 | 0 | 1KHz | HAL/libc 対象外 |
| 360 | `completed.1` | - | (libc) | 0 | 0 | 0 | W:- / R:- | HAL/libc 対象外 |
| 361 | `object.0` | - | (libc) | 0 | 0 | 0 | W:- / R:- | HAL/libc 対象外 |

## 7. プローブ IM-110_Probe — 全 52 件 一覧

列の意味は §6 と同じ。

| # | 変数 | B | 宣言 | W | R | & | 用途 (宣言コメント / 参照関数) | 判定 |
|---|---|---|---|---|---|---|---|---|
| 1 | `g_store` | 512 | Core/Src/IM_110.c:1048 | 1 | 11 | 14 | RAM 像 (DOUBLEWORD 読出のため 8B アライン) | 保存済 統合ストア512B(Flash page63) [書出+復元] |
| 2 | `ADC_Span` | 20 | Core/Src/IM_110.c:37 | 3 | 4 | 0 | W:uart_Set_ADC_Span,Set_Param_INI,store_apply_to_live / R:MCP3424_AD_Read,ua… | 保存済 統合ストア512B(Flash page63) [書出+復元] |
| 3 | `LED_Out` | 20 | Core/Src/IM_110.c:38 | 3 | 4 | 0 | W:uart_Set_PWM_Duty,Set_Param_INI,store_apply_to_live / R:uart_Read_Param,ua… | 保存済 統合ストア512B(Flash page63) [書出+復元] |
| 4 | `Product_Name` | 7 | Core/Src/IM_110.c:34 | 2 | 4 | 0 | Parameter Variables | 保存済 統合ストア512B(Flash page63) [書出+復元] |
| 5 | `Probe_ID` | 4 | Core/Src/IM_110.c:35 | 3 | 2 | 0 | W:uart_Set_Probe_ID,Set_Param_INI,store_apply_to_live / R:uart_Read_Param,st… | 保存済 統合ストア512B(Flash page63) [書出+復元] |
| 6 | `g_store_valid` | 1 | Core/Src/IM_110.c:1049 | 7 | 1 | 0 | W:(ファイルスコープ),store_commit,store_new_probe_init / R:uart_Read_Store | 保存済 統合ストア512B(Flash page63) [復元のみ] |
| 7 | `ad_ave_buf` | 5100 | Core/Src/IM_110.c:25 | 3 | 2 | 0 | W:MCP3424_Init,AD_AVE_Calc / R:MCP3424_Init,AD_AVE_Calc | ★要判断 |
| 8 | `AD_AVE_mV` | 20 | Core/Src/IM_110.c:21 | 3 | 5 | 0 | W:AD_AVE_Calc / R:uart_Put_Measure_Data | ★要判断 |
| 9 | `ADC_Span_S` | 20 | Core/Src/IM_110.c:39 | 1 | 1 | 0 | W:Set_Param_INI / R:uart_Read_Param | ★要判断 |
| 10 | `ADC_Zero` | 20 | Core/Src/IM_110.c:36 | 2 | 1 | 0 | W:uart_Set_ADC_Zero,Set_Param_INI / R:uart_Read_Param | ★要判断 |
| 11 | `MCP3424_AD_Digit` | 20 | Core/Src/IM_110.c:19 | 2 | 3 | 0 | MCP3424 Variables | ★要判断 |
| 12 | `MCP3424_AD_mV` | 20 | Core/Src/IM_110.c:20 | 2 | 4 | 0 | W:MCP3424_AD_Read / R:AD_AVE_Calc | ★要判断 |
| 13 | `ad_ave_idx` | 5 | Core/Src/IM_110.c:26 | 4 | 3 | 0 | W:MCP3424_Init,AD_AVE_Calc / R:MCP3424_Init,AD_AVE_Calc | ★要判断 |
| 14 | `HSPPAD143_Pressure_hPa` | 4 | Core/Src/IM_110.c:30 | 2 | 1 | 1 | HSPPAD143 Variables | ★要判断 |
| 15 | `AD_AVE_Count` | 1 | Core/Src/IM_110.c:22 | 2 | 6 | 0 | W:(ファイルスコープ),uart_Set_AD_AVE / R:AD_AVE_Calc | ★要判断 |
| 16 | `mcp3424_ch_mask` | 1 | Core/Src/IM_110.c:23 | 2 | 3 | 0 | W:(ファイルスコープ),MCP3424_Init / R:MCP3424_Init,MCP3424_AD_Read | ★要判断 |
| 17 | `mcp3424_current_ch` | 1 | Core/Src/IM_110.c:24 | 4 | 13 | 0 | W:(ファイルスコープ),MCP3424_Init,MCP3424_AD_Read / R:MCP3424_Init,MCP3424_AD_Read | ★要判断 |
| 18 | `MS_flag` | 1 | Core/Src/IM_110.c:16 | 3 | 1 | 0 | W:(ファイルスコープ),uart_Set_MS_Flag / R:main | ★要判断 |
| 19 | `RxData` | 1 | Core/Src/IIJIMA_Templete.c:15 | 0 | 6 | 4 | W:- / R:uart_init,HAL_UART_RxCpltCallback,HAL_UART_ErrorCallback | ポインタ経由で書込 (&渡し) 揮発で正しい |
| 20 | `ad_ave_num` | 5 | Core/Src/IM_110.c:27 | 3 | 5 | 0 | W:MCP3424_Init,AD_AVE_Calc / R:MCP3424_Init,AD_AVE_Calc | タイマ/カウンタ 揮発で正しい |
| 21 | `timer_uart` | 4 | Core/Src/IM_110.c:42 | 3 | 2 | 1 | Timer Variables | タイマ/カウンタ 揮発で正しい |
| 22 | `RecvData` | 512 | Core/Src/IIJIMA_Templete.c:13 | 3 | 40 | 0 | W:clear_RecvData,HAL_UART_RxCpltCallback / R:clear_RecvData,uart_Enter_Bootl… | 通信バッファ/状態 揮発で正しい |
| 23 | `RecvDataP` | 4 | Core/Src/IIJIMA_Templete.c:14 | 3 | 2 | 0 | W:(ファイルスコープ),clear_RecvData,HAL_UART_RxCpltCallback / R:HAL_UART_RxCpltCallb… | 通信バッファ/状態 揮発で正しい |
| 24 | `uart_end` | 1 | Core/Src/IIJIMA_Templete.c:19 | 3 | 1 | 0 | 実行してから clear_RecvData() で降ろす (本体/ID-200T と同じ構造。ISR 内で実行しない)。 | 通信バッファ/状態 揮発で正しい |
| 25 | `__global_locale` | 364 | (libc) | 0 | 0 | 0 | W:- / R:- | HAL/libc 対象外 |
| 26 | `__sf` | 312 | (libc) | 0 | 0 | 0 | W:- / R:- | HAL/libc 対象外 |
| 27 | `huart2` | 148 | Core/Src/main.c:55 | 11 | 3 | 18 | W:MX_USART2_UART_Init / R:(ファイルスコープ) | HAL/libc 対象外 |
| 28 | `hdma_usart2_rx` | 92 | Core/Src/main.c:56 | 9 | 4 | 2 | W:HAL_UART_MspInit / R:(ファイルスコープ),HAL_UART_MspInit | HAL/libc 対象外 |
| 29 | `hi2c1` | 84 | Core/Src/main.c:46 | 9 | 2 | 6 | Private variables --------------------------------------------------------- | HAL/libc 対象外 |
| 30 | `hi2c2` | 84 | Core/Src/main.c:47 | 9 | 2 | 6 | W:MX_I2C2_Init / R:(ファイルスコープ) | HAL/libc 対象外 |
| 31 | `_impure_data` | 76 | (libc) | 0 | 0 | 0 | W:- / R:- | HAL/libc 対象外 |
| 32 | `htim1` | 76 | Core/Src/main.c:51 | 7 | 3 | 7 | W:MX_TIM1_Init / R:(ファイルスコープ) | HAL/libc 対象外 |
| 33 | `htim16` | 76 | Core/Src/main.c:53 | 7 | 3 | 12 | W:MX_TIM16_Init / R:(ファイルスコープ) | HAL/libc 対象外 |
| 34 | `htim3` | 76 | Core/Src/main.c:52 | 6 | 3 | 7 | W:MX_TIM3_Init / R:(ファイルスコープ) | HAL/libc 対象外 |
| 35 | `FLASH_Program_Fast` | 60 | Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_flash.c:678 | 0 | 0 | 0 | / | HAL/libc 対象外 |
| 36 | `pFlash` | 28 | Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_flash.c:109 | 0 | 0 | 0 | / | HAL/libc 対象外 |
| 37 | `hiwdg` | 16 | Core/Src/main.c:49 | 4 | 1 | 2 | W:MX_IWDG_Init / R:(ファイルスコープ) | HAL/libc 対象外 |
| 38 | `__sglue` | 12 | (libc) | 0 | 0 | 0 | W:- / R:- | HAL/libc 対象外 |
| 39 | `__malloc_free_list` | 4 | (libc) | 0 | 0 | 0 | W:- / R:- | HAL/libc 対象外 |
| 40 | `__malloc_sbrk_start` | 4 | (libc) | 0 | 0 | 0 | W:- / R:- | HAL/libc 対象外 |
| 41 | `__sbrk_heap_end` | 4 | Core/Src/sysmem.c:31 | 3 | 3 | 0 | / | HAL/libc 対象外 |
| 42 | `__stdio_exit_handler` | 4 | (libc) | 0 | 0 | 0 | W:- / R:- | HAL/libc 対象外 |
| 43 | `_impure_ptr` | 4 | (libc) | 0 | 0 | 0 | W:- / R:- | HAL/libc 対象外 |
| 44 | `errno` | 4 | (libc) | 7 | 2 | 0 | W:_kill,_wait,_unlink / R:(ファイルスコープ) | HAL/libc 対象外 |
| 45 | `SystemCoreClock` | 4 | Core/Src/system_stm32g0xx.c:159 | 7 | 0 | 0 | / | HAL/libc 対象外 |
| 46 | `uwTick` | 4 | Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal.c:80 | 0 | 0 | 0 | / | HAL/libc 対象外 |
| 47 | `uwTickPrio` | 4 | Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal.c:81 | 0 | 0 | 0 | Invalid PRIO | HAL/libc 対象外 |
| 48 | `__lock___malloc_recursive_mutex` | 1 | (libc) | 0 | 0 | 0 | W:- / R:- | HAL/libc 対象外 |
| 49 | `__lock___sfp_recursive_mutex` | 1 | (libc) | 0 | 0 | 0 | W:- / R:- | HAL/libc 対象外 |
| 50 | `uwTickFreq` | 1 | Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal.c:82 | 0 | 0 | 0 | 1KHz | HAL/libc 対象外 |
| 51 | `completed.1` | - | (libc) | 0 | 0 | 0 | W:- / R:- | HAL/libc 対象外 |
| 52 | `object.0` | - | (libc) | 0 | 0 | 0 | W:- / R:- | HAL/libc 対象外 |
