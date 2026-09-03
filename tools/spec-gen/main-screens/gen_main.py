"""IM-110 本体 通常運用画面 (測定/校正/設定/ガイダンス/エラー) の画面図を実装コードどおりに生成する。
   描画元: Display.c / common/LS027_Display.c / IIJIMA_Templete.c (LS027_disp_battery_icon,
   LS027_disp_datetime, LS027_disp_airbar3, LS027_disp_bar_icon, LS027_disp_cal)。
   実機撮影ではなく再現図。表示値は代表値。QR コードの中身は省略 (枠のみ)。
   使い方: python3 gen_main.py <出力ディレクトリ>
"""
import sys, os
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'adboad'))
from lcdemu import Screen, ICON, TABLE

OUT = sys.argv[1] if len(sys.argv) > 1 else '.'

# ---- IIJIMA_Templete.c の共通描画ヘルパ再現 --------------------------------
def battery(s, level=3):                       # LS027_disp_battery_icon (bat_x=44, bat_y=1)
    s.icon(44, 1, f'icon_battery{level}')

def datetime(s, y=26, mo=8, d=11, hh=10, mm=30):   # LS027_disp_datetime
    s.number(16, 1, 2, y // 10 % 10, 0); s.number(18, 1, 2, y % 10, 0)
    s.icon(20, 1, 's_slash')
    s.number(22, 1, 2, mo // 10 % 10, 0); s.number(24, 1, 2, mo % 10, 0)
    s.icon(26, 1, 's_slash')
    s.number(28, 1, 2, d // 10 % 10, 0);  s.number(30, 1, 2, d % 10, 0)
    s.number(33, 1, 2, hh // 10 % 10, 0); s.number(35, 1, 2, hh % 10, 0)
    s.icon(37, 1, 's_colon')
    s.number(39, 1, 2, mm // 10 % 10, 0); s.number(41, 1, 2, mm % 10, 0)

def airbar3(s, pct):                           # LS027_disp_airbar3 (界面判断バー)
    bar2 = 16.0 * pct / 100.0 + 0.5
    if bar2 > 19.0: bar2 = 19.0
    s.icon(0, 5, 'icon_air_bar3')
    s.fill_box(0, 7, int(bar2), 1, 0)

def progbar(s, pct):                           # LS027_disp_bar_icon (進捗バー)
    s.icon(5, 29, 'icon_bar')
    s.fill_box(5, 29, int(pct * 40 / 100), 1, 0)

def caldate(s, x, y, yy=2026, mo=8, d=11):     # LS027_disp_cal type=0 (年月日)
    s.number(x + 6, y, 2, yy, 0)
    s.icon(x + 8, y, 's_slash')
    s.number(x + 10, y, 2, mo // 10 % 10, 0); s.number(x + 12, y, 2, mo % 10, 0)
    s.icon(x + 14, y, 's_slash')
    s.number(x + 16, y, 2, d // 10 % 10, 0);  s.number(x + 18, y, 2, d % 10, 0)

def mode30(s, x_mode, x_slash, x_30, no=1, x_no=None, y=25):   # 「モード N /30」ラベル
    s.icon(x_mode, y, 'icon_mode'); s.icon(x_slash, y, 's_slash'); s.number(x_30, y, 2, 30, 0)
    if x_no is not None:
        s.number(x_no, y, 2, no, 0)

def mlss_main(s, val):                         # Meas_2/3 MLSS 主表示 (y=11)
    if val >= 10000:
        s.number(38, 11, 0, val / 1000.0, 1); s.icon(46, 12, 'l_gl')
    elif val >= 1000:
        v = int(val + 0.5)
        s.icon(38, 11, TABLE['l_num'][v % 10]);        s.icon(32, 11, TABLE['l_num'][v // 10 % 10])
        s.icon(26, 11, TABLE['l_num'][v // 100 % 10]); s.icon(24, 11, 'l_comma2')
        s.icon(18, 11, TABLE['l_num'][v // 1000]);     s.icon(44, 17, 'l_mgl')
    else:
        s.number(38, 11, 0, val, 0); s.icon(44, 17, 'l_mgl')

def mlss_ifc(s, pct=62.0, depth=0.12):         # MLSS 界面機能表示 (バー+現水深+ホールド未捕捉)
    airbar3(s, pct)
    s.number(26, 5, 2, depth, 2); s.icon(28, 5, 'm_m')          # 現在水深 (S)
    s.icon(33, 5, 'm_minus'); s.icon(36, 5, 'm_dot')            # ホールド "-.--"
    s.icon(38, 5, 'm_minus'); s.icon(41, 5, 'm_minus'); s.icon(45, 8, 'm_m')

def nav(s, pairs):                             # 左列 SW 凡例 [(y, sw_icon, label_icon)]
    for y, sw, lb in pairs:
        s.icon(0, y, sw)
        if lb: s.icon(5, y, lb)

def base(title):                               # タイトル+電池+日時
    s = Screen(); s.icon(0, 1, title); battery(s); datetime(s)
    return s

SCR = {}

# ---- 2章 測定 ---------------------------------------------------------------
s = Screen()                                                     # 起動画面 (disp_STRTDSP)
s.icon(4, 7, 'msg_IM110'); s.number(43, 16, 2, 0.43, 2)
s.icon(31, 16, 'msg_Ver'); s.icon(21, 24, 'msg_iijima')
SCR['strt_logo'] = s

s = Screen()                                                     # 操作説明 (disp_STRTDSP2)
s.icon(1, 1, 'msg_cal_menu');  s.icon(14, 1, 'msg_toscr')
s.icon(3, 9, 'icon_MEM');      s.icon(9, 9, 'msg_push1')
s.icon(26, 9, 'icon_POWER');   s.icon(32, 9, 'msg_push2')
s.icon(1, 16, 'msg_setting_menu'); s.icon(29, 16, 'msg_toscr')
s.icon(3, 24, 'icon_DISP');    s.icon(9, 24, 'msg_push1')
s.icon(26, 24, 'icon_POWER');  s.icon(32, 24, 'msg_push2')
SCR['strt_help'] = s

s = base('t_MLSS')                                               # MLSS 測定中 (disp_MLSS_Meas_2)
s.icon(3, 8, 'icon_wait1'); mlss_main(s, 2340); mlss_ifc(s)
s.number(39, 25, 2, 1, 0); mode30(s, 28, 41, 45)
progbar(s, 40); nav(s, [(18, 'icon_DISP', 'icon_history')])
SCR['meas_mlss_wait'] = s

s = base('t_MLSS')                                               # MLSS ホールド (disp_MLSS_Meas_3 save=0)
mlss_main(s, 2340); mlss_ifc(s)
s.number(39, 25, 2, 1, 0); mode30(s, 28, 41, 45)
nav(s, [(13, 'icon_DISP', 'icon_history'), (18, 'icon_MEM', 'icon_save'), (23, 'icon_MEM', 'icon_l_calmemory')])
SCR['meas_mlss_hold'] = s

s = base('t_MLSS')                                               # MLSS 記録完了 (disp_MLSS_Meas_3 save=1)
mlss_main(s, 2340); mlss_ifc(s)
s.number(39, 25, 2, 1, 0); mode30(s, 28, 41, 45)
nav(s, [(13, 'icon_DISP', 'icon_history'), (23, 'icon_MEM', 'icon_l_calmemory')])
s.icon(0, 18, 'icon_save'); s.icon(10, 18, 'icon_complete')
SCR['meas_mlss_saved'] = s

s = base('t_MLSS')                                               # MLSS 後校正記録完了 (disp_MLSS_Meas_3 save=3)
mlss_main(s, 2340); mlss_ifc(s)
s.number(39, 25, 2, 1, 0); mode30(s, 28, 41, 45)
nav(s, [(13, 'icon_DISP', 'icon_history'), (18, 'icon_MEM', 'icon_save')])
s.icon(0, 23, 'icon_save'); s.icon(10, 23, 'icon_complete')
SCR['meas_mlss_pcal'] = s

s = base('t_SS')                                                 # SS 測定中
s.icon(3, 8, 'icon_wait1'); s.number(38, 11, 0, 245, 0); s.icon(44, 17, 'l_mgl')
progbar(s, 55); nav(s, [(18, 'icon_DISP', 'icon_history')])
SCR['meas_ss_wait'] = s

s = base('t_Transparency')                                       # 透視度 測定中
s.icon(3, 8, 'icon_wait1'); s.number(38, 11, 0, 65.3, 1); s.icon(44, 17, 'm_cm')
progbar(s, 70); nav(s, [(18, 'icon_DISP', 'icon_history')])
SCR['meas_tr_wait'] = s

s = base('t_Transparency')                                       # 透視度 100cm 超 (100.0+OVER)
s.icon(3, 8, 'icon_over'); s.number(38, 11, 0, 100.0, 1); s.icon(44, 17, 'm_cm')
progbar(s, 100); nav(s, [(18, 'icon_DISP', 'icon_history')])
SCR['meas_tr_over'] = s

s = base('t_Transparency')                                       # 透視度 0cm 以下 ("-.-")
s.icon(3, 8, 'icon_wait1')
s.icon(28, 11, 'l_minus'); s.icon(34, 11, 'l_dot'); s.icon(38, 11, 'l_minus')
s.icon(44, 17, 'm_cm'); progbar(s, 0); nav(s, [(18, 'icon_DISP', 'icon_history')])
SCR['meas_tr_minus'] = s

s = base('t_measure')                                            # モード切替 (disp_M_CHANGE flg=0)
nav(s, [(18, 'icon_DISP', 'icon_select'), (23, 'icon_MEM', 'icon_decision')])
s.icon(18, 7, 'b_MLSS'); s.icon(34, 7, 'w_SS'); s.icon(18, 18, 'w_Transparency')
SCR['meas_change'] = s

s = base('t_measure')                                            # 「MLSSに設定しました」(disp_M_CHANGE flg=1)
nav(s, [(18, 'icon_DISP', 'icon_select'), (23, 'icon_MEM', 'icon_decision')])
s.icon(15, 6, 'msg_MLSS')
SCR['meas_change_msg'] = s

s = base('t_correlation')                                        # 相関式 2桁入力 (disp_Corr_setting_30)
s.icon(24, 9, TABLE['b_l_num'][2]); s.icon(30, 9, TABLE['l_num'][1])   # "21" (10の位 編集中)
s.number(38, 21, 1, 1250, 0); s.icon(42, 23, 'm_mgl')                  # MLSS 試算値
nav(s, [(13, 'icon_DISP', 'icon_l_return'), (18, 'icon_DISP', 'icon_change'), (23, 'icon_MEM', 'icon_next')])
SCR['corr_input'] = s

s = base('t_depth_setting')                                      # 界面しきい値設定 (disp_Depth_Setting)
for i, (x, n) in enumerate(zip((16, 22, 28, 34, 40), (1, 0, 0, 0, 0))):
    s.icon(x, 5, TABLE['b_l_num' if i == 0 else 'l_num'][n])     # 10000 (先頭桁 編集中)
s.icon(42, 17, 'l_mgl')
nav(s, [(13, 'icon_DISP', 'icon_l_return'), (18, 'icon_DISP', 'icon_change'), (23, 'icon_MEM', 'icon_next')])
SCR['depth_setting'] = s

def hist_row(s, x, y, no, hh, mm, mo, d, val, depth, mode='mlss'):   # disp_his_data 相当
    s.number(x, y, 2, no // 10 % 10, 0); s.number(x + 2, y, 2, no % 10, 0)
    s.number(x + 6, y, 2, hh // 10 % 10, 0); s.number(x + 8, y, 2, hh % 10, 0)
    s.icon(x + 10, y, 's_colon')
    s.number(x + 12, y, 2, mm // 10 % 10, 0); s.number(x + 14, y, 2, mm % 10, 0)
    s.number(x + 18, y, 2, mo // 10 % 10, 0); s.number(x + 20, y, 2, mo % 10, 0)
    s.icon(x + 22, y, 's_slash')
    s.number(x + 24, y, 2, d // 10 % 10, 0);  s.number(x + 26, y, 2, d % 10, 0)
    dx, dy = x + 2 + 10, y + 4                                   # disp_do_temp_data
    s.number(dx, dy, 2, val, 1 if mode == 'tr' else 0)
    s.icon(dx + 2, dy, {'tr': 's_cm', 'mlss': 's_mgl', 'ss': 's_mgl'}[mode])
    if mode == 'mlss':                                           # 水深は MLSS のみ
        s.number(dx + 2 + 7 + 6, dy, 2, depth, 1); s.icon(dx + 2 + 7 + 6 + 2, dy, 's_m')

s = base('t_disp_meas_history')                                  # 測定履歴 (disp_DISPHIS index=0)
nav(s, [(13, 'icon_DISP', 'icon_measure'), (23, 'icon_MEM', 'icon_next')])
s.icon(0, 18, 'icon_l_erase')
hist_row(s, 16, 6, 1, 10, 21, 8, 11, 2340, 0.1)
s.icon(16, 13, 'icon_line')
hist_row(s, 16, 14, 2, 9, 45, 8, 11, 1980, 0.1)
s.icon(16, 21, 'icon_line')
hist_row(s, 16, 22, 3, 15, 2, 8, 10, 2100, 0.2)
SCR['hist_mlss'] = s

# ---- 3章 校正 ---------------------------------------------------------------
s = base('t_cal_menu')                                           # 校正メニュー 電源ON MLSS (disp_C_MENU)
nav(s, [(13, 'icon_DISP', 'icon_l_return'), (18, 'icon_DISP', 'icon_select'), (23, 'icon_MEM', 'icon_decision')])
s.icon(17, 7, 'b_0cal'); s.icon(34, 7, 'w_2pcal')
s.icon(17, 17, 'w_3pcal'); s.icon(34, 17, 'w_calreset_s')
SCR['cal_menu'] = s

s = base('t_cal_menu')                                           # 校正メニュー 透視度 (3点校正なし)
nav(s, [(13, 'icon_DISP', 'icon_l_return'), (18, 'icon_DISP', 'icon_select'), (23, 'icon_MEM', 'icon_decision')])
s.icon(17, 7, 'b_0cal'); s.icon(34, 7, 'w_2pcal'); s.icon(34, 17, 'w_calreset_s')
SCR['cal_menu_tr'] = s

s = base('t_zero_cal')                                           # ゼロ校正中 MLSS (disp_MLSS_ZCal_2)
s.icon(3, 8, 'icon_wait1'); s.number(38, 5, 0, 10, 0); s.icon(44, 11, 'l_mgl')
s.number(36, 25, 2, 1, 0); mode30(s, 25, 38, 42)
nav(s, [(23, 'icon_MEM', 'icon_abort')]); progbar(s, 60)
SCR['zcal_run'] = s

s = base('t_zero_cal')                                           # ゼロ校正 待機 (disp_MLSS_ZCal_3 msg=0)
s.number(38, 5, 0, 10, 0); s.icon(44, 11, 'l_mgl')
s.number(36, 25, 2, 1, 0); mode30(s, 25, 38, 42)
nav(s, [(23, 'icon_MEM', 'icon_l_cal_start')])
SCR['zcal_wait'] = s

s = base('t_zero_cal')                                           # 校正完了 (disp_MLSS_ZCal_3 msg=1)
s.icon(20, 11, 'msg_cal_comp')
nav(s, [(23, 'icon_MEM', 'icon_l_cal_start')])
SCR['zcal_comp'] = s

s = base('t_2p_cal')                                             # 2点校正 設定値入力 MLSS (disp_MLSS_SCal_1)
s.number(38, 5, 0, 7990, 0); s.icon(44, 11, 'l_mgl')
for x, n in zip((29, 32, 35, 38, 41), (0, 8, 0, 0, 0)):
    s.number(x, 18, 1, n, 0)
s.icon(29, 18, TABLE['b_m_num'][0])                              # 編集中桁 (10000の位) 反転
s.number(36, 25, 2, 1, 0); mode30(s, 25, 38, 42)
nav(s, [(13, 'icon_DISP', 'icon_l_return'), (18, 'icon_DISP', 'icon_change'), (23, 'icon_MEM', 'icon_next')])
SCR['scal_set'] = s

s = base('t_3p_cal')                                             # 3点校正 中間濃度設定 MLSS (disp_MLSS_SCal_1 three_p=1)
s.number(38, 5, 0, 4010, 0); s.icon(44, 11, 'l_mgl')
for x, n in zip((29, 32, 35, 38, 41), (0, 4, 0, 0, 0)):
    s.number(x, 18, 1, n, 0)
s.icon(29, 18, TABLE['b_m_num'][0])                              # 編集中桁 (10000の位) 反転
s.number(36, 25, 2, 1, 0); mode30(s, 25, 38, 42)
nav(s, [(13, 'icon_DISP', 'icon_l_return'), (18, 'icon_DISP', 'icon_change'), (23, 'icon_MEM', 'icon_next')])
SCR['mcal_set'] = s

def cal_wait(title, val, setval):                                # disp_MLSS_SCal_3 / MCal_3 (msg_flag=0) 待機画面
    s = base(title)
    s.number(38, 5, 0, val, 0); s.icon(44, 11, 'l_mgl')
    s.number(41, 18, 1, setval, 0)
    s.number(36, 25, 2, 1, 0); mode30(s, 25, 38, 42)
    nav(s, [(13, 'icon_DISP', 'icon_setting'), (18, 'icon_MEM', 'icon_cal_start'), (23, 'icon_MEM', 'icon_l_postcal')])
    return s

def cal_comp(title):                                             # disp_MLSS_SCal_3 / MCal_3 (msg_flag=1) 校正完了
    s = base(title); s.icon(20, 11, 'msg_cal_comp')
    nav(s, [(13, 'icon_DISP', 'icon_setting'), (18, 'icon_MEM', 'icon_cal_start'), (23, 'icon_MEM', 'icon_l_postcal')])
    return s

SCR['scal_wait'] = cal_wait('t_2p_cal', 7990, 8000)
SCR['scal_comp'] = cal_comp('t_2p_cal')
SCR['mcal_wait'] = cal_wait('t_3p_cal', 4010, 4000)
SCR['mcal_comp'] = cal_comp('t_3p_cal')

s = base('t_2p_cal')                                             # 2点校正 校正中 MLSS (disp_MLSS_SCal_2)
s.icon(3, 8, 'icon_wait1'); s.number(38, 5, 0, 7990, 0); s.icon(44, 11, 'l_mgl')
s.number(41, 18, 1, 8000, 0)
s.number(36, 25, 2, 1, 0); mode30(s, 25, 38, 42)
nav(s, [(23, 'icon_MEM', 'icon_abort')]); progbar(s, 45)
SCR['scal_run'] = s

s = base('t_3p_cal')                                             # 3点校正 校正中 MLSS (disp_MLSS_MCal_2)
s.icon(3, 8, 'icon_wait1'); s.number(38, 5, 0, 4010, 0); s.icon(44, 11, 'l_mgl')
s.number(41, 18, 1, 4000, 0)
s.number(36, 25, 2, 1, 0); mode30(s, 25, 38, 42)
nav(s, [(23, 'icon_MEM', 'icon_abort')]); progbar(s, 45)
SCR['mcal_run'] = s

s = base('t_reset')                                              # 校正リセット確認 MLSS No.21 (disp_C_RESET)
s.icon(18, 6, 'msg_corr_No'); s.number(36, 6, 2, 21, 0)
s.icon(18, 10, 'msg_corr_reset')
nav(s, [(18, 'icon_DISP', 'icon_return'), (23, 'icon_MEM', 'icon_decision')])
SCR['cal_reset'] = s

s = base('t_pcal_select')                                        # 後校正値選択 (disp_CAL_HSEL、行 = disp_cal_hsel_row)
nav(s, [(18, 'icon_DISP', 'icon_decision'), (23, 'icon_MEM', 'icon_next')])
hist_row(s, 16, 6, 1, 10, 21, 8, 11, 2340, 0.0)                   # 後校正記録は水深を保存しない (0 固定)
s.icon(16, 13, 'icon_line')
hist_row(s, 16, 14, 2, 9, 45, 8, 11, 1980, 0.0)
s.icon(16, 21, 'icon_line')
hist_row(s, 16, 22, 3, 15, 2, 8, 10, 2100, 0.0)
SCR['cal_hsel'] = s

# ---- 4章 設定 ---------------------------------------------------------------
s = base('t_setting_menu')                                       # 設定メニュー (disp_S_MENU)
nav(s, [(13, 'icon_DISP', 'icon_l_return'), (18, 'icon_DISP', 'icon_select'), (23, 'icon_MEM', 'icon_decision')])
s.icon(17, 7, 'b_settime_s'); s.icon(34, 7, 'w_reset_s')
s.icon(17, 17, 'w_info_s');   s.icon(34, 17, 'w_depth_setting')
SCR['menu_setting'] = s

s = Screen(); s.icon(0, 1, 't_set_time'); battery(s)             # 時刻設定 (disp_SETTIME)
nav(s, [(18, 'icon_DISP', 'icon_select'), (23, 'icon_MEM', 'icon_decision')])
s.icon(44, 10, 'b_plus'); s.icon(44, 16, 'w_minus')
s.icon(28, 21, 'w_set');  s.icon(39, 21, 'w_cancel')
caldate(s, 17, 5)
s.number(15, 9, 0, 1, 0); s.number(21, 9, 0, 0, 0)               # 10:30
s.icon(27, 9, 'l_colon')
s.number(31, 9, 0, 3, 0); s.number(37, 9, 0, 0, 0)
SCR['set_time'] = s

s = base('t_reset')                                              # 初期化確認 (disp_RESET msg=0)
nav(s, [(18, 'icon_DISP', 'icon_return'), (23, 'icon_MEM', 'icon_decision')])
s.icon(18, 6, 'msg_reset')
SCR['sys_reset'] = s

s = base('t_reset')                                              # 初期化完了 (disp_RESET msg=1)
nav(s, [(18, 'icon_DISP', 'icon_return')])
s.icon(18, 6, 'msg_reset_comp')
SCR['sys_reset_comp'] = s

s = base('t_QR')                                                 # 製品情報 (disp_QR qr_flg=0)
nav(s, [(18, 'icon_DISP', 'icon_return')])
s.icon(18, 24, 'icon_info_err'); s.number(46, 24, 2, 0, 0); s.icon(18, 27, 'icon_line')
SCR['info'] = s

# ---- 6-7章 ガイダンス / エラー ----------------------------------------------
s = Screen(); battery(s)                                         # ガイダンス「保管方法」(disp_GUIDE5)
s.icon(0, 1, 'gui_title5'); s.icon(0, 5, 'gui_point'); s.icon(16, 6, 'gui_message5')
s.icon(0, 19, 'icon_DISP'); s.icon(5, 19, 'icon_hidden')
s.icon(0, 24, 'icon_MEM');  s.icon(5, 24, 'icon_finish')
SCR['guide_store'] = s

# エラー画面は Display.c の error1_table / error2_table をそのまま写し、
# common/LS027_Display.c の disp_error_page1 / disp_error_page2 の配置で描画する。
# QR コード (qrcode_err_disp) の中身は省略し err_QR 枠のみ。

SYS_ERRNO = 90                     # ERROR6 併記の本体エラー番号 代表値 (90=EEPROM異常)

def err6_1_extra(s):                                             # Display.c error6_1_extra
    s.icon(15, 13, 'm_No'); s.number(24, 13, 1, SYS_ERRNO, 0)

def err6_2_extra(s):                                             # Display.c error6_2_extra
    s.icon(14, 14, 'm_No')

def error_page1(title, message, icon, show_proc, show_rel, extra=None):
    s = Screen(); battery(s)
    s.icon(0, 1, title); s.icon(0, 6, message)
    s.icon(29, 7, 'err_QR'); s.icon(16, 20, icon)
    if show_proc:
        s.icon(0, 19, 'icon_DISP'); s.icon(5, 19, 'icon_procedure')
    if show_rel:
        s.icon(0, 24, 'icon_MEM');  s.icon(5, 24, 'icon_release')
    if extra:
        extra(s)
    return s

def error_page2(title, guide, show_qr, show_rel, extra=None):
    s = Screen(); battery(s)
    s.icon(0, 1, title); s.icon(16, 6, guide)
    if show_qr:
        s.icon(0, 19, 'icon_DISP'); s.icon(5, 19, 'icon_QR_disp')
    if show_rel:
        s.icon(0, 24, 'icon_MEM');  s.icon(5, 24, 'icon_release')
    if extra:
        extra(s)
    return s

# Display.c error1_table: (key, title, message, icon, show_procedure, show_release, extra)
ERR1_TABLE = [
    ('e01', 'err_title3',  'err_message3',  'err_icon3', 1, 1, None),   # ERROR1 (暫定 ERROR3 と同一文面)
    ('e02', 'err_title3',  'err_message3',  'err_icon3', 1, 1, None),   # ERROR2 (同上)
    ('e03', 'err_title3',  'err_message3',  'err_icon3', 1, 1, None),   # ERROR3
    ('e04', 'err_title4',  'err_message4',  'err_icon3', 1, 1, None),   # ERROR4 (No.4-6 共用)
    ('e05', 'err_title5',  'err_message5',  'err_icon5', 1, 1, None),   # ERROR5 (未到達)
    ('e06', 'err_title6',  'err_message6',  'err_icon6', 0, 1, err6_1_extra),  # ERROR6 手順表示なし
    ('e07', 'err_title7',  'err_message7',  'err_icon6', 1, 0, None),   # ERROR7 解除なし
    ('e08', 'err_title8',  'err_message8',  'err_icon5', 1, 1, None),   # ERROR8
    ('e09', 'err_title9',  'err_message9',  'err_icon5', 1, 0, None),   # ERROR9 解除なし
    ('e17', 'err_title17', 'err_message3',  'err_icon3', 1, 1, None),   # ERROR17 (未到達)
    ('e19', 'err_title19', 'err_message19', 'err_icon6', 0, 1, None),   # ERROR19 (未到達)
]

# Display.c error2_table: (key, title, guide, show_qr_disp, show_release, extra)
ERR2_TABLE = [
    ('e01', 'err_title3',  'err_guide3',  1, 1, None),
    ('e02', 'err_title3',  'err_guide3',  1, 1, None),
    ('e03', 'err_title3',  'err_guide3',  1, 1, None),
    ('e04', 'err_title4',  'err_guide4',  1, 1, None),
    ('e05', 'err_title5',  'err_guide4',  1, 1, None),
    ('e06', 'err_title6',  'err_guide6',  0, 1, err6_2_extra),          # page1 に DISP 無し = 未到達
    ('e07', 'err_title7',  'err_guide7',  1, 0, None),
    ('e08', 'err_title8',  'err_guide4',  1, 1, None),
    ('e09', 'err_title9',  'err_guide4',  1, 0, None),
    ('e17', 'err_title17', 'err_guide17', 1, 1, None),
    ('e19', 'err_title19', 'err_guide3',  1, 1, None),                  # 同上 未到達
]

# ページ2 は ERRWAIT1 の DISP でのみ到達する。page1 の show_procedure=0 の画面
# (ERROR6 / ERROR19) はページ2 が定義だけで到達不能なため生成しない。
P1_HAS_PROC = {k: p for k, _, _, _, p, _, _ in ERR1_TABLE}

# ERRDSP1 (Normal.c) の switch から呼ばれる = 実機で表示できる画面のみ生成する。
# ERROR5 は disp_ERROR5_1/_2 が存在するが switch に case が無く、ERROR17 / ERROR19 は
# 描画関数すら無い (テーブル行と素材だけが ID-200T から残った残骸)。
REACHABLE = {'e01', 'e02', 'e03', 'e04', 'e06', 'e07', 'e08', 'e09'}

for k, title, message, icon, sp, sr, ex in ERR1_TABLE:
    if k not in REACHABLE:
        continue
    SCR['err_%s_p1' % k] = error_page1(title, message, icon, sp, sr, ex)
for k, title, guide, sq, sr, ex in ERR2_TABLE:
    if k in REACHABLE and P1_HAS_PROC[k]:
        SCR['err_%s_p2' % k] = error_page2(title, guide, sq, sr, ex)

# ---- 出力 -------------------------------------------------------------------
os.makedirs(OUT, exist_ok=True)
for k, s in sorted(SCR.items()):
    s.save_png(f'{OUT}/{k}.png'); s.save_bmp(f'{OUT}/{k}.bmp')
print('generated:', len(SCR))
