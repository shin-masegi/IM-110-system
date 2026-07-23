// ===== Adjust.c : case SETDAYS_2 (L1413) =====
	case SETDAYS_2:		//日付設定前表示
		fl_flag2 = 0;
		if (disp_SETDAYS(cur_sel_item2, bar_flag, fl_flag2, 0) == DISP_OK)
		{
			operation_mode = SETDAYS_3;
		}
		break;


// ===== Adjust.c : case SETDAYS_5 (L1467) =====
	case SETDAYS_5:		//日付設定表示
		operation_mode = SETDAYS_6;
		if (disp_timer == 0)
		{
			timer_set(&disp_timer, DISP_CYCLE);
			if (disp_SETDAYS(cur_sel_item2, bar_flag, fl_flag2, 1) == DISP_OK)
			{
				if (fl_flag2 == 0)
				{
					fl_flag2 = 1;
				}
				else
				{
					fl_flag2 = 0;
				}
			}
		}
		break;
