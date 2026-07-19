/**
 * @file AllDef.h
 * @brief 共通定義ヘッダーファイル
 * @author 飯島電子 柵木
 * @date 2018/07/25
 */

#include "main.h"
#include "IM_110.h"

//機能有効/無効
#define WIRE_ENABLE (0) // 1-Wire制御 (0...無効, 1...有効) IM-110では無効

//デバッグ用
#define DEBUG (0)	//デバッグ用
#define UART_ON (1)	//測定モードUART（0:無効, 1:有効)

//画面遷移を中心とした状態定義(変数名 operation_mode)
//状態変数説明[0xABCD] A...起動方法によるモード B...メニュー名(タイトル) CD...メニュー内の状態
#define NORMAL 0x0000	//測定モード===============================================

#define STRTDSP (NORMAL | 0x000)	//起動画面表示-----------------------------------------
#define STRT_1 (STRTDSP | 0x00)		//起動画面表示前
#define STRT_2 (STRTDSP | 0x10)		//起動画面表示中(RTC確認)
#define STRT_3 (STRTDSP | 0x20)		//起動画面表示中(1秒経過待ち)
#define STRT_4 (STRTDSP | 0x30)		//操作説明画面
#define STRT_5 (STRTDSP | 0x40)		//操作説明画面

#define MEASURE (NORMAL | 0x100)	//通常測定-----------------------------------------
#define MSR_1 (MEASURE | 0x00)		//測定開始前
#define MSR_2 (MEASURE | 0x10)		//測定中
#define MSR_3 (MEASURE | 0x20)		//安定中
#define MSR_4 (MEASURE | 0x30)		//履歴記録
#define MSR_5 (MEASURE | 0x40)		//記録完了1
#define MSR_6 (MEASURE | 0x50)		//記録完了2
#define MSR_7 (MEASURE | 0x60)		//安定中表示
#define MSR_8 (MEASURE | 0x70)		//通常測定切替メッセージ表示
#define MSR_9 (MEASURE | 0x80)		//通常測定切替待ち

#define LOGGER (NORMAL | 0x200)		//データロガー測定---------------------------------
#define LOG_1 (LOGGER | 0x00)		//データロガー測定開始前
#define LOG_2 (LOGGER | 0x10)		//データロガー測定開始前表示
#define LOG_3 (LOGGER | 0x20)		//データロガー測定待機中1
#define LOG_4 (LOGGER | 0x30)		//データロガー測定待機中2(確認画面)
#define LOG_5 (LOGGER | 0x40)		//データロガー測定中表示
#define LOG_6 (LOGGER | 0x50)		//データロガー測定中処理（タイマー）
#define LOG_7 (LOGGER | 0x60)		//データロガー測定中処理（記録）
#define LOG_8 (LOGGER | 0x70)		//データロガー測定中処理（スイッチ）
#define LOG_9 (LOGGER | 0x80)		//データロガー測定中処理（表示復帰）
#define LOG_A (LOGGER | 0x90)		//データロガー測定完了表示
#define LOG_B (LOGGER | 0xA0)		//データロガー測定完了処理
#define LOG_C (LOGGER | 0xB0)		//ロガー測定切替メッセージ表示
#define LOG_D (LOGGER | 0xC0)		//ロガー測定切替待ち


//ID-200T
#define DISPHIS (NORMAL | 0x300)	//測定履歴表示--------------------------------------
#define DISPHIS_1 (DISPHIS | 0x00)	//測定履歴表示前
#define DISPHIS_2 (DISPHIS | 0x10)	//測定履歴表示
#define DISPHIS_3 (DISPHIS | 0x20)	//測定履歴表示中
#define DISPHIS_4 (DISPHIS | 0x30)	//測定履歴消去中1
#define DISPHIS_5 (DISPHIS | 0x40)	//測定履歴消去中2
#define DISPHIS_6 (DISPHIS | 0x50)	//測定履歴消去中3

#define DISPLOG (NORMAL | 0x500)	//データロガー表示----------------------------------
#define DISPLOG_1 (DISPLOG | 0x00)	//データロガー表示前
#define DISPLOG_2 (DISPLOG | 0x10)	//データロガー表示
#define DISPLOG_3 (DISPLOG | 0x20)	//データロガー表示中

