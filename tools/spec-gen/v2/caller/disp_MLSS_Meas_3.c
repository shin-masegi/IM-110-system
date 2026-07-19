// ===== Normal.c : case MSR_7 (L2260) =====
	case MSR_7:
		//表示用数値計算と点滅制御
		if (disp_MLSS_Meas_3(WAG_flash_flag, 0, bar_flag) == DISP_OK)
		{
			//SendSerialData((uint8_t *)"MLSS_Meas3\r\n");
			WAG_flash_flag ^= 1;
			operation_mode = MSR_3;
		}
		break;


// ===== Normal.c : case MSR_5 (L2323) =====
	case MSR_5:
		//記録完了画面表示
		normal_disp(&dspdo, &dspTemp);
		if (disp_MLSS_Meas_3(WAG_flash_flag, 1, bar_flag) == DISP_OK)
		{
			//SendSerialData((uint8_t *)"MLSS_Meas3_MSR_5\r\n");
			//Ver.1.14より記録完了を2秒に変更 2021/07/27 柵木
			timer_set(&lcd_off_timer, 40);
			operation_mode = MSR_6;
		}
		break;
