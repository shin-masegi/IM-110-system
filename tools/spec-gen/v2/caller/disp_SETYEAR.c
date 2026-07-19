// ===== Adjust.c : case SETYEAR_2 (L1197) =====
	case SETYEAR_2:		//日付設定前画面表示
		fl_flag2 = 0;
		if (disp_SETYEAR(cur_sel_item2, bar_flag, fl_flag2, 0) == DISP_OK)
		{
			operation_mode = SETYEAR_3;
		}
		break;


// ===== Adjust.c : case SETYEAR_5 (L1232) =====
	case SETYEAR_5:		//日付設定表示
		operation_mode = SETYEAR_6;
		if (disp_timer == 0)
		{
			timer_set(&disp_timer, DISP_CYCLE);
			if (disp_SETYEAR(cur_sel_item2, bar_flag, fl_flag2, 1) == DISP_OK)
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
