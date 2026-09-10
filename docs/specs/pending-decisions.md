# 未裁定事項 一覧（1 パスで潰すための決定表）

> 作成: 2026-07-27。`variable-audit-full.md` の総ざらいで残った**判断が要る項目だけ**を集めた。
> 各行に「現状・根拠(file:line)・誰が使うか・裁定案・裁定した場合の影響」を載せてあるので、
> **この表だけで決められる**（他の資料を開かなくてよい）状態にしてある。
>
> 裁定済みの項目は §0 に記載。実装は裁定が揃ってから行う。コード・仕様書には未着手。

---

## 0. 裁定済み（2026-07-27）

| 項目 | 裁定 |
|---|---|
| `LED_Out` | `[0]`〜`[4]` の 5 個すべて保存。`[2..4]` は当面未使用 |
| `Adb_Ref_*` 6 個 | 保存しない。FW 初期値を毎起動セット |
| 水深 出荷時ゼロ | `Depth_offset` から分離して保存 |
| 水深 スパン | 既存 `k_depth` を出荷時スパンとして使う。別枠は作らない |
| `AD_AVE_Count` | プローブ flash に保存。**初期値 0**（測定中は本体が `SADA,0` を送り、平均は本体側 10 件で行うため） |
| `mcp3424_ch_mask` | 保存しない（画面ごとに 0011/1100 と変えるため） |
| `Stbl_STD` | 変数は残す・保存しない・`#define` で初期値 **1.0**（`mainSub.c:1582` の旧既定と同値） |
| `stbwidthA` `stbwidthB` `stbtime` `stbsize` `Stbl_sel` | `Stbl_STD` と同じ扱い（変数残す・保存しない・`#define` 初期値） |
| 1Wire レガシー（page5-9） | 変数・関数・呼び出しごと削除 |
| ストアの正当性判定 | `magic` + 各ページ XOR + checksum の 3 段のみ（実装済み）。§6 参照 |

**判断不要と確定したもの**: 「★未参照 9 件」は全て外部ライブラリ内部変数
（`generator` `aindex` `alpha` `generatorInitialized` = qrencode/rsecc.c、
`tzinfo` `_tzname` `_daylight` `_timezone` `prev_tzenv` = newlib）。プロジェクト側の判断対象ではない。

---

## 1. ID-200T の DO 測定チェーンを IM-110 から切るか

### 現状

`start_measure()`（`mainSub.c:2481`）は **DO 計の測定開始処理そのもの**。

```c
uint8_t start_measure(void) {
    if (Y0 < Y0mid) { calc1(); calc2(); }   // Y0(水温ADmV) から水温計算
    else            { calc3(); calc4(); }
    calc6(now_range); calc7(now_range);     // DO アンプレンジから DO 算出
    PowerOn_Amp  = AD;                      // センサー出力
    PowerOn_Temp = (int32_t)(WTemp * 10.0); // 水温
    normal_disp(NULL, NULL);
    if (DO < 1.0)      DOAcc = 2; else DOAcc = 1;   // DO 表示小数桁
    if (DO < Stbl_STD) Stbl_sel = 0; else Stbl_sel = 1;  // 安定判断条件
    ...
```

**これを IM-110 のコードが呼んでいる**: `Normal.c:1089` / `Normal.c:2606` / `Setting.c:268`

IM-110 が測るのは MLSS / SS / 透視度 / 水深（プローブ経由）で、DO と水温は測らない。
`Y0`（水温 AD mV）・`AD`（センサー出力）・`now_range`（DO アンプ増幅レンジ）に IM-110 で意味のある値は入らない。

### 関係する変数（16 個、いずれも保存対象ではない）

`AD` `AD20` `AIR_PER` `DO` `DOH` `DOAcc` `DO_Hold` `FCL` `FDOH` `FH` `FTH` `FTL` `R` `WA_F` `WA_T` `WTemp` `Y0` `Y1_` `Y2`

### 裁定案

**A. 切る** — `start_measure()` から DO/水温の計算を外し、IM-110 に必要な処理
（`normal_disp()` と安定判断条件のセット）だけ残す。`calc1-4/6/7` と DO 系変数は削除。

**B. 残す** — 動いているので触らない。DO 系変数は無意味な値のまま残る。