//IM-110
#define M_CHANGE (NORMAL | 0x400)		//測定モード切替------------------------------------
#define M_CHANGE_1 (M_CHANGE | 0x00)		//測定モード切替画面1
#define M_CHANGE_2 (M_CHANGE | 0x10)		//測定モード切替画面2
#define M_CHANGE_3 (M_CHANGE | 0x20)		//測定モード切替画面3
#define M_CHANGE_4 (M_CHANGE | 0x30)		//測定モード切替画面4
#define M_CHANGE_5 (M_CHANGE | 0x40)		//測定モード切替画面5
#define M_CHANGE_6 (M_CHANGE | 0x50)		//測定モード切替画面6

#define M_MENU (NORMAL | 0x600)		//測定メニュー------------------------------------
#define M_MENU_1 (M_MENU | 0x00)		//測定メニュー画面1
#define M_MENU_2 (M_MENU | 0x10)		//測定メニュー画面2
#define M_MENU_3 (M_MENU | 0x20)		//測定メニュー画面3
#define M_MENU_4 (M_MENU | 0x30)		//測定メニュー画面4
#define M_MENU_5 (M_MENU | 0x40)		//測定メニュー画面5
#define M_MENU_6 (M_MENU | 0x50)		//測定メニュー画面6

#define S_CORR (NORMAL | 0x700)		//相関式設定------------------------------------
#define S_CORR_1 (S_CORR | 0x00)		//相関式設定画面1
#define S_CORR_2 (S_CORR | 0x10)		//相関式設定画面2
#define S_CORR_3 (S_CORR | 0x20)		//相関式設定画面3
#define S_CORR_4 (S_CORR | 0x30)		//相関式設定画面4
#define S_CORR_5 (S_CORR | 0x40)		//相関式設定画面5
#define S_CORR_6 (S_CORR | 0x50)		//相関式設定画面6

#define D_SET (NORMAL | 0x800)		//界面設定値設定------------------------------------
#define D_SET_1 (D_SET | 0x00)		//界面設定値設定画面1
#define D_SET_2 (D_SET | 0x10)		//界面設定値設定画面2
#define D_SET_3 (D_SET | 0x20)		//界面設定値設定画面3
#define D_SET_4 (D_SET | 0x30)		//界面設定値設定画面4
#define D_SET_5 (D_SET | 0x40)		//界面設定値設定画面5

#define SETTING 0x1000	//設定メニュー(DISP swを押しながら起動)==================

#define MENU1 (SETTING | 0x000)		//設定メニュー1画面表示-----------------------------
#define MENU1_1 (MENU1 | 0x00)		//設定メニュー1画面表示前
#define MENU1_2 (MENU1 | 0x10)		//設定メニュー1画面表示
#define MENU1_3 (MENU1 | 0x20)		//設定メニュー1画面表示中

#define TANSUI (SETTING | 0x100)	//淡水/海水選択-------------------------------------
#define TANSUI_1 (TANSUI | 0x00)	//淡水/海水選択画面表示前
#define TANSUI_2 (TANSUI | 0x10)	//淡水/海水選択画面表示中
#define TANSUI_3 (TANSUI | 0x20)	//設定表示中1
#define TANSUI_4 (TANSUI | 0x30)	//設定表示中2

#define SETTIME (SETTING | 0x300)	//時刻設定------------------------------------------
#define SETTIME_1 (SETTIME | 0x00)	//時刻設定画面表示前
#define SETTIME_2 (SETTIME | 0x10)	//時刻設定画面表示
#define SETTIME_3 (SETTIME | 0x20)	//時刻設定画面表示中

#define RESET (SETTING | 0x400)		//校正値リセット------------------------------------
#define RESET_1 (RESET | 0x00)		//校正値リセット画面1
#define RESET_2 (RESET | 0x10)		//校正値リセット画面2
#define RESET_3 (RESET | 0x20)		//校正値リセット画面3
#define RESET_4 (RESET | 0x30)		//校正値リセット画面4

#define QR (SETTING | 0x500)		//QR表示------------------------------------
#define QR_1 (QR | 0x00)			//QR表示画面1
#define QR_2 (QR | 0x10)			//QR表示画面2
#define QR_3 (QR | 0x20)			//QR表示画面3
#define QR_4 (QR | 0x30)			//QR表示画面4

#define MENU2 (SETTING | 0x800)		//設定メニュー2画面表示-----------------------------
#define MENU2_1 (MENU2 | 0x00)		//設定メニュー2画面表示前
#define MENU2_2 (MENU2 | 0x10)		//設定メニュー2画面表示
#define MENU2_3 (MENU2 | 0x20)		//設定メニュー2画面表示中

