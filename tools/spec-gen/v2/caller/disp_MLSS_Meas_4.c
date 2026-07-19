// ===== Normal.c : case MSR_8 (L2344) =====
	case MSR_8:	//通常測定切替メッセージ表示
		flash_calc(FLASH_BAT, NULL, NULL, NULL, 0, 1, NULL);
		if (disp_MLSS_Meas_4(bar_flag) == DISP_OK)
		{
			//SendSerialData((uint8_t *)"MLSS_Meas4\r\n");
			timer_set(&lcd_off_timer, 60);
			timer_set(&flash_timer, DISP_CYCLE);
			operation_mode = MSR_9;
		}
		break;
