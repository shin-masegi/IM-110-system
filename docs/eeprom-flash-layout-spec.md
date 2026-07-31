# IM-110 不揮発メモリ割付 仕様書（プローブ flash / 本体 EEPROM）

> 作成: 2026-07-27。**本書が不揮発メモリのバイト割付に関する唯一の真実源。**
>
> - 係数の意味・計算式は `mlss-calc-reference.md`、通信は `protocol-rs232c.md` を参照（役割分担）。
> - 現状の実装棚卸しは `variable-audit-full.md`（全 512 変数）・`probe-store-layout.md`（現行バイト配置）。
> - **`probe-flash-map.md` は旧 2 領域構成前提で陳腐化しているため退役。参照しないこと。**
> - 本書は改訂版レイアウトを定義する。移行の扱いは §4。

---

## 0. 決定事項（2026-07-27 ユーザー裁定）

`variable-audit-full.md` §2「保存が必要なのに保存されていないもの」に対する裁定。

| 項目 | 裁定 |
|---|---|
| `LED_Out` | **`[0]`〜`[4]` の 5 個すべて保存する**。`[2]`〜`[4]` は当面未使用でよい |
| `Adb_Ref_MLSS2/3` `Adb_Ref_SS2/3` `Adb_Ref_TR2/3` | **保存しない。** FW 初期値を毎起動セットする |
| 水深ゼロ | `Depth_offset`（電源ON大気圧スナップショット）から**出荷時調整係数として分離し保存する** |
| 水深スパン | **新規に追加して保存する**（★下記 §5-1 要確定） |
| `AD_AVE_Count` | **プローブ flash に保存する。** 既定値は測定時に実際に使う移動平均件数（★§5-2 要確定） |
| `mcp3424_ch_mask`（SADC） | 保存しない。本体が起動時および画面ごとに送るため |
| `Stbl_STD` および 1Wire page5-8/page9 の変数群 | **変数ごと削除する。** 1Wire 未使用なのに残っていること自体が誤り |
| 安定判断パラメータ（`stbwidthA/B` `stbtime` `stbsize` `Stbl_sel`） | 未裁定（★§5-3） |

---

## 1. 物理配置

| 項目 | 値 |
|---|---|
| プローブ flash | STM32G070 Page63 = `0x0801F800`、消去単位 2048 B |
| ストア実体 | `probe_store_t` = **32 B × 16 ページ = 512 B**（flash ページ内に 1536 B の余裕） |
| ページ構成 | 先頭 31 B ペイロード ＋ 末尾 1 B ページ XOR |
| 本体 EEPROM | M95256 32 KB、1 ページ 32 B、使用範囲 page 0-125 |
| ミラー | 本体 EEPROM **page 61-76**（16 ページ）に **プローブ flash とバイト完全一致**で保持 |

**ミラー契約**: `本体EEPROM[61+N] == プローブflash[0+N]`（N = 0..15、バイト単位で一致）。
片方のレイアウトを変えたら必ず両方の `probe_store.h` を同時に差し替えること（CLAUDE.md §3.2）。

---

## 2. プローブ flash 統合ストア（改訂版レイアウト）

### 2-0. 現行実装からの変更点

| ページ | 現行 | 改訂版 |
|---|---|---|
| Page1 | `adc_span[5]` `led_out`(スカラ) `k_depth` | `adc_span[5]` `k_depth` `ad_ave_count` |
| Page2 | プローブ設定（`sada`/`sadc`/`sel_eq_mlss`/`meas_mode`、**全域未結線**） | **廃止し、LED/水深ページへ転用** |
| Page3-15 | 変更なし | 変更なし（ページ番号も不変） |

- **総ページ数は 16 のまま**。本体 EEPROM ミラー範囲（page61-76）も変更不要。
- 削除: `sadc`（本体が毎回送る）・`sel_eq_mlss` / `meas_mode`（本体 EEPROM page12 が真実源）。
- 移動: `sada` → Page1 の `ad_ave_count`（保存する裁定に伴い正式配置）。

