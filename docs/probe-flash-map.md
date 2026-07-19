# プローブ内蔵フラッシュ パラメータ一覧 (STM32G070 / IM-110T プローブ基板)

> 目的: プローブ側フラッシュに読み書きされるパラメータを漏れなく列挙した一次参照。
> 「電源ON直後のフラッシュR/Wを切り分けで無効化し初期値で動かす」作業(2026-07-19)の
> エンバグ防止のための事前記録。実体コードは `IM-110_Probe/Core/Src/IM_110.c` / `main.c`。

## 0. フラッシュ領域マップ (STM32G070KBT6, Flash 128kB / Page=2KB)

| 領域 | アドレス | ページ | サイズ | 用途 | 読出 | 書込 |
|---|---|---|---|---|---|---|
| **PARAM** | `PARAM_AD = 0x0801F300` | Page62 | 8×32B=256B(枠) | 機体パラメータ(製品名/ID/ADC校正/LED) | `read_Param()`→`Set_Flash2Param()` | `Set_Param2Flash()`→`write_Param()` |
| **VAULT** | `VAULT_AD = 0x0801F800` | Page63 | 1408B(ver2) | 校正係数(相関式/モード別、真実源) | `vault_load()` | `vault_commit()` |

- 共有プリミティブ `flash_erase_program(addr, src, bytes)` (IM_110.c:853) が両領域の**唯一の書込経路**。
  ページ消去(2KB)+DOUBLEWORD(8B)書込。**書込中は `HAL_UART_AbortReceive` + `__disable_irq()`**(ORE予防, IM_110.c:862-886)。
- **読出はメモリマップド**(`memcpy` / 直接参照)。IRQ禁止もRX停止も伴わない。
  → **電源ON直後(起動時)はフラッシュ「読出」のみ。書込は校正確定時(WCFC/WPP)にのみ発生。**

---

## 1. PARAM 領域 (Page62) — 機体パラメータ

格納/復元: `Set_Param2Flash()` (IM_110.c:973) / `Set_Flash2Param()` (IM_110.c:1014)
初期値設定: `Set_Param_INI()` (IM_110.c:904) ※RAMのみ、フラッシュ非アクセス

| 変数 | 型 | 個数 | Flash_Data行 | 初期値マクロ | 初期値 |
|---|---|---|---|---|---|
| `Product_Name[7]` | uint8_t | 7B | 行0 offset0 | `PRO_NAME` | `"IM-110 "` |
| `Probe_ID` | uint32_t | 1 | 行0 offset7 | — | `0xFFFFFFFF` |
| `ADC_Zero[5]` | float | 5 | 行1 | `ADC_ZERO_INI` | `0.0f` |
| `ADC_Span[5]` | float | 5 | 行2 | `ADC_SPAN_INI` | `1850.0f` |
| `LED_Out[5]` | float | 5 | 行3 | `LED_OUT_INI` | `0.36f` |
| `ADC_Span_S[5]` | float | 5 | 行4 | `ADC_SPAN_S_INI` | `1850.0f` |

- 新品判定: 起動時 `memcmp(Product_Name, PRO_NAME) != 0` なら `Set_Param_INI()` で再初期化 (main.c:175)。
- `LED_Out[0]` は起動時 `Set_PWM_Duty()` でPWMデューティへ反映 (main.c:178)。

---

## 2. VAULT 領域 (Page63) — 校正係数 (protocol §3.2.6, ver2=1408B)

構造体 `probe_vault_t` (IM_110.c:1088)。真実源=プローブ、本体=キャッシュ。
本体が `RCF` で読み出し、校正確定時に `WCF/WCS/WCFC` 等で書き戻す。

- ヘッダ: `magic`(`VAULT_MAGIC=0x494D3243 'IM2C'`), `ver`(`VAULT_VER=2`), `checksum`(バイト総和)
- **per-eq** `eq_coef_t` (15float=60B) × [MLSS No.21-30(10)] + [SS No.21-30(10)] + [TR(1)]:
  - `cf[7]` … ZR, FABS_SPAN, SP_A, SP_B, SP_C, 1A, 1B (RCF: `CF`)
  - `modecf[7]` … 相関式 Mode_CF 多項式係数 最大6次 (RCF: `CM`)
  - `kiza` … 機差補正 倍率 (RCF: `CK`)
- **per-mode** `mode_coef_t` (11float=44B) × [MLSS, SS, TR]:
  - `setval[2]` … 校正点濃度 SetVal_1/2 (RCF: `CS`)
  - `adzr[2]` … ゼロ点 [0]=受光ADZR, [1]=RefADZR (RCF: `CZ`, TRは未使用=SS参照)
  - `tempc[7]` … 温度補正 (RCF: `CT`, TRは未使用=SS参照)

初期値(未校正/新品): `vault_load()` の無効判定時 `memset(&g_vault,0,...)` + `g_vault_valid=0`。
→ この状態では `RCF` は **`NG`** を返し、**本体は EEPROM キャッシュへフォールバック**する。

