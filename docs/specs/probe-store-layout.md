# プローブ統合ストア バイト配置 棚卸し（現状 as-implemented）

> 作成: 2026-07-27 / 目的: **配置設計のやり直しにあたり、現状どのページの何バイトが何に使われ、
> どこが空いているかを一覧化する**。
>
> - このファイルは **実装（コード）から起こした現状記録**であり、意味論の真実源ではない。
>   各係数の意味・計算式は `mlss-calc-reference.md §12` を参照。
> - 参照した実装: `IM-110_Probe/Core/Inc/probe_store.h`（構造体定義）、
>   `IM-110_Probe/Core/Src/IM_110.c`（`store_apply_to_live` / `store_capture_from_live`）、
>   `IM-110/Core/Src/IM_110.c`（`store_unpack_to_globals` / `store_pack_from_globals`）、
>   `IM-110/Core/Inc/Eeprom.h`（ミラー配置）。
> - **`probe-flash-map.md` は旧2領域構成（PARAM Page62 + 係数 Page63）前提で全面的に陳腐化している。**
>   本ファイルが現状の配置記録であり、`probe-flash-map.md` は退役扱いとすること。

---

## 0. 物理配置と容量

| 項目 | 値 | 根拠 |
|---|---|---|
| 格納先 | プローブ flash Page63 = `0x0801F800` | `IM_110.h` `STORE_AD` |
| flash ページサイズ | **2048 B**（STM32G070、消去単位） | `flash_erase_program()` が `FLASH_PAGE_SIZE` 単位で消去 |
| ストアが使う量 | **512 B**（`probe_store_t`） | `_Static_assert(sizeof(probe_store_t)==512)` |
| **flash ページ内の未使用** | **1536 B** | 2048 − 512 |
| 構成 | 32 B × 16 ページ（Page0..15） | `STORE_PAGE_SIZE` / `STORE_NPAGE` |
| 各ページの実ペイロード | 31 B（末尾 1 B = ページXOR） | `STORE_PAGE_PAYLOAD` |
| 本体EEPROMミラー | page 61-76（32 B × 16、**バイト完全一致**） | `Eeprom.h` `EEP_MIRROR_PAGE`/`EEP_MIRROR_COUNT` |
| ミラー直後の予備 | page 77-80（4 page = **128 B**） | `Eeprom.h` "校正係数 拡張用 予備" |
| 撤去済みで空いている本体EEPROM | page 19-60（42 page = **1344 B**） | legacy 校正ページ、2026-07-26 撤去 |

**ストアを 16 ページ超へ拡張する場合の制約は本体EEPROM側**。page77-80 の 4 ページ（→計 20 ページ / 640 B）までは
既存予備で吸収でき、それ以上は撤去済みの page19-60 を再割当すれば最大 page19-80（62 page = 1984 B）まで取れる。
プローブ flash 側は 2 KB ページ内に 1536 B 余っているので事実上の制約にならない。

---

## 1. 凡例

**アクセス** 列 = その領域を live 変数へ反映／live から取り込むのが誰か。

| 記号 | 意味 |
|---|---|
| P | プローブ FW が live 変数と往復させる |
| M | 本体 FW が live グローバルと往復させる（`store_unpack_to_globals`/`store_pack_from_globals`） |
| — | **どちらも触らない**（枠だけあって結線されていない = 事実上デッド） |
| (保管) | プローブは 32 B ページとして保管・転送するだけで解釈しない |

---

## 2. ページ別 バイト配置

### Page0 — ヘッダ `store_hdr_t`（32 B）

| offset | size | フィールド | 型 | 内容 | アクセス |
|---|---|---|---|---|---|
| 0 | 4 | `magic` | u32 | `0x494D3253` 'IM2S' | P/M |
| 4 | 1 | `rsv1` | u8 | **空き** | — |
| 5 | **1** | `rsv0` | u8 | **空き** | — |
| 6 | 4 | `probe_id` | u32 | プローブID（`SID`/`RPP`/`WPP`） | P/M |
| 10 | 4 | `last_update` | u32 | 最終更新日 packed YYYYMMDD（本体RTC由来） | M |
| 14 | 7 | `product_name[7]` | u8×7 | `"IM-110 "` | P |
| 21 | **6** | `rsv[6]` | u8×6 | **空き** | — |
| 27 | 4 | `checksum` | u32 | ストア全体のバイト総和 | P/M |
| 31 | 1 | `xor_cs` | u8 | ページXOR | P/M |

