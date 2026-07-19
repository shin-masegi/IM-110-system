// ===== Normal.c : case ADCAL_8 (L1600) =====
	case ADCAL_8:
		if (disp_MLSS_SCal_3(fl_flag, 0, bar_flag) == DISP_OK)
		{
			timer_set(&flash_timer, DISP_CYCLE);
			operation_mode = ADCAL_2;
			fl_flag ^= 1;
		}
		break;

// ===== Normal.c : case ADCAL_5 (L1726) =====
	case ADCAL_5:
		//スパン校正完了表示処理（Good）
		if (disp_MLSS_SCal_3(fl_flag, 1, bar_flag) == DISP_OK)
		{
			timer_set(&lcd_off_timer, 60);
			operation_mode = ADCAL_6;
		}
		break;


// ===== Normal.c : case ADCAL_9 (L1743) =====
	case ADCAL_9:
		if (disp_MLSS_SCal_3(fl_flag, 0, bar_flag) == DISP_OK)
		{
			operation_mode = ADCAL_7;
		}
		break;