主要定数: `CF_EQ_BASE=21`, `CF_EQ_COUNT=10`, `CF_NFLOAT=7`, `CF_MODECF_N=7`, `CF_TEMPC_N=7`。

---

## 3. 起動時(電源ON直後)のフラッシュアクセス — 切り分け対象

`main.c` USER CODE BEGIN 2 (main.c:172-179):

```
Set_Param_INI();     // (1) パラメータ変数を初期値に (RAMのみ, flash非アクセス)
read_Param();        // (2) PARAM_AD → Flash_Data[][]  ★flash読出
Set_Flash2Param();   // (3) Flash_Data → 各変数        (RAMコピー、(1)を上書き)
if (memcmp(Product_Name, PRO_NAME,...) != 0) Set_Param_INI();  // (4) 新品なら再初期化
Set_PWM_Duty(LED_Out[0]);  // (5) LEDデューティ反映
vault_load();        // (6) VAULT_AD → g_vault         ★flash読出
```

### 切り分け措置 (2026-07-19, 一時) — 実装済み【本体側のみ / プローブ無改変】

**方針**: 疑うのは **VAULT 領域(第2領域, 係数のプローブ格納, Ver.0.14〜)** と、それを触る
**本体側の通信**。**第1領域(PARAM)は Ver.0.12 から正常動作しているため対象外**。
プローブ側コード/フラッシュは **一切改変しない**。

**本体側** `IM-110/Core/Src/IM_110.c` PB_HS_VAULT ハンドシェイク:
- 起動時の **`Probe_ReadVault()`(RCF読出) と `Probe_MigrateToVault()`(vault書込) を両方コメントアウト**。
- 本体は **EEPROMキャッシュ校正を継続使用**(pre-vault=Ver.0.12 相当の動作に近づく)。
- → 本体はプローブ VAULT に起動時アクセスしない。プローブは通常どおり起動(vault_load はプローブ内で
  走るが、本体が RCF を投げないため通信としては無関係)。

**ビルド**: 本体クリーン成功 (2026-07-19)。`build/IM-110.{hex,bin}` (v4.0.2)。プローブは再ビルド不要。

**位置づけ**: protocol変更ではない(仕様据え置き)。コメント復帰で即戻せる。
安定確認できたら復帰し、VAULT アクセスと本体成否判定の恒久整合を設計する。

### 補足: 起動以外の本体側 VAULT アクセス箇所 (今回は未変更)
起動時だけ切っても、運用中に触る経路が残っている。切り分けで残存するなら次はここも対象:
- `Normal.c:730` 相関式切替時に `Probe_ReadVault()`(RCF読出→EEPROMキャッシュ同期)
- `Normal.c:1442 / 1678` 後校正確定時に vault へ書戻し(`WCF/WCS/WCFC`=プローブflash書込)
- `LinkSerial.c:265` 手動マイグレーション `Probe_MigrateToVault()`

### 検証手順 (本体を書込後)
1. **本体のみ**書込(プローブは現行のまま)→ 電源再投入を**複数回**繰り返す。
2. 観察: 係数読み/計算値/起動タイトル表示が**毎回安定**するか(症状が変わらないか)。
3. 判定:
   - 安定した → **本体⇔プローブの VAULT 通信が不安定の原因**。恒久対策(タイミング整合 or 構造見直し)へ。
   - 変わらず不安定 → 原因は起動時 VAULT 通信以外。運用中 VAULT アクセス(上記補足)や UART 物理層へ切り分けを進める。

### 検証結果 (2026-07-19) — 不安定は消滅。原因=VAULT通信で確定
openocd で走行中 RAM を電源断なく読取(Probe_Conn_Status/Data_Valid/MS_started @0x200010d9、ADC_mV @0x200015e0)。
**電源再投入 5回連続**、全て:
- Conn=1 / DataValid=1 / MS=1(通信成立)
- ADC_mV[0..2] ≈ 1658 / 1021 / 1288 mV で安定(ch2 に一度 +11 のドリフトのみ、症状変化なし)

→ 起動分離で「起動間延び」も解消済み。**「電源再投入ごとに症状が変わる」不安定は再現せず。**
本体⇔プローブ VAULT 通信(プローブflashアクセス timing × 本体の成否判定 timing の噛み合わせ)が
不安定の元凶と確定。

### 恒久対策の選択肢 (次セッションで決定)
現状は「本体側 VAULT アクセスを起動時だけ無効化+EEPROMキャッシュ運用」の暫定。恒久化は2択:
- **(1) VAULT 通信を堅牢化して再有効化** — プローブ=真実源を維持。書込中flash-busy窓をまたぐ
  ACK/完了通知/リトライ整合、運用中アクセス(Normal.c:730/1442/1678)も同様に手当て。機能は残る。
- **(2) 係数の真実源を本体EEPROMに固定(VAULT 恒久オフ)** — 単純・確実。プローブが自前校正を運ぶ利点は失う。
  FUP透過ブリッジ等はそのまま。
※注意: 現在は係数をプローブから読まずキャッシュ/デフォルトで計算しているため、MLSS≈1821 等の
  **値の正しさは別軸**。恒久方針決定後に係数供給を正して確認する。