空き **7 B**

### Page1 — プローブ共通調整 `store_p1_common_t`（32 B）

| offset | size | フィールド | 型 | 内容 | アクセス |
|---|---|---|---|---|---|
| 0 | 20 | `adc_span[5]` | f32×5 | ch1-5 空中1700 span傾き基準 | P |
| 20 | 4 | `led_out` | f32 | LED PWM duty **スカラ1個** | P（`LED_Out[0]` のみ） |
| 24 | 4 | `k_depth` | f32 | 水深換算 傾き [m/hPa] | M（`Depth_k`） |
| 28 | **3** | `rsv[3]` | u8×3 | **空き** | — |
| 31 | 1 | `xor_cs` | u8 | ページXOR | P/M |

空き **3 B**（このページは実質満杯）

### Page2 — プローブ設定 `store_p2_setting_t`（32 B）★全域デッド

| offset | size | フィールド | 型 | 内容 | アクセス |
|---|---|---|---|---|---|
| 0 | 1 | `sada` | u8 | ADC 移動平均回数（`SADA`） | **—** |
| 1 | 1 | `sadc` | u8 | ADC 有効chビットマスク（`SADC`） | **—** |
| 2 | 1 | `sel_eq_mlss` | u8 | 選択中 相関式No.(MLSS) | **—** |
| 3 | 1 | `meas_mode` | u8 | 測定モード | **—**（live 変数自体が無い） |
| 4 | **27** | `rsv[27]` | u8×27 | **空き** | — |
| 31 | 1 | `xor_cs` | u8 | ページXOR | P/M |

**4 フィールドとも読み書きする側が存在しない。** プローブの `store_apply_to_live`/`store_capture_from_live` は
触らず、本体の `store_pack_from_globals` も「hdr/common/setting は既存 s を保持」として書かない。
`store_set_new_probe_defaults()` が memset した **0 のまま永久に 0**。
→ 実効空き **31 B**

### Page3 — MLSS ゼロ/温度 `store_zero_temp_t`（32 B）

| offset | size | フィールド | 型 | 内容 | アクセス |
|---|---|---|---|---|---|
| 0 | 4 | `zr_ship` | f32 | ゼロ基準 出荷時 | M |
| 4 | 4 | `zr_field` | f32 | ゼロ基準 現場（`AZC` 上書き） | M |
| 8 | 4 | `refzr` | f32 | Ref ゼロ | M |
| 12 | 4 | `b` | f32 | Ref 温度補正 1次 | M |
| 16 | 4 | `b2` | f32 | Ref 温度補正 2次 | M |
| 20 | **11** | `rsv[11]` | u8×11 | **空き** | — |
| 31 | 1 | `xor_cs` | u8 | ページXOR | P(保管)/M |

空き **11 B**

### Page4 — MLSS ベース/校正 `store_base_cal_t`（32 B）

| offset | size | フィールド | 型 | 内容 | アクセス |
|---|---|---|---|---|---|
| 0 | 12 | `c0,c1,c2` | f32×3 | 出荷時2次式（基準式 = 相関式No.1） | M |
| 12 | 12 | `sp_a,sp_b,sp_c` | f32×3 | スパン校正 2次 | M |
| 24 | **7** | `rsv[7]` | u8×7 | **空き** | — |
| 31 | 1 | `xor_cs` | u8 | ページXOR | P(保管)/M |

空き **7 B**

### Page5-9 — MLSS 相関式 No.21-30 `store_eqpair_t` × 5（各 32 B）

1 ページに 2 式。Page5 = No.21/22、Page6 = No.23/24 … Page9 = No.29/30。

| offset | size | フィールド | 型 | 内容 | アクセス |
|---|---|---|---|---|---|
| 0 | 12 | `q0[3]` | f32×3 | 2次式 c0,c1,c2（偶数側） | M |
| 12 | 12 | `q1[3]` | f32×3 | 2次式 c0,c1,c2（奇数側） | M |
| 24 | **7** | `rsv[7]` | u8×7 | **空き** | — |
| 31 | 1 | `xor_cs` | u8 | ページXOR | P(保管)/M |

空き **7 B × 5 = 35 B**

