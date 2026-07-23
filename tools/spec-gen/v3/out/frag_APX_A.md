本付録は、IM-110 が水質計測値（MLSS・SS・透視度）および界面（水深）を算出する際に用いる計算式・補正式を、現行ファームウェア実装（本体 `IM-110/Core/Src/IM_110.c`、プローブ `IM-110_Probe/Core/Src/IM_110.c`）を真実源として記載したものである。各式が MLSS・SS・透視度・界面のいずれに属するかを明示する。数式中の乗算は `*`、除算は `/`、括弧は `()` で表記する。

計算チェーン全体は、プローブ側で生 AD 値を mV に正規化して送信し、本体側でその mV から吸光度 ABS を求め、ベース式（相関式）で物理量へ変換し、最後にスパン校正を適用する流れである。旧仕様にあった受光暗時オフセット（ADZR 減算）および機差補正 kiza は、現行実装では廃止・削除済みである。

（プローブ側：生 mV 正規化 — MLSS・SS・透視度 共通）

プローブは AD 値を mV に正規化して本体へ送信するのみであり、補正・物理量計算は行わない。正規化は原点を通る傾き（span）補正のみで、空中出力が `ADC_SPAN_INI` になるようにする。旧仕様の `ADC_Zero` オフセット減算は廃止された（`ADC_Zero`／`SADZ` はコマンドのみ残置・計算未使用）。

```
プローブ出力mV = 生mV * ADC_SPAN_INI / ADC_Span[ch]
生mV = digit * LSB
```

ch4（透視度）のみ、span 正規化の前に基準オフセット項が入る。

```
生mV_ch4 = digit * LSB_ch4 + ADC_CH4_VREF
プローブ出力mV_ch4 = 生mV_ch4 * ADC_SPAN_INI / ADC_Span[4]
```

- `プローブ出力mV`: 本体へ送信する正規化後の電圧値 (mV)。空中では概ね `ADC_SPAN_INI` になる。
- `生mV`: ADC のデジタル値 `digit` を `LSB` で電圧に変換した生の値 (mV)。
- `digit`: 外部 ADC（MCP3424）のデジタル出力値。
- `LSB`: 1 digit あたりの電圧。ch1〜4（PGA=1）は 0.0625 mV/digit、ch5（ch4, PGA=2）は 0.03125 mV/digit。
- `ADC_Span[ch]`: 各 ch の空中出力（＝空中の生 mV）を記録した傾き基準。設定コマンド `SADS`。初期値 `ADC_SPAN_INI`。
- `ADC_SPAN_INI`: 正規化ターゲット（空中出力の目標 mV）。1700.0（旧 1850／1750 から変更）。
- `ADC_CH4_VREF`: ch4（透視度）IN- の 1V 基準オフセット補正。1000.0 mV。

なお本体へ届く値は移動平均後の `ADC_mV_ave[ch]`（平均回数 `SADA`、既定 30）であり、瞬時値は `ADC_mV[ch]` である。

（本体側：吸光度 ABS の算出 — MLSS）

本体は正規化済みの受光 mV（Y0）と Ref mV から、Ref を温度プロキシとした 20℃換算の分子 `I(0)` を求め、その `I(0)` と Y0 の比の常用対数を吸光度 ABS とする。受光暗時オフセット（Y0 − ADZR）は廃止され、分母は Y0 そのものである。受光ゼロ基準は分子側の `MLSS_ZR`（清水中オフセット）が担う。

```
d = Ref - MLSS_Coef_ADZR[1]
I(0) = MLSS_ZR + d * (MLSS_Coef_TempC[0] + d * MLSS_Coef_TempC[1])
ABS = log10(I(0) / Y0)
```

- `Y0`: MLSS 受光の正規化済み mV（平均値 `ADC_mV_ave[0]`、瞬時値 `ADC_mV[0]`）。0 以下のときは 0.001 にクランプ。
- `Ref`: MLSS Ref の正規化済み mV（`ADC_mV_ave[1]`）。
- `MLSS_Coef_ADZR[1]`（refZR）: Ref 基準（20℃清水 Ref）。温度校正 `ATCF` が設定。未校正は 0。
- `d`: 20℃基準からの Ref 変化幅（`Ref − refZR`）。
- `MLSS_ZR`（MLSSZR）: 清水中ゼロ基準（分子の基準 mV）。初期値 `INI_MLSS_ZR = 1700.0`、清水ゼロ校正 `AZC` が実値で上書き。
- `MLSS_Coef_TempC[0]`（B）／`MLSS_Coef_TempC[1]`（B2）: Ref 温度補正の 1 次・2 次係数。未校正は 0。
- `I(0)`: 温度補正込みの分子。`I(0) ≤ 0` のときは安全のため `MLSS_ZR` に置換。
- `ABS`: 吸光度。温度補正が未校正（B=B2=0）なら `d*(…)=0` となり `I(0)=MLSS_ZR` に帰着する。清水中では `MLSS_ZR=Y0` となり `ABS=0`（MLSS=0）となる。

