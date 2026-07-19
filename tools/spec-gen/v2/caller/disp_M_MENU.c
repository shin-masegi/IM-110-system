// ===== Normal.c : case M_MENU_2 (L2021) =====
	case M_MENU_2:		//測定モード切替画面表示
		operation_mode = M_MENU_3;
		flash_calc(FLASH_BAT, NULL, NULL, NULL, 0, 1, NULL);
		if (disp_M_MENU(meas_menu_sel, bar_flag, fl_flag) == DISP_OK)
		{
			fl_flag ^= 1;
			timer_set(&disp_timer, DISP_CYCLE);
		}

		break;