# IM-110 本体 基板調整UART (USART1 / CN2) コマンド仕様

> 本資料は **本体基板の基板調整・検査用UART**（USART1、外部コネクタ CN2）のコマンド仕様である。
> 本体⇔プローブ間の計測用 RS-232C（LPUART1）とは**別系統**であり、そちらは
> [`protocol-rs232c.md`](protocol-rs232c.md) を参照。
>
> **真実源**: 本資料の記述は本体FW `IM-110/Core/Src/LinkSerial.c`（コマンドパーサ実装）を
> 一次資料として検証した。各行は `LinkSerial.c:<行>` で根拠を示す。相違があれば実コードを優先し、
> 本資料を更新すること。

---

## 1. 物理層・接続

| 項目 | 内容 |
|---|---|
| MCU ペリフェラル | **USART1**（本体 STM32L452、TX=U4.42 / RX=U4.43） |
| 外部コネクタ | **CN2 (CON6)**。USART1 TXD/RXD/CTS/RTS と **NTEST** を R13/R15/R16/R17/R18 経由で引き出し |
| レベル | **TTL 3.3V**（USART1 直結。MAX3232 は通さない）。接続する USB シリアルは **TTL 3.3V 品**を用いる |
| ボーレート | 9600 bps / 8bit / パリティなし / ストップ 1 / フロー制御なし（**9600 8N1**） |
| Mac 側デバイス | `/dev/cu.usbserial-0001` 等（受信は `cu.*` を使う） |

> **CN2 は基板調整コネクタを兼ねる**: CN2 の **NTEST** 線が MCU の `PC4_nTEST_in` に繋がり、
> これを LOW にして電源 ON すると `operation_mode = ADBOAD`（基板調整モード）で起動する
> （`main.c` 起動モード判定。§3 参照）。すなわち調整ケーブルを CN2 に挿す = UART 接続 + ADBOAD 起動。

## 2. フレーム構造

- **行終端**: **LF (`0x0A`)** でコマンド確定（`LinkSerial.c:180-184`）。CR (`0x0D`) は必須でなく、
  判定は先頭一致のため余分な後続文字は無視される。
- **大文字小文字**: `strncmp` による**先頭一致**で**大文字小文字を区別**する。小文字は不一致 → `NG\n`。
- **引数**: `,`（カンマ）区切り。`Adj_arg_f()` が `strchr(RecvData, ',')` 以降を `strtof` で数値化（`LinkSerial.c:1933-1943`）。
- **受信 → 実行**: USART1 の 1 バイト受信を `RecvData[]` に蓄積（`HAL_UART_RxCpltCallback`, `LinkSerial.c:132-192`）、
  LF 受信で `uart_end=1` → メインループ `main.c:506-513` が `CheckSerialCMD()`（`LinkSerial.c:234-589`）を呼ぶ。
- **送信**: `SendSerialData()` → `HAL_UART_Transmit(&huart1, ...)`（`LinkSerial.c:223-226`）。

## 3. ゲート条件（重要）

- コマンド実行のゲートは **`use_UART_flag` のみ**。`operation_mode`（ADBOAD 等）ではゲートしていない。
- `use_UART_flag` は `main()` 冒頭で**無条件に `1`**（`main.c:236`）にされ、以降どのモードでも `0` に戻す
  コードは存在しない（確認済み）。ADBOAD 分岐での `=1`（`main.c:277`）は実質冗長。
- **結論**: UART 調整コマンドは ADBOAD に限らず **NORMAL / 校正 / 設定 を含む全モードで受理**される。
  ADBOAD は「調整ケーブル(CN2 NTEST=LOW)接続時に入る正規の調整モード」だが、コマンド受理の必要条件ではない。

## 4. コマンド一覧

### 4.1 調整機能 A系（`Handle_Adjust_CMD`, `LinkSerial.c:1953-2098`）

`CheckSerialCMD` 冒頭で `RecvData[0]=='A'` の場合に先に委譲される（`LinkSerial.c:265-268`）。
接頭辞衝突回避のため長い接頭辞から判定（ALDA→ALD, ATCF→ATC, AMCF/AMCP→AMC。`LinkSerial.c:1950-1951`）。

