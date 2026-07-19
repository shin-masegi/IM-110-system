// ===== Normal.c : case LOG_C (L3298) =====
	case LOG_C:	//ロガー測定切替メッセージ表示
		flash_calc(FLASH_BAT, NULL, NULL, NULL, 0, 1, NULL);
		if (disp_LOG5(bar_flag) == DISP_OK)
		{
			timer_set(&lcd_off_timer, 60);
			timer_set(&flash_timer, DISP_CYCLE);
			operation_mode = LOG_D;
		}
		break;