#define SETDSP (SETTING | 0x900)	//設定起動画面表示-----------------------------
#define SETDSP_1 (SETDSP | 0x00)	//設定起動画面1
#define SETDSP_2 (SETDSP | 0x10)	//設定起動画面2
#define SETDSP_3 (SETDSP | 0x20)	//設定起動画面3

#define BT_RESET (SETTING | 0xA00)		//BT初期化------------------------------------
#define BT_RESET_1 (BT_RESET | 0x00)		//BT初期化画面1
#define BT_RESET_2 (BT_RESET | 0x10)		//BT初期化画面2
#define BT_RESET_3 (BT_RESET | 0x20)		//BT初期化画面3
#define BT_RESET_4 (BT_RESET | 0x30)		//BT初期化画面4
#define BT_RESET_5 (BT_RESET | 0x40)		//BT初期化画面5
#define BT_RESET_6 (BT_RESET | 0x50)		//BT初期化画面6

//ID-200T
#define MEASMODE (SETTING | 0xB00)		//モード設定------------------------------------
#define MEASMODE_1 (MEASMODE | 0x00)		//モード設定画面1
#define MEASMODE_2 (MEASMODE | 0x10)		//モード設定画面2
#define MEASMODE_3 (MEASMODE | 0x20)		//モード設定画面3
#define MEASMODE_4 (MEASMODE | 0x30)		//モード設定画面4
#define MEASMODE_5 (MEASMODE | 0x40)		//モード設定画面5
#define MEASMODE_6 (MEASMODE | 0x50)		//モード設定画面6

#define ACALSET (SETTING | 0xC00)		//自動校正------------------------------------
#define ACALSET_1 (ACALSET | 0x00)		//自動校正画面1
#define ACALSET_2 (ACALSET | 0x10)		//自動校正画面2
#define ACALSET_3 (ACALSET | 0x20)		//自動校正画面3
#define ACALSET_4 (ACALSET | 0x30)		//自動校正画面4
#define ACALSET_5 (ACALSET | 0x40)		//自動校正画面5
#define ACALSET_6 (ACALSET | 0x50)		//自動校正画面6

#define APPDL (SETTING | 0xD00)		//アプリDL先------------------------------------
#define APPDL_1 (APPDL | 0x00)		//アプリDL先画面1
#define APPDL_2 (APPDL | 0x10)		//アプリDL先画面2
#define APPDL_3 (APPDL | 0x20)		//アプリDL先画面3
#define APPDL_4 (APPDL | 0x30)		//アプリDL先画面4
#define APPDL_5 (APPDL | 0x40)		//アプリDL先画面5
#define APPDL_6 (APPDL | 0x50)		//アプリDL先画面6


#define ADBOAD 0x2000	//基板調整(基板調整用ケーブルを差し込んで電源ON)=========

#define PRGVER (ADBOAD | 0x000)		//バージョン表示------------------------------------
#define PRG_1 (PRGVER | 0x00)		//バージョン表示前
#define PRG_2 (PRGVER | 0x10)		//バージョン表示中
#define PRG_3 (PRGVER | 0x20)		//履歴消去
#define PRG_4 (PRGVER | 0x30)		//消去完了待ち

#define EEPTEST (ADBOAD | 0x100)	//EEPROM確認----------------------------------------
#define EEP_1 (EEPTEST | 0x00)		//EEPROM確認開始前
#define EEP_2 (EEPTEST | 0x10)		//EEPROM確認待機中
#define EEP_3 (EEPTEST | 0x20)		//EEPROM確認[0xAA]書込
#define EEP_4 (EEPTEST | 0x30)		//EEPROM確認[0xAA]ベリファイ
#define EEP_5 (EEPTEST | 0x40)		//EEPROM確認[0x55]書込
#define EEP_6 (EEPTEST | 0x50)		//EEPROM確認[0x55]ベリファイ
#define EEP_7 (EEPTEST | 0x60)		//EEPROM確認[0x00]書込
#define EEP_8 (EEPTEST | 0x70)		//EEPROM確認[0x00]ベリファイ
#define EEP_9 (EEPTEST | 0x80)		//EEPROM確認結果表示
#define EEP_A (EEPTEST | 0x90)		//EEPROM確認結果表示中

