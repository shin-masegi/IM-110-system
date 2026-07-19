/**
 * @file Eeprom.h
 * @brief EEPROM制御関連ヘッダーファイル
 * @author 飯島電子 柵木
 * @date 2018/07/25 (initial), 2026/05/13 (IM-110 layout v2.0), 2026/06/01 (v2.1 履歴3種分離)
 *
 * ============================================================================
 * IM-110 EEPROM レイアウト v2.1  (単一真実源, 2026-06-01 確定)
 *  デバイス  : M95256 (32KB, SPI、HW 上は 1024 ページ存在)
 *  運用単位  : 1 ページ = 32B (うち末尾 1B は XOR チェックサム)
 *  使用範囲  : page 0..125 (EEP_PAGE_MAX = 126)
 *              ※製造時の EEPROM 初回クリア時間短縮のため、ロガー領域を
 *                確保せず使用ページ数を絞り込んでいる。ロガー機能を実装する
 *                段階で EEP_PAGE_MAX を page 126 以降まで再拡張する。
 *
 *  起動時の整合性チェック: page 0 先頭 PRO_SIZE バイトが PRO_NAME ("IM-110")
 *  と一致しなければ、page 0-125 を初期値で再構築する (page 126 以降は触らない)。
 *
 * ----------------------------------------------------------------------------
 *  page 0           "IM-110" ヘッダ + 予備
 *  page 1-3         本体基板調整 予備領域 (将来拡張枠)
 *  page 4           エラー情報・本体情報         (EEP_INFO_PAGE)
 *  page 5           IM-110 界面測定設定          (EEP_INTERFACE_PAGE)
 *                     Interface_Threshold (float) + 予備
 *  page 6-11        IM-110 共通設定 拡張用 予備 (上流側、共通設定追加時に消費)
 *  page 12          IM-110 共通設定              (EEP_COMMON_PAGE)
 *                     Meas_Mode (1B)
 *                     MLSS_MODE / SS_MODE / TR_MODE (uint8 ×3、相関式内部値 0..29)
 *                     MLSS/SS/TR_Cal_SetVal_1/2 (float ×6)
 *  page 13-18       IM-110 共通設定 拡張用 予備 (下流側、共通設定追加時に消費)
 *
 *  page 19-38       MLSS 校正係数 No.21-30        (EEP_MLSS_CF_PAGE, 2 page × 10)
 *  page 39-58       SS   校正係数 No.21-30        (EEP_SS_CF_PAGE,   2 page × 10)
 *  page 59-60       TR   校正係数 (相関式無し)    (EEP_TR_CF_PAGE,   2 page × 1)
 *  page 61-80       校正係数 拡張用 予備 (20 page)
 *
 *  --- 測定履歴 30件 × 3種別 (各 15 page、計 45 page) ----------------------
 *  page 81-95       MLSS/界面 測定履歴 (EEP_HIS_MLSS_PAGE, 15 page)
 *  page 96-110      SS       測定履歴 (EEP_HIS_SS_PAGE,   15 page)
 *  page 111-125     透視度    測定履歴 (EEP_HIS_TR_PAGE,   15 page)
 *                     ※界面測定 (Meas_Mode=3) は値が MLSS と被るため
 *                       MLSS バンク (page 81-95) を共用する
 *
 *  page 126-1023    未使用 (EEP_PAGE_MAX = 126 で範囲外、EEPROM クリア対象外)
 *                     将来データロガー (MLSS+水深 ×2000件 ×3種) 用に予約。
 *                     ロガー実装時に EEP_PAGE_MAX を拡張する。
 *
 * ----------------------------------------------------------------------------
 *  各測定履歴レコード (15B、2件 + 末尾 CS で 1 page = 31B 使用):
 *    offset 0       year  (uint8, 西暦下2桁)
 *    offset 1       month (uint8)
 *    offset 2       day   (uint8)
 *    offset 3       hour  (uint8)
 *    offset 4       min   (uint8)
 *    offset 5..8    value (float, 測定値 MLSS/SS mg/L・透視度 度・界面 水深 m)
 *    offset 9..12   depth (float, 水深 m)
 *    offset 13..14  raw_mv_x10 (int16, 透過光電圧 0.1mV 単位、後校正の逆算用)
 *  → 1 page に 2 件 (offset 0..14, 15..29) + offset 31 に XOR CS
 *
 * ----------------------------------------------------------------------------
 *  v2.0 → v2.1 変更点 (2026-06-01):
 *    - 測定履歴を 1 本 (30件) → 3 種別 (MLSS/SS/透視度 各 30件) に分離。
 *      page 81-90 (10p) → page 81-125 (45p = 15p × 3) に拡張
 *    - 履歴レコードを DO 計レガシー {DO, TEMP} から {value, depth, raw_mv_x10}
 *      に刷新 (15B、ページあたり 2 件は維持)。後校正用に透過光電圧を同梱
 *    - EEP_HIS_COUNT を 10 → 15 に修正 (実装は元々 30件=15p 書込)
 *    - EEP_PAGE_MAX を 121 → 126 に変更 (ロガーは未実装、page 126 以降は予約)
 *
 *  v1.0 → v2.0 変更点 (2026-05-13):
 *    - 旧 1Wire 領域 (page 5-9)、ロガー設定 (page 10)、データロガー
 *      (page 40-929)、校正履歴系 (page 930-946) を**廃止**し前詰め
 *    - 校正係数を末尾 (page 947+) から前半 (page 19-60) に移動
 *    - 測定履歴を page 20-39 → page 81-90 (10 page) に移動・縮小
 *    - TR 校正係数を「相関式 1 つ分」(2 page) のみに削減 (TR は相関式選択無し)
 *    - EEP_PAGE_MAX を 1024 → 121 に変更
 *
 *  相関式と校正の関係:
 *    - MLSS / SS: 相関式 No.1-30 (内部値 0-29)、No.1-20 は固定係数 (校正不可)、
 *                 No.21-30 は校正可能 (EEPROM に係数を保存)
 *    - TR: 相関式選択無し、校正係数は 1 セットのみ (page 59-60 を直接読み書き)
 *
 *  1 相関式あたりの校正係数ページ配置 (2 page = 64B 枠):
 *    page A (32B 中、offset 0 = EEP_CF_MAGIC 0x5A、offset 1+ に 7 float)
 *      offset 0      EEP_CF_MAGIC (= 0x5A、未書込判定用)
 *      offset 1..4   ZR        (float, ゼロ点校正係数 mV)
 *      offset 5..8   FABS_SPAN (float, 器差校正係数)
 *      offset 9..12  SP_A      (float, スパン係数 y=Ax^2+Bx+C)
 *      offset 13..16 SP_B
 *      offset 17..20 SP_C
 *      offset 21..24 1A        (float, フルスパン基準)
 *      offset 25..28 1B        (float, 1/2 スパン基準)
 *      offset 29..30 予備 (2B)
 *      offset 31     XOR checksum
 *    page B (予備、将来 ADZR 等を追加する余地)
 * ============================================================================
 */
