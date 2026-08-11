"""ADBOAD (基板調整モード) 全画面図を実装コードどおりに生成する。
   描画元: Adjust.c (画面別の値/単位) + Display.c (disp_ADBOAD_PROBE/PROBE2/SHIP, disp_PRGVER 他)。"""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from lcdemu import Screen, ICON

OUT = sys.argv[1] if len(sys.argv) > 1 else '.'
TITLE = ['t_MLSS_LED_PWM','t_MLSS_A','t_MLSS_Ref_A','t_SS_LED_PWM','t_SS_A','t_SS_Ref_A',
         't_MLSS_Ref_05','t_MLSS_Ref_20','t_MLSS_Ref_35','t_SS_Ref_05','t_SS_Ref_20','t_SS_Ref_35',
         't_MLSS_0','t_MLSS_8000','t_MLSS_4000','t_SS_0','t_SS_1000','t_SS_500',
         't_TR_100','t_TR_90','t_TR_60','t_TR_30','t_Depth_000','t_Depth_600']   # scr 4..27 (90cm=23 挿入)

def sw(s, l18, l23):                       # 左列スイッチ凡例 (DISP/MEM)
    s.icon(0,18,'icon_DISP'); s.icon(0,23,'icon_MEM')
    s.icon(5,18,l18);         s.icon(5,23,l23)

def probe(scr, val, digit, unit, over=0):  # disp_ADBOAD_PROBE_ex
    s = Screen(); s.icon(0,1,TITLE[scr-4])
    if unit == 'mv': s.number(36,6,0,val,digit)
    else:            s.number(38,5,0,val,digit)
    if over: s.icon(3,8,'icon_over')
    {'mv':lambda:s.icon(43,14,'m_mVoltage'), 'mgl':lambda:s.icon(44,11,'l_mgl'),
     'cm':lambda:s.icon(44,11,'m_cm'), 'm':lambda:s.icon(45,11,'m_m'), '':lambda:None}[unit]()
    sw(s,'icon_next','icon_start'); return s

def probe2(scr, juko, ref):                # disp_ADBOAD_PROBE2_ex (受光/Ref 2値)
    s = Screen(); s.icon(0,1,TITLE[scr-4])
    s.number(36,4,0,juko,0);  s.icon(43,12,'m_mVoltage')
    s.number(36,16,0,ref,0);  s.icon(43,24,'m_mVoltage')
    sw(s,'icon_next','icon_start'); return s

def ship(scr, mode, live, setval, over=0): # disp_ADBOAD_SHIP (ライブ値+設定値)
    s = Screen(); s.icon(0,1,TITLE[scr-4])
    if mode == 'tr':
        if over: s.icon(3,8,'icon_over')
        s.number(38,5,0,live,1); s.icon(44,11,'m_cm');  s.number(41,18,1,setval,1)
    else:
        s.number(38,5,0,live,0); s.icon(44,11,'l_mgl'); s.number(41,18,1,setval,0)
    sw(s,'icon_next','icon_start'); return s

SCR = {}
# 1-3: プログラムVer / EEPROMテスト / 電池電圧
s = Screen(); s.icon(0,0,'t_prog_Ver'); s.number(36,9,0,0.42,2)
s.icon(0,17,'icon_DISP'); s.icon(5,17,'icon_next'); s.icon(0,22,'icon_MEM'); s.icon(5,22,'icon_l_erase')
SCR['01'] = s
s = Screen(); s.icon(0,1,'t_EEP'); s.icon(0,18,'icon_DISP'); s.icon(5,18,'icon_next')
s.icon(0,23,'icon_MEM'); s.icon(5,23,'icon_start'); SCR['02'] = s
s = Screen(); s.icon(0,1,'t_bat_vol'); s.number(36,10,0,3.62,2); s.icon(43,18,'m_Voltage')
s.icon(0,18,'icon_DISP'); s.icon(5,18,'icon_next'); SCR['03'] = s
# 4-15: LED PWM / 空中調整 / 温度補正
SCR['04'] = probe2(4, 1750.0, 1362.0);  SCR['05'] = probe(5, 1433.0, 0, 'mv')
SCR['06'] = probe(6, 1358.0, 0, 'mv');  SCR['07'] = probe2(7, 1750.0, 1361.0)
SCR['08'] = probe(8, 1433.0, 0, 'mv');  SCR['09'] = probe(9, 1362.0, 0, 'mv')
for i, (scr, j, r) in enumerate([(10,1802.0,1421.0),(11,1750.0,1362.0),(12,1698.0,1305.0),
                                 (13,1795.0,1418.0),(14,1750.0,1361.0),(15,1704.0,1303.0)]):
    SCR[f'{scr:02d}'] = probe2(scr, j, r)
# 16-27: 出荷時3点/4点調整 / 水深 (2026-08-11 透視度 90cm=23 挿入で以降繰下げ)
SCR['16'] = probe(16, 0.0, 0, 'mgl');        SCR['17'] = ship(17,'mlss', 8000.0, 8000.0)
SCR['18'] = ship(18,'mlss', 4000.0, 4000.0); SCR['19'] = probe(19, 0.0, 0, 'mgl')
SCR['20'] = ship(20,'ss', 1000.0, 1000.0);   SCR['21'] = ship(21,'ss', 500.0, 500.0)
SCR['22'] = probe(22, 100.0, 1, 'cm', over=1)
SCR['23'] = ship(23,'tr', 90.0, 90.0)
SCR['24'] = ship(24,'tr', 60.0, 60.0);       SCR['25'] = ship(25,'tr', 30.0, 30.0)
SCR['26'] = probe(26, 0.0, 2, 'm');          SCR['27'] = probe(27, 6.0, 2, 'm')
# 28-30: 日時調整 (disp_SETYEAR / disp_SETDAYS / disp_SETHOUR 準拠。図は編集前状態 = 次へ/開始 凡例)
def dt(title, sel_icons, nums):
    s = Screen(); s.icon(0,1,title)
    s.icon(0,18,'icon_DISP'); s.icon(0,23,'icon_MEM')
    s.icon(5,18,'icon_next'); s.icon(5,23,'icon_start')
    for x,y,ic in sel_icons: s.icon(x,y,ic)
    for x,y,v in nums: s.number(x,y,0,v,0)
    return s
#桁ごとの +/- は 4 桁分 (x=16/22/32/38, y=2/18)、中止/セット は y=22 (年画面のみ y=21)
PM4 = [(16,2,'w_plus'),(16,18,'w_minus'),(22,2,'w_plus'),(22,18,'w_minus'),
       (32,2,'w_plus'),(32,18,'w_minus'),(38,2,'w_plus'),(38,18,'w_minus')]
SCR['28'] = dt('t_set_year', [(44,10,'w_plus'),(44,16,'w_minus'),(28,21,'w_cancel'),(39,21,'w_set')],
               [(37,9,2026.0)])
SCR['29'] = dt('t_set_days', PM4 + [(28,22,'w_cancel'),(39,22,'w_set'),(28,6,'l_slash')],
               [(16,6,0.0),(22,6,8.0),(32,6,1.0),(38,6,1.0)])          # 08/11
SCR['30'] = dt('t_set_time', PM4 + [(28,22,'w_cancel'),(39,22,'w_set'),(28,6,'l_colon')],
               [(16,6,1.0),(22,6,0.0),(32,6,3.0),(38,6,0.0)])          # 10:30
for k, s in sorted(SCR.items()):
    s.save_png(f'{OUT}/adboad_{k}.png'); s.save_bmp(f'{OUT}/adboad_{k}.bmp')
print('generated:', len(SCR))