| コマンド | 引数 | 機能 | 応答例 | 根拠 |
|---|---|---|---|---|
| `AST` | なし | ステータス表示 | `AST,mode:%d,eq:%d,cf_pts:%d,tc:0x%X\n` | 2088 |
| `AMV` | なし | 受光/Ref mV・ABS・最終値の生値表示 | `AMV,%d,J:%.2f,R:%.2f,ABS:%.5f,F:%.4f,V:%.3f\n` | 1959 |
| `AMODE` | `,<0/1/2>` | 測定モード設定（0:MLSS / 1:SS / 2:TR透視度） | `OK\n`/`NG\n` | 1969 |
| `AEQ` | `,<21-30>` | 相関式 No. 選択（MLSSのみ） | `OK\n`/`NG\n` | 1977 |
| `ALDA` | `,<start_duty>` | LED duty 自動調整（1750mV 収束 + WPP 保存） | `ALDA,OK/NOCONV/ERR,duty:%.4f,max:%.1f,it:%d\n` | 1985 |
| `ALD` | `,<duty>` | LED duty 手動設定（揮発） | `OK\n`/`NG\n` | 1997 |
| `AZR` | なし | ADZR 捕捉（遮光ダーク → vault CZ） | `OK\n`/`NG\n` | 2005 |
| `ATCF` | なし | 温度補正 3 点フィット | `OK\n`/`NG\n` | 2011 |
| `ATC` | `,<temp>` | 温度補正点捕捉（5/20/35℃） | `OK\n`/`NG\n` | 2017 |
| `AMCF` | `,<degree>` | Mode_CF 多項式フィット | `AMCF,OK,R2:%.5f\n`/`NG\n` | 2025 |
| `AMCP` | なし | Mode_CF 累乗フィット（透視度） | `AMCP,OK,R2:%.5f\n`/`NG\n` | 2036 |
| `AMD` | なし | 捕捉点ダンプ | `AMD,%d,ABS:%.5f,Y:%.3f\n` … `AMD,END,%d\n` | 2045 |
| `AMC` | `,<基準器 mgL/cm>` | Mode_CF 点捕捉 | `OK\n`/`NG\n` | 2059 |
| `AKZ` | `,<基準器 mgL>` | 機差補正（1 点比例 → CK） | `OK\n`/`NG\n` | 2067 |
| `AWC` | なし | 確定（RAM 像 → flash, WCFC 相当） | `OK\n`/`NG\n` | 2075 |
| `AMR` | なし | 捕捉バッファリセット | `OK\n` | 2081 |

### 4.2 プローブ中継 / 主要レガシー系（`CheckSerialCMD`, `LinkSerial.c`）

| コマンド | 機能 | 応答 | 根拠 |
|---|---|---|---|
| `PON` | プローブパススルー ON | `P:ON\n` | 237 |
| `POF` | プローブパススルー OFF | `P:OFF\n` | 243 |
| `P:<cmd>` | `P:` を外し **LPUART1（プローブ）へ転送**（例 `P:VR` `P:MD`） | `P:OK\n`/`P:DISABLED\n` | 271 |
| `MIGV` | EEPROM 校正係数 → プローブ vault 手動移行 | `MIGV:OK\n`/`MIGV:NG\n` | 252 |
| `VR` | 本体 FW バージョン | `VR,%d.%02d\r\n` | 293 / 761-767 |
| `BVR` | 電池電圧 | `BVR,%.2f\r\n` | 556 / 1869-1876 |
| `OFF` | 電源 OFF 要求 | （応答なし） | 286 |
| `04M` / `16M` | システムクロック 4/16MHz | `4MHz\n` / `16MHz\n` | 512 / 519 |
| `AD` / `SAD` | 測定値・アナログ値取得 | — | 341 / 544 |
| `RAD` | 基板調整値取得 | — | 347 |
| `REP` / `WEP` / `IEP` / `SIP` | EEPROM ページ 読/書/初期設定/全初期化 | — | 323/329/335/360 |
| `ZR` / `SP` / `ATP` | 基板 ゼロ/スパン/水温 調整係数設定 | — | 402/408/414 |
| `WAD` | 全基板調整パラメータ書込 | — | 366 |
| `TR` / `TW` / `RRS` / `WRS` | RTC 日時 読/書 / ステータス 読/リセット | — | 305/317/299/311 |
| `RCD` / `CCH` | 校正履歴 取得/消去 | `OK\n`(CCH) | 574/580 |
| `EPT` | EEPROM テスト開始 | （別途結果送信） | 562 |

- 未該当の受信行は `NG\n`（`LinkSerial.c:588`）。
- 1Wire 系（`W1TS`/`R1PG`/`R1HD` 等, `LinkSerial.c:431-510`）は `#if WIRE_ENABLE` ビルド時のみ有効。
- 上表は主要コマンド抜粋。全コマンドの網羅と各レガシー応答書式の確定は `LinkSerial.c` を随時参照・追記のこと。

## 5. 応答フォーマットの注意

- A系の応答は `\n` 終端、`VR`/`BVR` 等一部レガシーは `\r\n` 終端で**不統一**。
- 汎用成功/失敗は `OK\n` / `NG\n`。

## 6. 疎通テスト手順（デバッグ）

1. TTL 3.3V USB シリアルを CN2（USART1 TXD/RXD/GND）に接続。9600 8N1。
2. 受信を開始（例: 用意した `tools/... /uart_listen.py` 相当の read-only リスナ）。
3. `AST\n` または `VR\n` を送信 → ステータス / バージョンが返れば疎通 OK。
4. `AMV\n` で受光・Ref・ABS の生値を確認。`P:VR\n` で本体経由プローブの応答も確認可能。

## 7. 変更履歴
- 2026-07-19: 初版。`LinkSerial.c` 実装調査に基づき作成。
