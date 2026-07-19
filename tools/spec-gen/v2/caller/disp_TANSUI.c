// ===== Setting.c : case TANSUI_1 (L767) =====
		case TANSUI_1:		//淡水/海水選択画面表示
			operation_mode = TANSUI_2;
			flash_calc(FLASH_BAT, NULL, NULL, NULL, 0, 1, NULL);
			if (disp_TANSUI(tansui_sw_flag, 0, bar_flag, fl_flag) == DISP_OK)
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

// ===== Setting.c : case TANSUI_3 (L805) =====
		case TANSUI_3:		//セット後のメッセージ表示
			flash_calc(FLASH_BAT, NULL, NULL, NULL, 0, 1, NULL);
			if (disp_TANSUI(tansui_sw_flag, tansui_sw_flag + 1, bar_flag, fl_flag) == DISP_OK)
			{
				timer_set(&lcd_off_timer, 60);
				timer_set(&flash_timer, DISP_CYCLE);
				operation_mode = TANSUI_4;
			}
			break;