#define BATVOL (ADBOAD | 0x200)		//電池電圧表示--------------------------------------
#define BAT_1 (BATVOL | 0x00)		//電池電圧表示前
#define BAT_2 (BATVOL | 0x10)		//電池電圧表示中

//レンジ調整画面表示用（構造共通化のため）=================
#define ADJ_1 0x00		//調整前画面表示
#define ADJ_2 0x10		//調整中画面表示
#define ADJ_3 0x20		//画面切替操作待ち
#define ADJ_4 0x30		//未使用
#define ADJ_5 0x40		//未使用
#define ADJ_6 0x50		//未使用

#define LOWRANGE0 (ADBOAD | 0xA00)	//低レンジゼロ調整--------------------------------------
#define LOW0_1 (LOWRANGE0 | 0x00)	//低レンジゼロ調整測定前
#define LOW0_2 (LOWRANGE0 | 0x10)	//低レンジゼロ調整測定中表示
#define LOW0_3 (LOWRANGE0 | 0x20)	//低レンジゼロ調整測定中
#define LOW0_4 (LOWRANGE0 | 0x30)	//低レンジゼロ調整測定中表示更新
#define LOW0_5 (LOWRANGE0 | 0x40)	//低レンジゼロ調整完了

#define MIDRANGE0 (ADBOAD | 0xB00)	//中レンジゼロ調整--------------------------------------
#define MID0_1 (MIDRANGE0 | 0x00)	//中レンジゼロ調整測定前
#define MID0_2 (MIDRANGE0 | 0x10)	//中レンジゼロ調整測定中表示
#define MID0_3 (MIDRANGE0 | 0x20)	//中レンジゼロ調整測定中
#define MID0_4 (MIDRANGE0 | 0x30)	//中レンジゼロ調整測定中表示更新
#define MID0_5 (MIDRANGE0 | 0x40)	//中レンジゼロ調整完了

#define HIRANGE0 (ADBOAD | 0xC00)	//高レンジゼロ調整--------------------------------------
#define HIGH0_1 (HIRANGE0 | 0x00)	//高レンジゼロ調整測定前
#define HIGH0_2 (HIRANGE0 | 0x10)	//高レンジゼロ調整測定中表示
#define HIGH0_3 (HIRANGE0 | 0x20)	//高レンジゼロ調整測定中
#define HIGH0_4 (HIRANGE0 | 0x30)	//高レンジゼロ調整測定中表示更新
#define HIGH0_5 (HIRANGE0 | 0x40)	//高レンジゼロ調整完了

#define LOWRANGE (ADBOAD | 0x300)	//低レンジ調整--------------------------------------
#define LOW_1 (LOWRANGE | 0x00)		//低レンジ調整測定前
#define LOW_2 (LOWRANGE | 0x10)		//低レンジ調整測定中表示
#define LOW_3 (LOWRANGE | 0x20)		//低レンジ調整測定中
#define LOW_4 (LOWRANGE | 0x30)		//低レンジ調整測定中表示更新
#define LOW_5 (LOWRANGE | 0x40)		//低レンジ調整完了

#define MIDRANGE (ADBOAD | 0x400)	//中レンジ調整--------------------------------------
#define MID_1 (MIDRANGE | 0x00)		//中レンジ調整測定前
#define MID_2 (MIDRANGE | 0x10)		//中レンジ調整測定中表示
#define MID_3 (MIDRANGE | 0x120)	//中レンジ調整測定中
#define MID_4 (MIDRANGE | 0x30)		//中レンジ調整測定中表示更新
#define MID_5 (MIDRANGE | 0x40)		//中レンジ調整完了

#define HIRANGE (ADBOAD | 0x500)	//高レンジ調整--------------------------------------
#define HIGH_1 (HIRANGE | 0x00)		//高レンジ調整測定前
#define HIGH_2 (HIRANGE | 0x10)		//高レンジ調整測定中表示
#define HIGH_3 (HIRANGE | 0x20)		//高レンジ調整測定中
#define HIGH_4 (HIRANGE | 0x30)		//高レンジ調整測定中表示更新
#define HIGH_5 (HIRANGE | 0x40)		//高レンジ調整完了

