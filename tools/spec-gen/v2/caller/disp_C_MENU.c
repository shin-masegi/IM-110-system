// ===== Normal.c : case C_MENU_2 (L855) =====
	case C_MENU_2:		//校正メニュー画面表示
		operation_mode = C_MENU_3;
		flash_calc(FLASH_BAT, NULL, NULL, NULL, 0, 1, NULL);
		if (disp_C_MENU(cal_setting_sel, cal_menu_sel, bar_flag, fl_flag) == DISP_OK)
		{
			fl_flag ^= 1;
			timer_set(&disp_timer, DISP_CYCLE);
		}

		break;