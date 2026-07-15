# GDB 初期化 — IM-110 調整機能 計算式デバッグ (adjust-mode-design.md ★A)
# 使い方:
#   arm-none-eabi-gdb ../IM-110/build/IM-110.elf -x tools/adjust-debug/adj.gdb
#   (openocd を別 shell で先に起動しておく)
#
# 方針: 測定ループの MLSS_FABSS_from_raw_mv は ~2Hz で頻繁に呼ばれるので常時ブレークしない。
#       A コマンド (シリアル) で叩かれる fit/adjust 関数にだけブレークを張り、算出の途中値を覗く。
#       ホットパスは必要時に stepmlss/stepss/steptr で 1 サイクルだけ捕まえる。

set pagination off
set print pretty on
set confirm off
target remote :3333

# ---- グローバル観測マクロ ----
define adcs
  printf "Meas_Mode=%d  MLSS_MODE=%d SS_MODE=%d TR_MODE=%d\n", Meas_Mode, MLSS_MODE, SS_MODE, TR_MODE
  printf "ADC_mV     [J0 R1 J2 R3 J4 D5]: %.2f %.2f %.2f %.2f %.2f %.2f\n", \
    ADC_mV[0],ADC_mV[1],ADC_mV[2],ADC_mV[3],ADC_mV[4],ADC_mV[5]
  printf "ADC_mV_ave                    : %.2f %.2f %.2f %.2f %.2f %.2f\n", \
    ADC_mV_ave[0],ADC_mV_ave[1],ADC_mV_ave[2],ADC_mV_ave[3],ADC_mV_ave[4],ADC_mV_ave[5]
end
document adcs
生 ADC / 移動平均と現モード・相関式 No. を一覧
end

# MLSS の校正状態 (live 係数 + vault staging)
define mlssv
  printf "MLSS: ZR=%.3f ADZR=%.3f  MLSS_ABSS=%.5f MLSS_FABSS=%.4f MLSS=%.3f\n", \
    MLSS_ZR, MLSS_ADZR, MLSS_ABSS, MLSS_FABSS, MLSS
  printf "  live Mode_CF[%d] = ", MLSS_MODE
  p MLSS_Mode_CF[MLSS_MODE]
  printf "  vault ModeCF[%d]= ", MLSS_MODE-20
  p MLSS_Vault_ModeCF[MLSS_MODE-20]
  printf "  vault ADZR[juko,refZR]=%.3f,%.3f  Kiza=%.5f\n", \
    MLSS_Vault_ADZR[0], MLSS_Vault_ADZR[1], MLSS_Vault_Kiza[MLSS_MODE-20]
  printf "  vault TempC(B,B2..)= "
  p MLSS_Vault_TempC
  printf "  SP_A/B/C = %.4g %.4g %.4g\n", MLSS_SP_A, MLSS_SP_B, MLSS_SP_C
end
document mlssv
MLSS の live/vault 校正係数と中間値を一括表示
end

define ssv
  printf "SS: ZR=%.3f ADZR=%.3f  SS_ABSS=%.5f SS_FABSS=%.4f SS=%.3f\n", \
    SS_ZR, SS_ADZR, SS_ABSS, SS_FABSS, SS
  printf "  vault ModeCF[%d]= ", SS_MODE-20
  p SS_Vault_ModeCF[SS_MODE-20]
  printf "  vault ADZR=%.3f,%.3f Kiza=%.5f  TempC[0]=%.5g\n", \
    SS_Vault_ADZR[0], SS_Vault_ADZR[1], SS_Vault_Kiza[SS_MODE-20], SS_Vault_TempC[0]
  printf "  SP_A/B/C = %.4g %.4g %.4g\n", SS_SP_A, SS_SP_B, SS_SP_C
end
document ssv
SS の live/vault 校正係数と中間値
end

define trv
  printf "TR: ZR=%.3f ADZR=%.3f  TR_ABSS=%.5f TR_FABSS=%.4f Transparency=%.3f\n", \
    TR_ZR, TR_ADZR, TR_ABSS, TR_FABSS, Transparency
  printf "  vault ModeCF[a,b,..]= "
  p TR_Vault_ModeCF
  printf "  vault Kiza=%.5f  (refZR/B は SS 参照)\n", TR_Vault_Kiza
  printf "  SP_A/B/C = %.4g %.4g %.4g\n", TR_SP_A, TR_SP_B, TR_SP_C
end
document trv
透視度の live/vault 校正係数と中間値 (累乗 a,b)
end

# Mode_CF 回帰の捕捉点 (adj_x=ABS, adj_y=基準器)。AMD コマンドと同内容を機上で確認
define caps
  printf "capture: n=%d mode=%d (0xFF=空)\n", adj_n, adj_buf_mode
  set $i = 0
  while $i < adj_n
    printf "  [%d] ABS=%.6f  Y=%.3f\n", $i, adj_x[$i], adj_y[$i]
    set $i = $i + 1
  end
  printf "tempc capture: set=0x%x mode=%d  ref[5/20/35]=%.2f/%.2f/%.2f juko=%.2f/%.2f/%.2f\n", \
    adj_tc_set, adj_tc_mode, adj_tc_ref[0],adj_tc_ref[1],adj_tc_ref[2], \
    adj_tc_juko[0],adj_tc_juko[1],adj_tc_juko[2]
end
document caps
Mode_CF/温度補正の捕捉バッファ (adj_x/adj_y/adj_tc_*) を表示
end

# ---- ブレークポイント ----
# fit/adjust 関数 (A コマンドで叩かれる = 低頻度)。算出の入口で止めて引数・途中値を見る
define armfit
  break adj_fit_poly
  break adj_fit_power
  break adj_calc_tempc
  break adj_calc_kiza
  break Adj_ModeCF_FitPoly
  break Adj_ModeCF_FitPower
  break Adj_TempC_Fit
  break Adj_CaptureADZR
  break Adj_Kiza
  break Adj_LED_Auto
  break Apply_Vault_To_Live
  printf "armed: fit/adjust breakpoints\n"
end
document armfit
A コマンドで呼ばれる算出/vault 反映関数にブレークを張る
end

# ホットパス (測定ループ) を 1 回だけ捕まえて式を step する
define stepmlss
  tbreak MLSS_FABSS_from_raw_mv
  continue
end
document stepmlss
次の 1 サイクルの MLSS 算出で停止 (I(0)/abss/fabss を step で追える)
end
define stepss
  tbreak SS_FABSS_from_raw_mv
  continue
end
define steptr
  tbreak TR_FABSS_from_raw_mv
  continue
end

printf "\n=== IM-110 adjust debug ===\n"
printf "macros: adcs / mlssv / ssv / trv / caps / armfit / stepmlss|stepss|steptr\n"
printf "attach 済 (running)。'monitor reset halt' でリセット停止も可。\n\n"