#define WAT05C (ADBOAD | 0x600)		//水温5℃調整---------------------------------------
#define WAT05C_1 (WAT05C | 0x00)	//水温5℃調整測定前
#define WAT05C_2 (WAT05C | 0x10)	//水温5℃調整測定中表示
#define WAT05C_3 (WAT05C | 0x20)	//水温5℃調整測定中
#define WAT05C_4 (WAT05C | 0x30)	//水温5℃調整測定中表示更新
#define WAT05C_5 (WAT05C | 0x40)	//水温5℃調整完了

#define WAT20C (ADBOAD | 0x700)		//水温20℃調整--------------------------------------
#define WAT20C_1 (WAT20C | 0x00)	//水温20℃調整測定前
#define WAT20C_2 (WAT20C | 0x10)	//水温20℃調整測定中表示
#define WAT20C_3 (WAT20C | 0x20)	//水温20℃調整測定中
#define WAT20C_4 (WAT20C | 0x30)	//水温20℃調整測定中表示更新
#define WAT20C_5 (WAT20C | 0x40)	//水温20℃調整完了

#define WAT35C (ADBOAD | 0x800)		//水温35℃調整--------------------------------------
#define WAT35C_1 (WAT35C | 0x00)	//水温35℃調整測定前
#define WAT35C_2 (WAT35C | 0x10)	//水温35℃調整測定中表示
#define WAT35C_3 (WAT35C | 0x20)	//水温35℃調整測定中
#define WAT35C_4 (WAT35C | 0x30)	//水温35℃調整測定中表示更新
#define WAT35C_5 (WAT35C | 0x40)	//水温35℃調整完了

#define CALTEMP (ADBOAD | 0x900)	//水温1点調整--------------------------------------
#define CALTEMP_1 (CALTEMP | 0x00)	//水温1点調整表示前
#define CALTEMP_2 (CALTEMP | 0x10)	//水温1点調整表示
#define CALTEMP_3 (CALTEMP | 0x20)	//水温1点調整表示中
#define CALTEMP_4 (CALTEMP | 0x30)	//水温1点調整表示前2
#define CALTEMP_5 (CALTEMP | 0x40)	//水温1点調整表示2
#define CALTEMP_6 (CALTEMP | 0x50)	//水温1点調整表示中2

#define SETYEAR (ADBOAD | 0xD00)	//日付設定年--------------------------------------
#define SETYEAR_1 (SETYEAR | 0x00)	//日付設定年表示前
#define SETYEAR_2 (SETYEAR | 0x10)	//日付設定年表示
#define SETYEAR_3 (SETYEAR | 0x20)	//日付設定年表示中
#define SETYEAR_4 (SETYEAR | 0x30)	//日付設定年表示前2
#define SETYEAR_5 (SETYEAR | 0x40)	//日付設定年表示2
#define SETYEAR_6 (SETYEAR | 0x50)	//日付設定年表示中2

#define SETDAYS (ADBOAD | 0xE00)	//日付設定月日--------------------------------------
#define SETDAYS_1 (SETDAYS | 0x00)	//日付設定月日表示前
#define SETDAYS_2 (SETDAYS | 0x10)	//日付設定月日表示
#define SETDAYS_3 (SETDAYS | 0x20)	//日付設定月日表示中
#define SETDAYS_4 (SETDAYS | 0x30)	//日付設定月日表示前2
#define SETDAYS_5 (SETDAYS | 0x40)	//日付設定月日表示2
#define SETDAYS_6 (SETDAYS | 0x50)	//日付設定月日表示中2

#define SETHOUR (ADBOAD | 0xF00)	//日付設定時分--------------------------------------
#define SETHOUR_1 (SETHOUR | 0x00)	//日付設定時分表示前
#define SETHOUR_2 (SETHOUR | 0x10)	//日付設定時分表示
#define SETHOUR_3 (SETHOUR | 0x20)	//日付設定時分表示中
#define SETHOUR_4 (SETHOUR | 0x30)	//日付設定時分表示前2
#define SETHOUR_5 (SETHOUR | 0x40)	//日付設定時分表示2
#define SETHOUR_6 (SETHOUR | 0x50)	//日付設定時分表示中2

#define ERRMSG 0x3000	//エラー、ガイダンス画面表示中========================================
#define ERR1W 0x3000				//エラー履歴1Wire書き込み
#define ERRDSP1 0x3500				//エラー画面1表示
#define ERRWAIT1 0x3100				//エラー1表示中（SW待ち）
#define ERRDSP2 0x3200				//エラー画面2表示
#define ERRWAIT2 0x3300				//エラー2表示中（SW待ち）
#define ERRWAIT3 0x3400				//ガイダンス表示中（SW待ち）