#ifndef EEPROM_H
#define EEPROM_H

#ifdef EEPROM_C
#define EXTERN
#else
#define EXTERN extern
#endif

#define EEP_PAGE_BYTE 32		//ページあたりバイト数
#define EEP_PAGE_MAX 126		//EEPROM使用ページ数 (HW 上は 1024 ページ、126 まで使用。ロガー実装時に拡張)

#define EEP_OK 0		//通信OK
#define EEP_BUSY 1		//ビジー
#define EEP_PRM 2		//ページ範囲外
#define EEP_CS 3		//チェックサムエラー
#define EEP_VRFY 4		//ベリファイエラー

#define SPI_OK 0		//通信OK
#define SPI_PARAM 1		//パラメータ異常（未使用）
#define SPI_BUSY 2		//ビジー

/* --- IM-110 ページアドレス定数 v2.0 (詳細は冗頭レイアウト参照) ---------- */
#define EEP_HEADER_PAGE      0		//"IM-110" ヘッダページ
#define EEP_BOARDADJ_PAGE    1		//本体基板調整 予備領域 先頭 (page 1-3)
#define EEP_INFO_PAGE        4		//エラー情報・本体情報
#define EEP_INTERFACE_PAGE   5		//IM-110 界面測定設定 (Interface_Threshold + 予備)
#define EEP_COMMON_PAGE     12		//IM-110 共通設定 (測定モード/相関式選択/SP濃度)

/* --- IM-110 校正係数ページ ----------------------------------------------- */
#define EEP_MLSS_CF_PAGE    19		//MLSS 校正係数 先頭ページ (No.21-30、2 page × 10)
#define EEP_SS_CF_PAGE      39		//SS  校正係数 先頭ページ (No.21-30、2 page × 10)
#define EEP_TR_CF_PAGE      59		//TR  校正係数 先頭ページ (相関式無し、2 page × 1)
#define EEP_CF_STRIDE        2		//1 相関式あたりの確保ページ数
#define EEP_CF_COUNT        10		//MLSS/SS の校正可能相関式の数 (No.21-30)
#define EEP_TR_CF_COUNT      1		//TR は相関式無し、1 セットのみ
#define EEP_CF_BASE_IDX     20		//校正可能相関式の最小内部値 (No.21 = 20)

