// ===== Normal.c : case LOG_5 (L2968) =====
	case LOG_5:
		//ロガー測定中画面表示
		if ((lcd_on_flag) && (!wakeup_flag))
		{
			normal_disp(&dspdo, &dspTemp);
			if (disp_LOG3(WAG_flash_flag, dspdo, DOAcc, dspTemp, prog_bar, bar_flag, bt_connect_flag, LOG_STOP_flag) == DISP_OK)
			{
				timer_set(&disp_timer, DISP_CYCLE);
				WAG_flash_flag ^= 1;
				disp_colon_flag = !disp_colon_flag;
			}
			else{
				//何もしない
			}
		}
		operation_mode = LOG_6;
		break;