#define CAL 0x4000		//校正モード（MEM swを押しながら電源ON）==============================================

#define CAL_MODE_Z  0	//校正モード選択: ゼロ校正
#define CAL_MODE_2P 1	//校正モード選択: 2点校正
#define CAL_MODE_3P 2	//校正モード選択: 3点校正

#define ZCAL (CAL | 0x000)			//ゼロ校正 ---------------------------------------
#define ZCAL_1 (ZCAL | 0x00)		//ゼロ校正開始前
#define ZCAL_2 (ZCAL | 0x10)		//ゼロ校正開始前表示
#define ZCAL_3 (ZCAL | 0x20)		//ゼロ校正待機中
#define ZCAL_4 (ZCAL | 0x30)		//ゼロ校正中
#define ZCAL_5 (ZCAL | 0x40)		//ゼロ校正完了表示（Good）
#define ZCAL_6 (ZCAL | 0x50)		//ゼロ校正完了待ち
#define ZCAL_7 (ZCAL | 0x60)		//ゼロ校正完了表示
#define ZCAL_8 (ZCAL | 0x70)		//ゼロ校正予備1
#define ZCAL_9 (ZCAL | 0x80)		//ゼロ校正予備2

#define ADCAL (CAL | 0x100)			//スパン校正---------------------------------------
#define ADCAL_1 (ADCAL | 0x00)		//校正開始前
#define ADCAL_2 (ADCAL | 0x10)		//校正開始前表示
#define ADCAL_3 (ADCAL | 0x20)		//校正待機中
#define ADCAL_4 (ADCAL | 0x30)		//校正中
#define ADCAL_5 (ADCAL | 0x40)		//校正完了表示（Good）
#define ADCAL_6 (ADCAL | 0x50)		//校正完了待ち
#define ADCAL_7 (ADCAL | 0x60)		//校正完了表示
#define ADCAL_8 (ADCAL | 0x70)		//校正予備1
#define ADCAL_9 (ADCAL | 0x80)		//校正予備2

#define MCAL (CAL | 0x200)			//中間校正---------------------------------------
#define MCAL_1 (MCAL | 0x00)		//校正開始前
#define MCAL_2 (MCAL | 0x10)		//校正開始前表示
#define MCAL_3 (MCAL | 0x20)		//校正待機中
#define MCAL_4 (MCAL | 0x30)		//校正中
#define MCAL_5 (MCAL | 0x40)		//校正完了表示（Good）
#define MCAL_6 (MCAL | 0x50)		//校正完了待ち
#define MCAL_7 (MCAL | 0x60)		//校正完了表示
#define MCAL_8 (MCAL | 0x70)		//校正予備1
#define MCAL_9 (MCAL | 0x80)		//校正予備2

#define C_MODE (CAL | 0x300)		//校正モード選択------------------------------------
#define C_MODE_1 (C_MODE | 0x00)		//校正モード選択画面1
#define C_MODE_2 (C_MODE | 0x10)		//校正モード選択画面2
#define C_MODE_3 (C_MODE | 0x20)		//校正モード選択画面3
#define C_MODE_4 (C_MODE | 0x30)		//校正モード選択画面4
#define C_MODE_5 (C_MODE | 0x40)		//校正モード選択画面5
#define C_MODE_6 (C_MODE | 0x50)		//校正モード選択画面6

#define C_S_CORR (CAL | 0x400)		//相関式設定------------------------------------
#define C_S_CORR_1 (C_S_CORR | 0x00)		//相関式設定画面1
#define C_S_CORR_2 (C_S_CORR | 0x10)		//相関式設定画面2
#define C_S_CORR_3 (C_S_CORR | 0x20)		//相関式設定画面3
#define C_S_CORR_4 (C_S_CORR | 0x30)		//相関式設定画面4
#define C_S_CORR_5 (C_S_CORR | 0x40)		//相関式設定画面5
#define C_S_CORR_6 (C_S_CORR | 0x50)		//相関式設定画面6

#define C_S_SET (CAL | 0x500)		//スパン校正値設定------------------------------------
#define C_S_SET_1 (C_S_SET | 0x00)		//スパン校正値設定画面1
#define C_S_SET_2 (C_S_SET | 0x10)		//スパン校正値設定画面2
#define C_S_SET_3 (C_S_SET | 0x20)		//スパン校正値設定画面3
#define C_S_SET_4 (C_S_SET | 0x30)		//スパン校正値設定画面4
#define C_S_SET_5 (C_S_SET | 0x40)		//スパン校正値設定画面5
#define C_S_SET_6 (C_S_SET | 0x50)		//スパン校正値設定画面6