### Page10 — SS ゼロ/温度 `store_zero_temp_t`（32 B）

Page3 と同一レイアウト。空き **11 B**

### Page11 — SS ベース/校正 `store_base_cal_t`（32 B）

Page4 と同一レイアウト。空き **7 B**

### Page12 — SS 相関式 No.21 `store_eqsingle_t`（32 B）

| offset | size | フィールド | 型 | 内容 | アクセス |
|---|---|---|---|---|---|
| 0 | 12 | `q0[3]` | f32×3 | 2次式 c0,c1,c2 | M |
| 12 | **19** | `rsv[19]` | u8×19 | **空き** | — |
| 31 | 1 | `xor_cs` | u8 | ページXOR | P(保管)/M |

空き **19 B**

### Page13 — TR ゼロ/ベース/校正 `store_tr_t`（32 B）

| offset | size | フィールド | 型 | 内容 | アクセス |
|---|---|---|---|---|---|
| 0 | 4 | `trzr` | f32 | TR ゼロ（**出荷時/現場の区別なし 1 枠のみ**） | M |
| 4 | 4 | `pow_a` | f32 | 出荷時ベース 累乗 a | M |
| 8 | 4 | `pow_b` | f32 | 出荷時ベース 累乗 b | M |
| 12 | 12 | `sp_a,sp_b,sp_c` | f32×3 | スパン校正 2次 | M |
| 24 | **7** | `rsv[7]` | u8×7 | **空き** | — |
| 31 | 1 | `xor_cs` | u8 | ページXOR | P(保管)/M |

空き **7 B**。温度補正は SS（Page10）を参照する。

### Page14 — TR 校正後累乗 `store_p14_t`（32 B）

| offset | size | フィールド | 型 | 内容 | アクセス |
|---|---|---|---|---|---|
| 0 | 4 | `pow_a_cal` | f32 | 校正後 累乗 a（0 = 未校正マーカー） | M |
| 4 | 4 | `pow_b_cal` | f32 | 校正後 累乗 b | M |
| 8 | **23** | `rsv[23]` | u8×23 | **空き** | — |
| 31 | 1 | `xor_cs` | u8 | ページXOR | P(保管)/M |

空き **23 B**

### Page15 — 校正点濃度 SetVal `store_setval_t`（32 B）

| offset | size | フィールド | 型 | 内容 | アクセス |
|---|---|---|---|---|---|
| 0 | 4 | `mlss_sv1` | f32 | MLSS 校正点1 濃度 | M |
| 4 | 4 | `mlss_sv2` | f32 | MLSS 校正点2 濃度 | M |
| 8 | 4 | `ss_sv1` | f32 | SS 校正点1 | M |
| 12 | 4 | `ss_sv2` | f32 | SS 校正点2 | M |
| 16 | 4 | `tr_sv1` | f32 | TR 校正点1 | M |
| 20 | 4 | `tr_sv2` | f32 | TR 校正点2 | M |
| 24 | **7** | `rsv[7]` | u8×7 | **空き** | — |
| 31 | 1 | `xor_cs` | u8 | ページXOR | P(保管)/M |

空き **7 B**

---

## 3. 集計

| 区分 | バイト数 |
|---|---|
| ストア全体 | 512 |
| ページXOR（1 B × 16） | 16 |
| ヘッダ管理情報（magic/ver/checksum） | 9 |
| 実データ（係数・調整値） | 323 |
| **明示的な空き（`rsv` 合計）** | **164** |
| うち Page2 のデッドフィールド 4 B を含めた実効空き | 168 |

ページ別の空き内訳:

| ページ | 空き | ページ | 空き |
|---|---|---|---|
| Page0 | 7 B | Page9 | 7 B |
| Page1 | 3 B | Page10 | 11 B |
| Page2 | **27 B**（+デッド 4 B） | Page11 | 7 B |
| Page3 | 11 B | Page12 | **19 B** |
| Page4 | 7 B | Page13 | 7 B |
| Page5 | 7 B | Page14 | **23 B** |
| Page6 | 7 B | Page15 | 7 B |
| Page7 | 7 B | | |
| Page8 | 7 B | | |

**512 B のうち 164 B（32 %）が空き**、加えて flash ページ内に 1536 B、本体EEPROM側にも
page77-80 の 128 B と撤去済み page19-60 の 1344 B がある。容量は制約になっていない。

