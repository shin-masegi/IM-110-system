// ===== Normal.c : case CAL_HSEL_2 (L513) =====
	case CAL_HSEL_2:
		//記録選択画面表示
		operation_mode = CAL_HSEL_3;
		flash_calc(FLASH_BAT, NULL, NULL, NULL, 0, 1, NULL);
		page_start = (cal_hsel_cursor / 3) * 3;
		if (disp_CAL_HSEL(page_start, cal_hsel_cursor, cal_hsel_count, fl_flag, bar_flag) == DISP_OK)
		{
			fl_flag ^= 1;
			timer_set(&disp_timer, DISP_CYCLE);
		}
		break;
