// ===== Setting.c : case RESET_1 (L69) =====
		case RESET_1:		//リセット前画面表示
			if (disp_RESET(0, bar_flag) == DISP_OK)
			{
				timer_set(&flash_timer, DISP_CYCLE);
				operation_mode = RESET_2;
			}
			break;

// ===== Setting.c : case RESET_3 (L107) =====
		case RESET_3:		//リセット後画面表示
			if (disp_RESET(1, bar_flag) == DISP_OK)
			{
				timer_set(&flash_timer, DISP_CYCLE);
				timer_set(&lcd_off_timer, 60);
				operation_mode = RESET_4;
			}
			break;