/* --- IM-110 測定履歴ページ v2.1 (30件 × 3種別、各 15 page) ------------- */
#define EEP_HIS_COUNT       15		//1 種別あたりの測定履歴 page 数 (30件 ÷ 2件/page)
#define EEP_HIS_MLSS_PAGE   81		//MLSS/界面 測定履歴 先頭ページ (page 81-95)
#define EEP_HIS_SS_PAGE     96		//SS       測定履歴 先頭ページ (page 96-110)
#define EEP_HIS_TR_PAGE    111		//透視度    測定履歴 先頭ページ (page 111-125)
#define EEP_HIS_PAGE        EEP_HIS_MLSS_PAGE	//(旧 API 互換) 既定 = MLSS バンク先頭

/* page 内マジックバイト: page 0 全体のマジック (PRO_NAME) とは別系統で、
 * 個別ページが「全 0」や「全 0xFF」のまま誤って正常扱いされるのを防ぐ。
 * write_param_* で offset 0 に書き、read_param_* で判定する。 */
#define EEP_COMMON_MAGIC     0xA5	//page 12 (共通設定) マジックバイト
#define EEP_CF_MAGIC         0x5A	//校正係数ページ マジックバイト
#define EEP_INTERFACE_MAGIC  0x3C	//page 5 (界面測定設定) マジックバイト

//構造体・共用体の定義
union eepromdata {		//EEPROMデータ分割用共用体
    uint8_t ui8[4];
    float f;
    uint32_t ui32;
    uint16_t ui16[2];
};

union eepromadr {		//EEPROMアドレス分割用共用体
    uint8_t ui8[2];
    uint16_t ui16;
    int16_t i16;
};


EXTERN uint8_t eep_rdata[EEP_PAGE_BYTE];	//EEPROM読込ページデータ
EXTERN uint8_t eep_wdata[EEP_PAGE_BYTE];	//EEPROM書込ページデータ


EXTERN uint8_t eep_read_page(uint16_t pageNum);
EXTERN uint8_t eep_write_page(uint16_t pageNum);
EXTERN void eep_write_enable();
EXTERN uint8_t eep_write_verify_page(uint16_t pageNum);

EXTERN uint8_t eep_init_param(void);
EXTERN uint8_t eep_write_info(void);
EXTERN uint8_t eep_write_adjust(void);
EXTERN uint8_t eep_write_setting(void);

EXTERN uint8_t eep_read_1wireinfo(void);
EXTERN uint8_t eep_read_1wirebk(void);
EXTERN uint8_t eep_write_1wireinfo(void);
EXTERN uint8_t eep_write_1wirebk(void);

EXTERN uint8_t eep_read_history(void);
EXTERN uint8_t eep_write_history(void);
EXTERN uint8_t eep_delete_history(void);
EXTERN uint8_t eep_clear_history(void);
EXTERN uint8_t write_param_info(void);

EXTERN uint8_t eep_clear_logh(uint8_t num);
EXTERN uint8_t eep_read_logh(uint8_t num);
EXTERN uint8_t eep_read_logd(uint8_t num, uint16_t page);
EXTERN uint8_t eep_write_log(uint8_t num);
EXTERN void init_param_set(void);
EXTERN uint8_t Backup_WAGData_Read(void);

//ID-200T
EXTERN uint8_t eep_read_calhis(void);
EXTERN uint8_t eep_write_calhis(uint8_t er);
EXTERN uint8_t eep_delete_calhis(void);
EXTERN uint8_t eep_clear_calhis(void);
EXTERN uint8_t eep_read_precal(void);
EXTERN uint8_t eep_write_precal(void);

/* --- IM-110 v2.0 追加: 界面測定設定 + 共通設定 + 校正係数 read/write --- */
EXTERN uint8_t read_param_interface(void);
EXTERN uint8_t write_param_interface(void);
EXTERN uint8_t read_param_common(void);
EXTERN uint8_t write_param_common(void);
EXTERN uint8_t read_param_calibration(uint8_t meas_mode, uint8_t corr_idx);
EXTERN uint8_t write_param_calibration(uint8_t meas_mode, uint8_t corr_idx);
EXTERN uint8_t read_calibration_page_raw(uint8_t meas_mode, uint8_t corr_idx, float out[7]);	//vault同期用 (グローバル非汚染)
EXTERN uint8_t write_calibration_page_raw(uint8_t meas_mode, uint8_t corr_idx, const float in[7]);


#undef EXTERN
#endif /* #ifndef EEPROM_H */
