// ===== Normal.c : case LOG_2 (L2860) =====
	case LOG_2:
		normal_disp(&dspdo, &dspTemp);
		if (disp_LOG1(dspdo, DOAcc, dspTemp, bar_flag, bt_connect_flag) == DISP_OK)
		{
			timer_set(&disp_timer, DISP_CYCLE);
			WAG_flash_flag ^= 1;
			operation_mode = LOG_3;
		}
		break;
