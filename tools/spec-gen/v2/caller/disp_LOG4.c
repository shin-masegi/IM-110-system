// ===== Normal.c : case LOG_A (L3219) =====
	case LOG_A:
		//データロガー測定完了画面表示
		normal_disp(&dspdo, &dspTemp);
		if (disp_LOG4(dspdo, DOAcc, dspTemp, prog_bar, bar_flag, bt_connect_flag) == DISP_OK)
		{
			timer_set(&disp_timer, DISP_CYCLE);
			WAG_flash_flag ^= 1;
			operation_mode = LOG_B;
		}
		break;