### 2-1. Page0 — ヘッダ `store_hdr_t`（変更なし）

| off | B | フィールド | 型 | 内容 |
|---|---|---|---|---|
| 0 | 4 | `magic` | u32 | `0x494D3253` 'IM2S' |
| 4 | 1 | `ver` | u8 | **2**（Ver.1 から変更） |
| 5 | 1 | `rsv0` | u8 | 予備 |
| 6 | 4 | `probe_id` | u32 | プローブ個体 ID |
| 10 | 4 | `last_update` | u32 | 最終更新日 packed `YYYYMMDD`（本体 RTC 由来、0=未設定） |
| 14 | 7 | `product_name[7]` | u8 | `"IM-110 "` |
| 21 | 6 | `rsv[6]` | u8 | 予備 |
| 27 | 4 | `checksum` | u32 | 全 512 B のペイロード総和（各ページ XOR と本フィールドを除く） |
| 31 | 1 | `xor_cs` | u8 | Page0 [0..30] の XOR |

### 2-2. Page1 — プローブ共通調整 A（改訂）

| off | B | フィールド | 型 | 内容 |
|---|---|---|---|---|
| 0 | 20 | `adc_span[5]` | f32×5 | ch1-5 空中 1700 span 傾き基準。`出力mV = 生mV × 1700 / adc_span[ch]` |
| 20 | 4 | `k_depth` | f32 | 水深換算 傾き [m/hPa]。★§5-1 の裁定次第で `depth_span_ship` に統合 |
| 24 | 1 | `ad_ave_count` | u8 | **新規**。プローブ移動平均件数（`SADA`）。0=平均OFF |
| 25 | 6 | `rsv[6]` | u8 | 予備 |
| 31 | 1 | `xor_cs` | u8 | ページ XOR |

### 2-3. Page2 — プローブ共通調整 B（新設。旧「プローブ設定」を置換）

| off | B | フィールド | 型 | 内容 |
|---|---|---|---|---|
| 0 | 20 | `led_out[5]` | f32×5 | **LED PWM duty 5 系統**。`[0]`=MLSS系(ch1,2) `[1]`=SS系(ch3,4)、`[2..4]` 予備 |
| 20 | 4 | `depth_zero_ship` | f32 | **新規**。水深 出荷時ゼロ（adboad.md 19）。`Depth_offset` から分離した永続値 |
| 24 | 4 | `depth_span_ship` | f32 | **新規**。水深 出荷時スパン（adboad.md 20）。★§5-1 |
| 28 | 3 | `rsv[3]` | u8 | 予備 |
| 31 | 1 | `xor_cs` | u8 | ページ XOR |

### 2-4. Page3 / Page10 — MLSS / SS ゼロ・温度（変更なし）

| off | B | フィールド | 内容 |
|---|---|---|---|
| 0 | 4 | `zr_ship` | ゼロ基準 出荷時 |
| 4 | 4 | `zr_field` | ゼロ基準 現場（`AZC` が上書き） |
| 8 | 4 | `refzr` | Ref ゼロ |
| 12 | 4 | `b` | Ref 温度補正 1 次 |
| 16 | 4 | `b2` | Ref 温度補正 2 次 |
| 20 | 11 | `rsv[11]` | 予備 |
| 31 | 1 | `xor_cs` | ページ XOR |

### 2-5. Page4 / Page11 — MLSS / SS ベース・スパン校正（変更なし）

| off | B | フィールド | 内容 |
|---|---|---|---|
| 0 | 12 | `c0,c1,c2` | 出荷時 2 次式（＝相関式 No.1 基準式） |
| 12 | 12 | `sp_a,sp_b,sp_c` | スパン校正 2 次 `y=A f²+B f+C` |
| 24 | 7 | `rsv[7]` | 予備 |
| 31 | 1 | `xor_cs` | ページ XOR |

### 2-6. Page5-9 — MLSS 相関式 No.21-30（変更なし）

