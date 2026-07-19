// ===== Normal.c : case C_S_CORR_2 (L697) =====
	case C_S_CORR_2:		//相関式設定画面2 (dead code: 現在到達しない経路。新 disp シグネチャに追従のみ)
		operation_mode = C_S_CORR_3;
		flash_calc(FLASH_BAT, NULL, NULL, NULL, 0, 1, NULL);
		if (disp_Corr_setting_30(corr_sel + 1, 0, bar_flag, fl_flag) == DISP_OK)
		{
			fl_flag ^= 1;
			timer_set(&disp_timer, DISP_CYCLE);
		}

		break;

// ===== Normal.c : case S_CORR_2 (L1914) =====
	case S_CORR_2:		//相関式設定画面2 (表示)
		operation_mode = S_CORR_3;
		flash_calc(FLASH_BAT, NULL, NULL, NULL, 0, 1, NULL);
		if (disp_Corr_setting_30(num[0] * 10 + num[1], corr_num_sel, bar_flag, fl_flag) == DISP_OK)
		{
			fl_flag ^= 1;
			timer_set(&disp_timer, DISP_CYCLE);
		}

		break;