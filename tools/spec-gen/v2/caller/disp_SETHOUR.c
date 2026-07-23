// ===== Adjust.c : case SETHOUR_2 (L1613) =====
	case SETHOUR_2:		//時刻設定前表示
		fl_flag2 = 0;
		if (disp_SETHOUR(cur_sel_item2, bar_flag, fl_flag2, 0) == DISP_OK)
		{
			operation_mode = SETHOUR_3;
		}
		break;


// ===== Adjust.c : case SETHOUR_5 (L1646) =====
	case SETHOUR_5:		//時刻設定表示
		operation_mode = SETHOUR_6;
		if (disp_timer == 0)
		{
			timer_set(&disp_timer, DISP_CYCLE);
			if (disp_SETHOUR(cur_sel_item2, bar_flag, fl_flag2, 1) == DISP_OK)
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