（本体側：吸光度 ABS の算出 — SS）

SS も MLSS と同一構造である。受光は `ADC_mV_ave[2]`、Ref は `ADC_mV_ave[3]` を用いる。

```
d = Ref - SS_Coef_ADZR[1]
I(0) = SS_ZR + d * (SS_Coef_TempC[0] + d * SS_Coef_TempC[1])
ABS = log10(I(0) / Y0)
```

- `Y0`: SS 受光の正規化済み mV（`ADC_mV_ave[2]`）。0 以下は 0.001 にクランプ。
- `Ref`: SS Ref の正規化済み mV（`ADC_mV_ave[3]`）。
- `SS_Coef_ADZR[1]`（refZR）: SS の Ref 基準（20℃清水 Ref）。
- `SS_ZR`（SSZR）: SS 清水中ゼロ基準。初期値 `INI_SS_ZR = 1700.0`。
- `SS_Coef_TempC[0]`（B）／`SS_Coef_TempC[1]`（B2）: SS の Ref 温度補正 1 次・2 次係数。
- `I(0)`: SS の温度補正込み分子。`I(0) ≤ 0` は `SS_ZR` に置換。

（本体側：吸光度 ABS の算出 — 透視度）

透視度は SS の Ref（`ADC_mV_ave[3]`）および SS の温度補正係数を流用する。分子の温度補正は 1 次項（B）のみで、2 次項は用いない。

```
d = Ref - SS_Coef_ADZR[1]
I(0) = TR_ZR + d * SS_Coef_TempC[0]
ABS = log10(I(0) / Y0)
```

- `Y0`: 透視度 受光の正規化済み mV（`ADC_mV_ave[4]`）。0 以下は 0.001 にクランプ。
- `Ref`: SS と共有する Ref mV（`ADC_mV_ave[3]`）。
- `SS_Coef_ADZR[1]`（refZR）: SS から流用する Ref 基準。
- `TR_ZR`（透視度ZR）: 透視度 清水中ゼロ基準。初期値 `INI_TR_ZR = 1700.0`。
- `SS_Coef_TempC[0]`（B）: SS から流用する温度補正 1 次係数。
- `I(0)`: 透視度の分子。`I(0) ≤ 0` は `TR_ZR` に置換。

（ベース式：ABS → 物理量 — MLSS）

MLSS は選択中の相関式 No. によりベース値（FABSS）を求める。No.1〜20 は共通の基準 2 次式（No.1 のテーブル `MLSS_Mode_CF[0]`）を傾きゲイン `k_No` で倍する。No.21〜30 は各スロット独立の 2 次式（ゲインは 1.00 固定）を用いる。基準式の評価 `eval_modecf` は係数枠 `modecf[7]`（最大 6 次）を持つが、現行の出荷時ベースは実質 2 次（3 次以上は 0）である。

```
eval_modecf(c, ABS) = c[0] + c[1]*ABS + c[2]*ABS^2 + c[3]*ABS^3 + c[4]*ABS^4 + c[5]*ABS^5 + c[6]*ABS^6
FABSS(No.1〜20) = eval_modecf(MLSS_Mode_CF[0], ABS) * k_No_MLSS[idx]
FABSS(No.21〜30) = eval_modecf(MLSS_Mode_CF[idx], ABS)
```

- `idx`: 選択中相関式のインデックス（`MLSS_MODE`）。0〜19 が No.1〜20、20〜29 が No.21〜30。
- `MLSS_Mode_CF[0]`: No.1 基準 2 次式の係数（No.1〜20 のベースカーブ）。
- `MLSS_Mode_CF[idx]`: No.21〜30 各スロットの独立 2 次式係数。
- `k_No_MLSS[idx]`: No.1〜20 の傾きゲイン。No.1=1.0000、以降等間隔（step ≈ 0.099364）で増加し No.20=2.8879。具体値は `{1.0000, 1.0994, 1.1987, 1.2981, 1.3975, 1.4968, 1.5962, 1.6955, 1.7949, 1.8943, 1.9936, 2.0930, 2.1924, 2.2917, 2.3911, 2.4905, 2.5898, 2.6892, 2.7886, 2.8879}`。
- `FABSS`: スパン校正適用前のベース MLSS 値（`MLSS_FABSS`）。

