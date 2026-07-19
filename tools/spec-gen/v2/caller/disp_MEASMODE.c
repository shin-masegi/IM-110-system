// ===== Setting.c : case MEASMODE_1 (L697) =====
		case MEASMODE_1:		//淡水/海水選択画面表示
			operation_mode = MEASMODE_2;
			flash_calc(FLASH_BAT, NULL, NULL, NULL, 0, 1, NULL);
			if (disp_MEASMODE(measure_mode_flag, 0, bar_flag, fl_flag) == DISP_OK)
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

// ===== Setting.c : case MEASMODE_3 (L735) =====
		case MEASMODE_3:		//セット後のメッセージ表示
			flash_calc(FLASH_BAT, NULL, NULL, NULL, 0, 1, NULL);
			if (disp_MEASMODE(measure_mode_flag, measure_mode_flag + 1, bar_flag, fl_flag) == DISP_OK)
			{
				timer_set(&lcd_off_timer, 60);
				timer_set(&flash_timer, DISP_CYCLE);
				operation_mode = MEASMODE_4;
			}
			break;