**影響**: A を採ると `DOAcc`（表示小数桁）と `Stbl_sel`（安定判断条件）の決定基準を
DO 値から IM-110 の測定値（MLSS など）へ置き換える必要がある。**この置き換え基準の指示が要る。**

---

## 2. ADBOAD に残る ID-200T の水温調整画面を消すか

### 現状

`do_syori4()`（DO 水温 20℃ 調整係数）と `do_syori5()`（35℃）が `Adjust.c:2093` / `Adjust.c:2099` から呼ばれている。
**`adboad.md` にこの画面は存在しない**（adboad.md の 1-23 に該当項目なし）。

### 裁定案

**消す。** adboad.md が調整画面の全数を定義しており、そこに無い画面は残す理由がない。

---

## 3. `SetVal`（校正点濃度 6 個）の二重保持を解消するか

### 現状

同じ値が 2 箇所に保存されている。

| 保存先 | 根拠 |
|---|---|
| プローブ統合ストア Page15 | `store_unpack_to_globals` / `store_pack_from_globals` |
| 本体 EEPROM page12 | `read_param_common:2783` / `write_param_common:2826`（`Eeprom.h:29`） |

`store_adopt_probe()`（`IM_110.c:799`）が unpack 直後に `write_param_common()` を呼ぶため、
**実質プローブ側が勝ち、page12 の値は使われない。**

### 裁定案

**page12 から SetVal ×6 を削除し、プローブストア Page15 を唯一の保存先とする。**
page12 は `Meas_Mode` / `MLSS_MODE` / `SS_MODE` / `TR_MODE` の 4 バイトのみになる。

**影響**: プローブ未接続時に SetVal がミラー（page61-76）経由で復元されるため、動作は変わらない。

---

## 4. プローブストア Page2 の `sel_eq_mlss` / `meas_mode` を削除するか

### 現状

Page2 に枠だけあり、プローブ側 `store_apply_to_live`/`store_capture_from_live` も
本体側 `store_pack_from_globals` も触らない（新品既定の 0 のまま）。
同じ値は本体 EEPROM page12 に保存されており、電源 OFF/ON で保持される（実機確認済）。

### 裁定案

**削除する。** 相関式 No. と測定モードは本体 EEPROM page12 を真実源とする。
本体交換時に本体側の設定が初期化されるのは仕様として正しい（調整係数はプローブから復元される）。

---

## 5. 透視度ゼロを出荷時/現場に分けるか

### 現状

| モード | ゼロ枠 |
|---|---|
| MLSS | `zr_ship` / `zr_field` の 2 枠（Page3） |
| SS | `zr_ship` / `zr_field` の 2 枠（Page10） |
| **透視度** | **`trzr` 1 枠のみ**（Page13） |

仕様書側も矛盾している。`mlss-calc-reference` §193 は「SS/TR も同様（2 枠）」、§501 は「P3/P10 のみ」。
Page13 には空きが 7 B あるため、4 B 追加して 2 枠化することは可能。

### 裁定案

**2 枠化する**（MLSS/SS と揃える）。`AZCS`(出荷時清水ゼロ) / `AZRI`(ゼロ初期化) が透視度でも
MLSS/SS と同じ動作になり、モードごとの例外が消える。

---

## 6. レイアウト改訂時の移行方式 → 裁定済み・実装済み（2026-07-30）

`store_valid()` の判定は **magic + 各ページ XOR + checksum の 3 段**。
両リポの `probe_store.h` は同一内容で、Page0 ヘッダの offset 4 は空き（`rsv1`）。

**運用上の注意**: 旧レイアウトのストアも「有効」として読まれる。
フィールド配置を変える改訂をしたら、実機を `RPF`（新品化）してから再調整すること。

---

## 7. 決定後の実装順

裁定が揃ったら以下の順で行う（`eeprom-flash-layout-spec.md` §6 と同じ）。

1. `probe_store.h` を Ver.2 へ改訂、両リポに同一内容で配置
2. `store_set_new_probe_defaults()` に追加フィールドの既定値
3. プローブ側 `store_apply_to_live` / `store_capture_from_live` の結線
4. 本体側 `store_unpack_to_globals` / `store_pack_from_globals` の結線
5. 1Wire レガシー削除、`Stbl_STD` ほかを `#define` 初期値へ
6. page12 から SetVal 削除
7. 両リポ `make -j` → `RPF` → `RPG` ダンプで表と突合