（ベース式：ABS → 物理量 — SS）

SS は相関式 No. を持たず、校正可能な単一式（No.21 相当、`SS_Mode_CF[SS_MODE]`、`SS_MODE` 初期値 20）のみを用いる。ゲイン倍は行わない。

```
FABSS = eval_modecf(SS_Mode_CF[SS_MODE], ABS)
```

- `SS_Mode_CF[SS_MODE]`: SS の単一 2 次式係数。
- `FABSS`: スパン校正適用前のベース SS 値（`SS_FABSS`）。

（ベース式：ABS → 物理量 — 透視度）

透視度の出荷時ベースは累乗式であり、係数 `a`（`TR_Coef_ModeCF[0]`）と `b`（`TR_Coef_ModeCF[1]`）で表す。`a=0`（未校正）の場合のみ旧多項式 `TR_Mode_CF[TR_MODE]` へフォールバックする。

```
FABSS = a * ABS^b            (a != 0 のとき)
FABSS = eval_modecf(TR_Mode_CF[TR_MODE], ABS)   (a == 0 のとき)
```

- `a`（`TR_Coef_ModeCF[0]`）: 累乗式の係数。
- `b`（`TR_Coef_ModeCF[1]`）: 累乗式の指数。
- `ABS`: 透視度の吸光度。`powf(負, 非整数)=NaN` を避けるため、`ABS < 1e-4` のときは 1e-4 にクランプする。
- `FABSS`: スパン校正適用前のベース透視度 [cm]（`TR_FABSS`）。

（スパン校正：FABSS → 最終出力 — MLSS・SS・透視度 共通形）

ベース値 FABSS に対し、現場 2 点／3 点校正で作成した 2 次のスパン校正式を適用して最終出力を得る。3 モードとも同一形（`A*f^2 + B*f + C`）である。

```
MLSS = MLSS_SP_A * FABSS^2 + MLSS_SP_B * FABSS + MLSS_SP_C
SS = SS_SP_A * FABSS^2 + SS_SP_B * FABSS + SS_SP_C
透視度 = TR_SP_A * FABSS^2 + TR_SP_B * FABSS + TR_SP_C
```

- `*_SP_A`／`*_SP_B`／`*_SP_C`: 各モードのスパン校正係数（2 次・1 次・定数）。初期値は `SP_A=0.0`、`SP_B=1.0`、`SP_C=0.0` であり、未校正時は出力 = FABSS となる。
- `FABSS`: 各モードのベース値（`MLSS_FABSS`／`SS_FABSS`／`TR_FABSS`）。
- `MLSS`／`SS`／`透視度`: 表示される最終物理量。透視度は cm 単位。

MLSS の界面バー表示に用いる瞬時値 `MLSS_inst` も同式で、瞬時 mV（`ADC_mV[0]`／`ADC_mV[1]`）から求めた瞬時 FABSS に対して算出する。

```
MLSS_inst = MLSS_SP_A * FABSS_inst^2 + MLSS_SP_B * FABSS_inst + MLSS_SP_C
```

- `FABSS_inst`: 瞬時受光・瞬時 Ref から求めたベース MLSS 値。
- `MLSS_inst`: 界面判断バー用の瞬時 MLSS 値（MLSS・界面）。

（界面：水深換算 — 界面）

水深は、電源 ON 直後の大気圧を原点（自動ゼロ）とした相対気圧に傾き `Depth_k` を掛けて算出する。オフセットは自動ゼロのため、校正は傾きのみの 1 点スロープ校正である。

```
Depth = (P - Depth_offset) * Depth_k
```

- `P`: 現在の圧力値（プローブ気圧センサ由来、本体 `ADC_mV[5]`）。
- `Depth_offset`: 電源 ON 直後の初回計測で取得した大気圧（原点）。校正画面での界面ゼロ校正でも現在値へ上書き可能。
- `Depth_k`: 水深換算の傾き [m/hPa 相当]。初期値 `1.0 / 100.0`（現行 100hPa/m 相当）。
- `Depth`: 算出された水深 [m]。負値は 0.0 にクランプする。

傾きの 1 点スロープ校正（6m 相当の加圧時に 6.0m 表示となるよう調整）は、電源 ON 大気圧を原点とした相対気圧 `dp` に対して次式で `Depth_k` を更新する（`dp ≤ 0` のときは校正失敗）。

```
dp = P - Depth_offset
Depth_k = 6.0 / dp
```

- `dp`: 電源 ON 大気圧を原点とした相対気圧。
- `Depth_k`: 6m 相当加圧時に `Depth=6.0` となるよう更新される傾き（界面）。
