// ===== Normal.c : case C_RESET_1 (L90) =====
	case C_RESET_1:		//リセット前画面表示
		if (disp_C_RESET(0, corr_sel + 1, bar_flag) == DISP_OK)
		{
			timer_set(&flash_timer, DISP_CYCLE);
			operation_mode = C_RESET_2;
		}
		break;

// ===== Normal.c : case C_RESET_3 (L110) =====
	case C_RESET_3:		//リセット後画面表示
		if (disp_C_RESET(1, corr_sel + 1, bar_flag) == DISP_OK)
		{
			timer_set(&flash_timer, DISP_CYCLE);
			timer_set(&lcd_off_timer, 60);
			operation_mode = C_RESET_4;
		}
		break;