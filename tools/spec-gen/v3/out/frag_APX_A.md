```mlss_formulas.md```

## 1. プローブ出力の生mV正規化

```
プローブ出力 mV = 生mV(digit×LSB) − ADZero
```

- `プローブ出力 mV`: プローブから出力される電圧値 (mV)
- `生mV(digit×LSB)`: ADCのデジタル値をLSB単位で変換した生の電圧値
- `ADZero`: 空中出力を1750mVにオフセットする値

## 2. 本体側のABS計算

```
adc_mv = Y0 − MLSS_ADZR                         (≤0 は 0.001 にクランプ)
d      = Ref − MLSS_Vault_ADZR[1]   (= Ref − refZR)
I(0)   = MLSS_ZR + d × ( TempC[0] + d × TempC[1] )   (I0≤0 は MLSS_ZR)
ABS    = log10( I(0) / adc_mv )
```

- `adc_mv`: 受光生mVからADZeroを引いた値
- `Y0`: MLSS受光の生mV
- `MLSS_ADZR`: 受光暗時ゼロ
- `d`: Refの生mVからrefZRを引いた値
- `Ref`: MLSS_Refの生mV
- `MLSS_Vault_ADZR[1]`: Ref基準（20℃清水Ref）
- `I(0)`: 温度補正後の受光値
- `MLSS_ZR`: ゼロ基準（清水/空中の分子基準mV）
- `TempC[0]`: Ref温度補正傾きB
- `TempC[1]`: Ref温度補正傾きB2
- `ABS`: 吸光度

## 3. FABSS計算（MLSS）

```
FABSS = Σ(k=0..6) MLSS_Mode_CF[MLSS_MODE][k] × ABS^k
```

- `FABSS`: 1次演算後の値
- `MLSS_Mode_CF[MLSS_MODE][k]`: 相関式の係数（機体・相関式ごとに異なる）
- `ABS`: 吸光度

## 4. スパン校正（MLSS）

```
MLSS = MLSS_SP_A × FABSS² + MLSS_SP_B × FABSS + MLSS_SP_C
```

- `MLSS`: 最終的な測定値 (mg/L)
- `MLSS_SP_A`, `MLSS_SP_B`, `MLSS_SP_C`: スパン校正係数（機体・相関式ごとに異なる）

## 5. TR(透視度)のカーブ形

### 出荷時ベース
```
cm₀ = a·ABS^b
```

- `cm₀`: 累乗式による初期値 (cm)
- `a`, `b`: 累乗フィット係数（機体・相関式ごとに異なる）

### 2次補正
```
cm = A·cm₀² + B·cm₀ + C
```

- `cm`: 最終的な透視度 (cm)
- `A`, `B`, `C`: 2次補正係数（機体・相関式ごとに異なる）

## 6. Ref温度補正

```
I(0) = MLSSZR + (現ref − refZR)·(B + (現ref − refZR)·B2)
```

- `I(0)`: 温度補正後の受光値
- `MLSSZR`: ゼロ基準（清水/空中の分子基準mV）
- `現ref`: 現在のRefの生mV
- `refZR`: Ref基準（20℃清水Ref）
- `B`, `B2`: Ref温度補正係数

## 7. 水深補正

```
Depth[m] = (P − P_atm0) × k_depth
```

- `Depth[m]`: 水深 (m)
- `P`: プローブの現在気圧 (hPa)
- `P_atm0`: 電源ON直後の大気圧 (hPa)
- `k_depth`: 傾き（機体・相関式ごとに異なる）

## 8. 各変数の説明

### プローブ出力の生mV正規化
- `プローブ出力 mV`: プローブから出力される電圧値 (mV)
- `生mV(digit×LSB)`: ADCのデジタル値をLSB単位で変換した生の電圧値
- `ADZero`: 空中出力を1750mVにオフセットする値

### 本体側のABS計算
- `adc_mv`: 受光生mVからADZeroを引いた値
- `Y0`: MLSS受光の生mV
- `MLSS_ADZR`: 受光暗時ゼロ
- `d`: Refの生mVからrefZRを引いた値
- `Ref`: MLSS_Refの生mV
- `MLSS_Vault_ADZR[1]`: Ref基準（20℃清水Ref）
- `I(0)`: 温度補正後の受光値
- `MLSS_ZR`: ゼロ基準（清水/空中の分子基準mV）
- `TempC[0]`: Ref温度補正傾きB
- `TempC[1]`: Ref温度補正傾きB2
- `ABS`: 吸光度

### FABSS計算（MLSS）
- `FABSS`: 1次演算後の値
- `MLSS_Mode_CF[MLSS_MODE][k]`: 相関式の係数（機体・相関式ごとに異なる）
- `ABS`: 吸光度

### スパン校正（MLSS）
- `MLSS`: 最終的な測定値 (mg/L)
- `MLSS_SP_A`, `MLSS_SP_B`, `MLSS_SP_C`: スパン校正係数（機体・相関式ごとに異なる）

### TR(透視度)のカーブ形
#### 出荷時ベース
- `cm₀`: 累乗式による初期値 (cm)
- `a`, `b`: 累乗フィット係数（機体・相関式ごとに異なる）

#### 2次補正
- `cm`: 最終的な透視度 (cm)
- `A`, `B`, `C`: 2次補正係数（機体・相関式ごとに異なる）

### Ref温度補正
- `I(0)`: 温度補正後の受光値
- `MLSSZR`: ゼロ基準（清水/空中の分子基準mV）
- `現ref`: 現在のRefの生mV
- `refZR`: Ref基準（20℃清水Ref）
- `B`, `B2`: Ref温度補正係数

### 水深補正
- `Depth[m]`: 水深 (m)
- `P`: プローブの現在気圧 (hPa)
- `P_atm0`: 電源ON直後の大気圧 (hPa)
- `k_depth`: 傾き（機体・相関式ごとに異なる）