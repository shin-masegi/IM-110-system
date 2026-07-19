// ===== Setting.c : case ACALSET_1 (L624) =====
		case ACALSET_1:		//自動校正画面表示
			operation_mode = ACALSET_2;
			flash_calc(FLASH_BAT, NULL, NULL, NULL, 0, 1, NULL);
			if (disp_ACALSET(AutoCal_time_sel, 0, bar_flag, fl_flag) == DISP_OK)
			{
				if (fl_flag == 0)
				{
					fl_flag = 1;
				}
				else
				{
					fl_flag = 0;
				}
				timer_set(&disp_timer, DISP_CYCLE);
			}

			break;

// ===== Setting.c : case ACALSET_3 (L665) =====
		case ACALSET_3:		//セット後のメッセージ表示
			flash_calc(FLASH_BAT, NULL, NULL, NULL, 0, 1, NULL);
			if (disp_ACALSET(AutoCal_time_sel, AutoCal_time_sel + 1, bar_flag, fl_flag) == DISP_OK)
			{
				timer_set(&lcd_off_timer, 60);
				timer_set(&flash_timer, DISP_CYCLE);
				operation_mode = ACALSET_4;
			}
			break;