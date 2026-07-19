// ===== Setting.c : case SETTIME_2 (L409) =====
	case SETTIME_2:		//時刻設定表示
		operation_mode = SETTIME_3;
		flash_calc(FLASH_BAT, NULL, NULL, NULL, 0, 0, NULL);
		if (disp_SETTIME(cur_sel_item2, bar_flag, fl_flag) == DISP_OK)
		{
			timer_set(&disp_timer, DISP_CYCLE);
			if (fl_flag == 0)
			{
				fl_flag = 1;
			}
			else
			{
				fl_flag = 0;
			}
		}
		break;