1 ページに 2 式。Page5=No.21/22 … Page9=No.29/30。

| off | B | フィールド | 内容 |
|---|---|---|---|
| 0 | 12 | `q0[3]` | 2 次式 c0,c1,c2（偶数側） |
| 12 | 12 | `q1[3]` | 2 次式 c0,c1,c2（奇数側） |
| 24 | 7 | `rsv[7]` | 予備 |
| 31 | 1 | `xor_cs` | ページ XOR |

### 2-7. Page12 — SS 相関式 No.21（変更なし）

| off | B | フィールド | 内容 |
|---|---|---|---|
| 0 | 12 | `q0[3]` | 2 次式 c0,c1,c2 |
| 12 | 19 | `rsv[19]` | 予備 |
| 31 | 1 | `xor_cs` | ページ XOR |

### 2-8. Page13 — 透視度 ゼロ・ベース・スパン校正（変更なし）

| off | B | フィールド | 内容 |
|---|---|---|---|
| 0 | 4 | `trzr` | 透視度 ゼロ（出荷時/現場の区別なし。★既知の仕様矛盾） |
| 4 | 8 | `pow_a, pow_b` | 出荷時ベース 累乗式 |
| 12 | 12 | `sp_a,sp_b,sp_c` | スパン校正 2 次 |
| 24 | 7 | `rsv[7]` | 予備 |
| 31 | 1 | `xor_cs` | ページ XOR |

温度補正は SS（Page10）を参照する。

### 2-9. Page14 — 透視度 校正後累乗（変更なし）

| off | B | フィールド | 内容 |
|---|---|---|---|
| 0 | 8 | `pow_a_cal, pow_b_cal` | 校正後 累乗式。**`pow_a_cal == 0` が未校正マーカー**（Page13 へフォールバック） |
| 8 | 23 | `rsv[23]` | 予備 |
| 31 | 1 | `xor_cs` | ページ XOR |

### 2-10. Page15 — 校正点濃度 SetVal（変更なし）

| off | B | フィールド | 内容 |
|---|---|---|---|
| 0 | 8 | `mlss_sv1, mlss_sv2` | MLSS 校正点 1/2 濃度 |
| 8 | 8 | `ss_sv1, ss_sv2` | SS |
| 16 | 8 | `tr_sv1, tr_sv2` | 透視度 |
| 24 | 7 | `rsv[7]` | 予備 |
| 31 | 1 | `xor_cs` | ページ XOR |

### 2-11. 空き容量（Ver.2）

| ページ | 空き | ページ | 空き |
|---|---|---|---|
| Page0 | 7 B | Page9 | 7 B |
| Page1 | 6 B | Page10 | 11 B |
| Page2 | 3 B | Page11 | 7 B |
| Page3 | 11 B | Page12 | 19 B |
| Page4 | 7 B | Page13 | 7 B |
| Page5-8 | 各 7 B | Page14 | 23 B |
| | | Page15 | 7 B |

**合計 空き 163 B / 512 B**。加えて flash ページ内に 1536 B 未使用。

---

## 3. 本体 EEPROM ページ割付（改訂）

| page | 用途 | 状態 |
|---|---|---|
| 0 | `"IM-110"` ヘッダ ＋ 予備 | 現行維持 |
| 1-3 | 本体基板調整 予備 | 現行維持（0 fill） |
| 4 | エラー情報・本体情報（`EEP_INFO_PAGE`） | 現行維持 |
| 5 | **界面測定設定**（`Interface_Threshold`） | **1Wire 撤去により専有が確定** |
| **6-9** | **解放（予備）** | **1Wire レガシー撤去で空く** |
| 10-11 | 共通設定 拡張用 予備 | 現行維持 |
| 12 | 共通設定（`Meas_Mode` / `MLSS_MODE` / `SS_MODE` / `TR_MODE`） | **SetVal ×6 を削除**（§3-2） |
| 13-18 | 共通設定 拡張用 予備 | 現行維持 |
| 19-60 | 未使用（legacy 校正係数、2026-07-26 撤去済） | 0 fill |
| 61-76 | **統合ストアミラー**（`probe_store_t` 512 B、バイト一致） | 現行維持 |
| 77-80 | 校正係数 拡張用 予備 | 現行維持 |
| 81-95 | MLSS/界面 測定履歴（30 件） | 現行維持 |
| 96-110 | SS 測定履歴 | 現行維持 |
| 111-125 | 透視度 測定履歴 | 現行維持 |
| 126- | 未使用（将来ロガー用に予約） | `EEP_PAGE_MAX = 126` |