---

## 4. 保存されるべきなのにストアに枠が無い / 結線されていない値

2026-07-27 の調査で判明した漏れ。**再設計時にこれを全て収容すること。**

| 値 | 現状 | 影響 |
|---|---|---|
| `LED_Out[1..4]` | 枠なし（`led_out` はスカラ）。旧 PARAM(Page62) は `LED_Out[0..2]` の 3 個を保存していた | LED PWM を MLSS系/SS系で分ける仕様変更（adboad.md 4 / 7）が保存できない |
| `AD_AVE_Count`（`SADA`） | Page2 に `sada` 枠はあるが**未結線** | 毎起動 `AD_AVE_INI`=30 に戻る |
| `mcp3424_ch_mask`（`SADC`） | Page2 に `sadc` 枠はあるが**未結線** | 毎起動 `MCP3424_Init(0x0F)` で 0x0F 固定 |
| 選択中相関式No.(MLSS) | Page2 に `sel_eq_mlss` 枠はあるが**未結線**（本体は自前EEPROM page12 に保存） | プローブ交換時に相関式選択が追随しない |
| 測定モード | Page2 に `meas_mode` 枠はあるが live 変数自体が存在しない | 未実装 |
| SEL アンプ切替 PWM 周波数（`SSF`） | 枠なし。TIM16 の PSC/ARR/CCR を直接操作 | 毎起動 CubeMX 既定へ |
| SEL モード（`SEL,0/1/2`）・SEL3 状態 | 枠なし | 毎起動リセット（ch5 廃止中のため現状実害なし） |
| **水深 出荷時ゼロ**（adboad.md 19） | 枠なし。**電源ON時の大気圧スナップショット `Depth_offset`（揮発 static）を流用している** | 出荷時調整が電源で消える。かつ現場の界面ゼロ校正（ZCAL）と同じ変数を共有しており相互に上書きする。`Depth_Calib_Span6m()` もこの揮発値を基準にしている |
| TR ゼロの出荷時/現場 分離 | Page13 は `trzr` 1 枠のみ（MLSS/SS は `zr_ship`/`zr_field` の2枠） | 仕様書 §193「SS/TR も同様」と §501「P3/P10 のみ」が矛盾したまま |
| `Adb_Ref_*`（ADBOAD 基準器設定値 6 個） | 枠なし（本体 RAM のみ） | 電源で初期値に戻る |

### 意図的に廃止済みだが残骸が残っているもの

| 値 | 状態 |
|---|---|
| `ADC_Zero[0..4]` | 旧 PARAM は保存していた。統合ストアで意図的に廃止。現在も `ADC_Zero[ch] = raw;` で書かれるが誰も読まない |
| `ADC_Span_S[0..4]` | 旧 PARAM でも未保存。1850 固定のデッドデータ |

いずれも `RPP`（`uart_Read_Param`）が 5 個ずつ表示し続けており、`LED Out[0..4]` と合わせて
**画面上は 15 個保存されているように見えるが、実際に flash と往復するのは `ADC_Span[5]` と `LED_Out[0]` の 6 個だけ**。

---

## 5. 再設計時の注意

1. **本体EEPROMミラーとバイト完全一致**（`mlss-calc-reference §A-2`）。レイアウトを変えたら
   `probe_store.h` を両リポで同時に差し替える（CLAUDE.md §3.2）。
2. **`store_valid()` は magic + 各ページ XOR + checksum で判定する**。旧レイアウトのストアも
   「有効」として読まれ、値が別の意味で解釈される。フィールドを動かす改訂をしたら、
   実機を `RPF`（新品化）してから再調整すること。
3. **1 ページ = 7 float が上限**（28 B + 予備3 B + XOR 1 B）。8 float 必要なら別ページへ割る。
4. **`store_set_new_probe_defaults()` は新品既定の単一ソース**。フィールド追加時は必ずここへ
   既定値を書く。0 のまま放置すると本体 unpack で `log10(0)` → NaN 破綻の再発になる
   （`mlss-calc-reference §12.1`）。
5. **ページ追加時は本体EEPROM側の `EEP_MIRROR_COUNT` と page77-80 の予備消費を確認**。
   20 ページ超なら legacy page19-60 の再割当が要る。
