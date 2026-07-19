// ===== Normal.c : case MSR_2 (L2243) =====
	case MSR_2:
		//測定中画面表示（安定待ち）
		if (!disp_timer)
		{
			//表示用数値計算と点滅制御
			flash_calc(FLASH_ALL, 0, 0, 0, 0, 0, NULL);
			//測定中画面表示
			if (disp_MLSS_Meas_2(WAG_flash_flag, get_stable_prog(), bar_flag) == DISP_OK)
			{
				//SendSerialData((uint8_t *)"MLSS_Meas2\r\n");
				timer_set(&disp_timer, DISP_CYCLE);
				WAG_flash_flag ^= 1;
			}
		}
		operation_mode = MSR_1;
		break;