### 3-1. 1Wire レガシーの撤去（決定事項）

**削除対象**（変数・関数・呼び出しをすべて）:

| 種別 | 対象 |
|---|---|
| 関数 | `read_param_1wireinfo` / `write_param_1wireinfo`（page5,6,7,8）、`read_param_1wirebk` / `write_param_1wirebk`（page9）、ラッパの `eep_read_1wireinfo` / `eep_read_1wirebk` / `eep_write_1wireinfo` / `eep_write_1wirebk` |
| 呼び出し | `Adjust.c:554,555` / `Adjust.c:799,800` / `LinkSerial.c:1428,1431` / `mainSub.c:1100,1101,1815` |
| 変数 | `Stbl_STD`、`WAG_*` 42 個、`WIRE_*` 6 個（`variable-audit-full.md` §5 に全リスト） |

**撤去の根拠**: IM-110 に 1Wire デバイスは存在しない（本体⇔プローブは RS-232C、プローブ基板ネットリストにも 1Wire 系なし）。
`Eeprom.h` の v2.0 変更履歴（2026-05-13）は既に「旧 1Wire 領域 (page 5-9) を**廃止**し前詰め」と記載しており、
コードの撤去が漏れていた。

**撤去により解消される不具合**: `write_param_1wireinfo()` が page5 を書くため、
`eep_write_adjust()` が `write_param_interface()` で書いた `Interface_Threshold` が直後に破壊されていた
（呼び出しが `eep_write_adjust(); eep_write_1wireinfo();` と並んでいる）。

**実装時の注意**: `Stbl_STD` は `normal_disp` と `start_measure` から読まれている（`variable-audit-full.md` §3-2）。
変数を削除する際、これらの参照箇所も併せて修正すること。

### 3-2. SetVal の二重保持を解消

`MLSS/SS/TR_Cal_SetVal_1/2`（6 float）は現在 **本体 EEPROM page12 と プローブストア Page15 の両方**にある。
`store_adopt_probe()`（`IM_110.c:799`）が unpack 直後に `write_param_common()` を呼ぶため実質プローブが勝っており、
page12 側は冗長。**page12 から SetVal を削除し、プローブストア Page15 を唯一の保存先とする。**

page12 の改訂後レイアウト:

| off | B | フィールド |
|---|---|---|
| 0 | 1 | マジックバイト |
| 1 | 1 | `Meas_Mode` |
| 2 | 1 | `MLSS_MODE` |
| 3 | 1 | `SS_MODE` |
| 4 | 1 | `TR_MODE` |
| 5 | 26 | 予備 |
| 31 | 1 | ページ XOR |

**相関式 No. と測定モードは本体 EEPROM が真実源**とする。本体を交換した場合、
相関式選択を含む本体側の設定が初期化されるのは仕様として正しい（調整係数はプローブから復元される）。

---

## 4. 移行（レイアウト改訂時）

- `store_valid()` の判定は **magic + 各ページ XOR + checksum の 3 段**（`probe_store.h`）。
- 旧レイアウトのストアもそのまま「有効」として読まれるため、フィールド配置を変える改訂では
  **実機を `RPF`（新品化）してから再調整する**運用を前提とすること
  （既存個体の値が別の意味で解釈されるのを防ぐ手段）。現時点で調整済みの実機は 1 台のみのため、再調整で足りる。
