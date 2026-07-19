// ===== Normal.c : case ZCAL_8 (L1123) =====
	case ZCAL_8:
		if (disp_MLSS_ZCal_3(fl_flag, 0, bar_flag) == DISP_OK)
		{
			fl_flag ^= 1;
			timer_set(&flash_timer, DISP_CYCLE);
			operation_mode = ZCAL_2;
		}
		break;

// ===== Normal.c : case ZCAL_5 (L1256) =====
	case ZCAL_5:
		//ゼロ校正完了表示処理（Good）
		if (disp_MLSS_ZCal_3(fl_flag, 1, bar_flag) == DISP_OK)
		{
			timer_set(&lcd_off_timer, 60);
			operation_mode = ZCAL_6;
			fl_flag ^= 1;
		}
		break;


// ===== Normal.c : case ZCAL_9 (L1274) =====
	case ZCAL_9:
		if (disp_MLSS_ZCal_3(fl_flag, 0, bar_flag) == DISP_OK)
		{
			fl_flag ^= 1;
			operation_mode = ZCAL_7;
		}
		break;
