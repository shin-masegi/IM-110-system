// ===== Adjust.c : case CALTEMP_2 (L1121) =====
	case CALTEMP_2:		//1点調整開始前画面表示
		fl_flag2 = 0;
		if (disp_CALTEMP(cur_sel_item2, bar_flag, fl_flag2, 0, WTemp) == DISP_OK)
		{
			timer_set(&disp_timer, DISP_CYCLE);
			operation_mode = CALTEMP_3;
		}
		break;


// ===== Adjust.c : case CALTEMP_5 (L1156) =====
	case CALTEMP_5:		//水温1点調整表示
		operation_mode = CALTEMP_6;
		if (disp_timer == 0)
		{
			timer_set(&disp_timer, DISP_CYCLE);
			if (disp_CALTEMP(cur_sel_item2, bar_flag, fl_flag2, 1, dWTemp) == DISP_OK)
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
