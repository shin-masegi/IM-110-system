// ===== Normal.c : case C_MODE_2 (L770) =====
	case C_MODE_2:		//校正モード選択画面表示
		operation_mode = C_MODE_3;
		flash_calc(FLASH_BAT, NULL, NULL, NULL, 0, 1, NULL);
		if (disp_CAL_SETTING(cal_setting_sel, bar_flag, fl_flag) == DISP_OK)
		{
			fl_flag ^= 1;
			timer_set(&disp_timer, DISP_CYCLE);
		}

		break;