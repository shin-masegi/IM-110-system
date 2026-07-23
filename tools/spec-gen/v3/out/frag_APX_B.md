本付録は、IM-110 本体基板が搭載する EEPROM（M95256、容量 32KB、SPI 接続）のページ割付を示すものである。運用単位は 1 ページ = 32 バイト（`EEP_PAGE_BYTE = 32`）であり、各ページの末尾 1 バイト（オフセット 31）は XOR チェックサムに充てられる。したがって 1 ページで実データとして扱えるのは先頭 31 バイトである。

ハードウェア上は 1024 ページが存在するが、実運用で使用する範囲は page 0 から page 125 までであり、`EEP_PAGE_MAX = 126` で範囲を絞り込んでいる。これは製造時における EEPROM 初回クリア時間を短縮するための措置であり、データロガー機能を実装する段階で `EEP_PAGE_MAX` を page 126 以降へ再拡張する想定である。

起動時には page 0 の先頭 `PRO_SIZE` バイトが `PRO_NAME`（"IM-110"）と一致するか検査され、不一致の場合は page 0 から page 125 までを初期値で再構築する。page 126 以降には触れない。

以下に、現行ヘッダ（`Core/Inc/Eeprom.h` レイアウト v2.1、2026-06-01 確定）に定義されるページ割付を示す。

| page 番号 | 領域名（マクロ） | 用途 | 備考 |
|---|---|---|---|
| 0 | `EEP_HEADER_PAGE` | "IM-110" ヘッダ + 予備 | 起動時整合性チェックの基準（`PRO_NAME` = "IM-110"） |
| 1-3 | `EEP_BOARDADJ_PAGE`（先頭 = 1） | 本体基板調整 予備領域 | 将来拡張枠。先頭ページのみマクロで定義される |
| 4 | `EEP_INFO_PAGE` | エラー情報・本体情報 | |
| 5 | `EEP_INTERFACE_PAGE` | IM-110 界面測定設定 | `Interface_Threshold`（float）+ 予備。マジックバイト `EEP_INTERFACE_MAGIC = 0x3C` |
| 6-11 | （マクロ定義なし） | IM-110 共通設定 拡張用 予備（上流側） | 共通設定を追加する際に消費 |
| 12 | `EEP_COMMON_PAGE` | IM-110 共通設定 | `Meas_Mode`（1B）、`MLSS_MODE`/`SS_MODE`/`TR_MODE`（uint8 ×3、相関式内部値 0..29）、`MLSS/SS/TR_Cal_SetVal_1/2`（float ×6）。マジックバイト `EEP_COMMON_MAGIC = 0xA5` |
| 13-18 | （マクロ定義なし） | IM-110 共通設定 拡張用 予備（下流側） | 共通設定を追加する際に消費 |
| 19-38 | `EEP_MLSS_CF_PAGE`（先頭 = 19） | MLSS 校正係数 No.21-30 | 2 page × 10 相関式。`EEP_CF_STRIDE = 2`、`EEP_CF_COUNT = 10`、`EEP_CF_BASE_IDX = 20`。マジックバイト `EEP_CF_MAGIC = 0x5A` |
| 39-58 | `EEP_SS_CF_PAGE`（先頭 = 39） | SS 校正係数 No.21-30 | 2 page × 10 相関式 |
| 59-60 | `EEP_TR_CF_PAGE`（先頭 = 59） | TR 校正係数（相関式無し） | 2 page × 1。`EEP_TR_CF_COUNT = 1`。TR は相関式選択が無く 1 セットのみ |
| 61-76 | `EEP_MIRROR_PAGE`（先頭 = 61） | 統合ストア ミラー（probe_store_t の本体 EEPROM ミラー） | `EEP_MIRROR_COUNT = 16` = `STORE_NPAGE`。1 page = 32B × 16 = 512B で probe_store_t（512B）に一致し、§12 統合ストアとバイト同一。予備領域 page 61-80 内の先頭 16 ページを占有 |
| 77-80 | （マクロ定義なし） | 校正係数 拡張用 予備 の残り | ヘッダのレイアウトコメントは page 61-80 の 20 page を「校正係数 拡張用 予備」と記載しており、その先頭 16 page をミラーが占有した残り 4 page に相当する（要確認） |
| 81-95 | `EEP_HIS_MLSS_PAGE`（先頭 = 81） | MLSS/界面 測定履歴 | 15 page（`EEP_HIS_COUNT = 15`）、30 件 ÷ 2 件/page。界面測定（Meas_Mode=3）は値が MLSS と被るため MLSS バンクを共用 |
| 96-110 | `EEP_HIS_SS_PAGE`（先頭 = 96） | SS 測定履歴 | 15 page、30 件 |
| 111-125 | `EEP_HIS_TR_PAGE`（先頭 = 111） | 透視度 測定履歴 | 15 page、30 件。`EEP_HIS_PAGE` は旧 API 互換の既定値として MLSS バンク先頭（= 81）を指す |
| 126-1023 | （範囲外、`EEP_PAGE_MAX = 126`） | 未使用・予約 | EEPROM クリア対象外。将来データロガー（MLSS + 水深 ×2000 件 ×3 種）用に予約。ロガー実装時に `EEP_PAGE_MAX` を拡張 |

統合ストア ミラー（page 61-76）について補足する。この領域はプローブ基板が保持する `probe_store_t`（512 バイト）を本体 EEPROM 側へバイト単位でそのまま写した鏡像であり、§12 で規定する統合ストアと内容がバイト同一である。1 ページ 32 バイト × 16 ページ（`EEP_MIRROR_COUNT = 16`、probe_store.h の `STORE_NPAGE` に一致）でちょうど 512 バイトを収める。旧来の校正係数領域（page 19-60）は、統合ストアへの移行（migration）完了後は legacy 扱いとなる。

校正係数の 1 相関式あたりのページ配置は 2 ページ（64B 枠）である。先頭ページ（page A）の offset 0 に未書込判定用マジック `EEP_CF_MAGIC`（0x5A）を置き、続いて `ZR`（ゼロ点校正係数 mV）、`FABS_SPAN`（器差校正係数）、`SP_A`/`SP_B`/`SP_C`（スパン係数 y=Ax^2+Bx+C）、`1A`（フルスパン基準）、`1B`（1/2 スパン基準）の float 7 個を格納し、offset 29-30 は予備（2B）、offset 31 が XOR チェックサムである。2 ページ目（page B）は ADZR 等を将来追加するための予備である。

各測定履歴レコードは 15 バイトであり、year/month/day/hour/min（各 uint8）、value（float、測定値）、depth（float、水深 m）、raw_mv_x10（int16、透過光電圧 0.1mV 単位、後校正の逆算用）で構成される。1 ページに 2 件（offset 0..14 と 15..29）を格納し、offset 31 に XOR チェックサムを置く。

なお、相関式と校正の関係については、MLSS / SS は相関式 No.1-30（内部値 0-29）を持ち、No.1-20 は固定係数で校正不可、No.21-30 のみ校正可能で EEPROM に係数を保存する。TR（透視度）は相関式選択が無く、校正係数は 1 セットのみで page 59-60 を直接読み書きする。