- 新品既定は `probe_store.h store_set_new_probe_defaults()` が唯一のソース。
  **追加フィールドには必ず既定値を書くこと。** 0 のまま放置すると本体 unpack で `log10(0)` → NaN 破綻が再発する
  （`mlss-calc-reference §12.1`）。

### 4-1. 追加フィールドの新品既定値

| フィールド | 既定値 | 由来 |
|---|---|---|
| `led_out[0]` | `0.36f`（`STORE_DEF_LED_OUT`） | 現行 `LED_OUT_INI` |
| `led_out[1]` | `0.36f` | 同上（SS 系も同一初期 duty） |
| `led_out[2..4]` | `0.36f` | 未使用枠。0 ではなく有効値を入れる |
| `depth_zero_ship` | ★§5-1 | |
| `depth_span_ship` | ★§5-1 | |
| `ad_ave_count` | ★§5-2 | |

---

## 5. 未確定事項

### 5-1. ★ 水深スパンの扱い

裁定は「スパン（傾き）も新規追加、保存する」。ただし傾きは既に `k_depth` として Page1 に保存されている
（`Depth = (P − Depth_offset) × k_depth`、`Depth_Calib_Span6m()` が `6.0f / dp` で書き込む）。

| 案 | 内容 |
|---|---|
| A | `k_depth` を出荷時スパンとして扱い、`depth_span_ship` は設けない（Page2 に 4 B の空きが増える） |
| B | `k_depth`（実行時換算係数）と `depth_span_ship`（出荷時値）を別枠で持ち、ゼロ初期化で後者から前者へ復帰させる。MLSS の `zr_ship`/`zr_field` と同じ構図 |

**未決。** 本書は暫定的に案 B（別枠）で記述している。案 A なら Page2 の `depth_span_ship` を削除する。

### 5-2. ★ `ad_ave_count` の既定値

裁定は「初期値は現在の測定中の移動平均件数」。ただし現状は次のとおり食い違っている。

- プローブ FW 既定 `AD_AVE_INI` = **30**
- 本体が起動ハンドシェイクで `SADA,0`（平均 OFF）を送る（`IM_110.c:1085`）ため、**測定中の実効値は 0**
- 本体側は別途 `MAIN_AD_AVE_COUNT` = 10 件で移動平均している

**未決**（0 か 30 か、あるいは別の値か）。

### 5-3. ★ 安定判断パラメータ

`stbwidthA` / `stbwidthB`（安定判断幅）、`stbtime`（表示更新間隔）、`stbsize`（サンプル数）、
`Stbl_sel`（条件選択）を保存対象にするか FW 固定にするかが未裁定。
保存するなら Page1 の空き 6 B では足りないため、Page12（空き 19 B）または Page14（空き 23 B）へ配置する。

### 5-4. ★ 透視度ゼロの出荷時/現場 分離

Page13 は `trzr` 1 枠のみで、MLSS/SS の `zr_ship`/`zr_field` 2 枠構成と非対称。
`mlss-calc-reference` §193「SS/TR も同様」と §501「P3/P10 のみ」が矛盾したまま。Page13 に空きは 7 B あるため
分離は可能。

---

## 6. 本書に従って実装するときの手順

1. `probe_store.h` を Ver.2 レイアウトへ改訂し、**両リポに同一内容で配置**する（CLAUDE.md §3.2）。
2. `store_set_new_probe_defaults()` に追加フィールドの既定値を書く。
3. プローブ `store_apply_to_live` / `store_capture_from_live` に `led_out[0..4]` / `ad_ave_count` を結線する。
4. 本体 `store_unpack_to_globals` / `store_pack_from_globals` に水深の出荷時ゼロ／スパンを結線する。
5. 1Wire レガシーを §3-1 のリストどおり削除し、`Stbl_STD` の参照箇所を修正する。
6. page12 から SetVal を削除する（§3-2）。
7. 両リポで `make -j` を通し、`RPF` → `RPG` でページダンプを取り、本書の各表と突き合わせて検証する。