#define C_M_SET (CAL | 0x600)		//中間濃度設定 (3点校正用、C_S_SET のコピー)------------
#define C_M_SET_1 (C_M_SET | 0x00)		//中間濃度設定画面1
#define C_M_SET_2 (C_M_SET | 0x10)		//中間濃度設定画面2
#define C_M_SET_3 (C_M_SET | 0x20)		//中間濃度設定画面3

#define C_MENU (CAL | 0x700)		//校正メニュー------------------------------------
#define C_MENU_1 (C_MENU | 0x00)		//校正メニュー画面1
#define C_MENU_2 (C_MENU | 0x10)		//校正メニュー画面2
#define C_MENU_3 (C_MENU | 0x20)		//校正メニュー画面3
#define C_MENU_4 (C_MENU | 0x30)		//校正メニュー画面4
#define C_MENU_5 (C_MENU | 0x40)		//校正メニュー画面5
#define C_MENU_6 (C_MENU | 0x50)		//校正メニュー画面6

#define C_MENU (CAL | 0x700)		//校正メニュー------------------------------------
#define C_MENU_1 (C_MENU | 0x00)		//校正メニュー画面1
#define C_MENU_2 (C_MENU | 0x10)		//校正メニュー画面2
#define C_MENU_3 (C_MENU | 0x20)		//校正メニュー画面3
#define C_MENU_4 (C_MENU | 0x30)		//校正メニュー画面4
#define C_MENU_5 (C_MENU | 0x40)		//校正メニュー画面5
#define C_MENU_6 (C_MENU | 0x50)		//校正メニュー画面6

#define DISPCAL (CAL | 0x800)	//校正履歴表示--------------------------------------
#define DISPCAL_1 (DISPCAL | 0x00)	//校正履歴表示前
#define DISPCAL_2 (DISPCAL | 0x10)	//校正履歴表示
#define DISPCAL_3 (DISPCAL | 0x20)	//校正履歴表示中
#define DISPCAL_4 (DISPCAL | 0x30)	//校正履歴消去中1
#define DISPCAL_5 (DISPCAL | 0x40)	//校正履歴消去中2
#define DISPCAL_6 (DISPCAL | 0x50)	//校正履歴消去中3

#define CALDSP (CAL | 0x900)		//校正起動画面表示-----------------------------
#define CALDSP_1 (CALDSP | 0x00)	//校正起動画面1
#define CALDSP_2 (CALDSP | 0x10)	//校正起動画面2
#define CALDSP_3 (CALDSP | 0x20)	//校正起動画面3

#define C_RESET (CAL | 0xA00)		//校正リセット------------------------------------
#define C_RESET_1 (C_RESET | 0x00)		//校正リセット画面1
#define C_RESET_2 (C_RESET | 0x10)		//校正リセット画面2
#define C_RESET_3 (C_RESET | 0x20)		//校正リセット画面3
#define C_RESET_4 (C_RESET | 0x30)		//校正リセット画面4
#define C_RESET_5 (C_RESET | 0x40)		//校正リセット画面5
#define C_RESET_6 (C_RESET | 0x50)		//校正リセット画面6

#define CAL_HSEL (CAL | 0xB00)		//後校正: 測定記録選択----------------------------
#define CAL_HSEL_1 (CAL_HSEL | 0x00)	//記録選択 表示前 (履歴読込・件数算出)
#define CAL_HSEL_2 (CAL_HSEL | 0x10)	//記録選択 表示
#define CAL_HSEL_3 (CAL_HSEL | 0x20)	//記録選択 操作待ち



//GPIO SW状態などの共通定義=============================================
#define UP 0x0
#define DOWN 0x1
#define OFF 0x0
#define ON 0x1
#define OK 0x0
#define NG 0x1

//DOアンプレンジ=======================================================
#define LOW_RANGE 0x0
#define MID_RANGE 0x1
#define HIGH_RANGE 0x2

//通信関連定義=================================================
#define STX 0x2
#define ETX 0x3
#define ACK 0x6
#define NAK 0x15
#define CRN 0xD
#define LF 0xA
#define CR 0xd
