// ===== Normal.c : case MCAL_8 (L1365) =====
	case MCAL_8:
		if (disp_MLSS_MCal_3(fl_flag, 0, bar_flag) == DISP_OK)
		{
			timer_set(&flash_timer, DISP_CYCLE);
			operation_mode = MCAL_2;
			fl_flag ^= 1;
		}
		break;


// ===== Normal.c : case MCAL_5 (L1490) =====
	case MCAL_5:
		//スパン校正完了表示処理（Good）
		if (disp_MLSS_MCal_3(fl_flag, 1, bar_flag) == DISP_OK)
		{
			timer_set(&lcd_off_timer, 60);
			operation_mode = MCAL_6;
		}
		break;


// ===== Normal.c : case MCAL_9 (L1507) =====
	case MCAL_9:
		if (disp_MLSS_MCal_3(fl_flag, 0, bar_flag) == DISP_OK)
		{
			operation_mode = MCAL_7;
		}
		break;
