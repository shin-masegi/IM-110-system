// ===== Setting.c : case MENU2_2 (L332) =====
	case MENU2_2:		//設定メニュー2画面待機中
		operation_mode = (operation_mode == MENU1_2) ? MENU1_3 : MENU2_3;
		flash_calc(FLASH_BAT, NULL, NULL, NULL, 0, 0, NULL);
		if (disp_S_MENU(cur_sel_item, bar_flag, fl_flag) == DISP_OK)
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
