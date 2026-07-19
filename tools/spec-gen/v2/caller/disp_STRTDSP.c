// ===== Normal.c : case CALDSP_1 (L1030) =====
	case CALDSP_1:
		//起動画面表示
		if (disp_STRTDSP() == DISP_OK)
		{
			timer_set(&lcd_off_timer, 70);
		}
		//スイッチ動作
		if (MEM_sw_check() >= 3)
		{
			MEM_sw_step = 0;
		}
		else if (DISP_sw_check() >= 3)
		{
			DISP_sw_step = 0;
		}
		operation_mode = CALDSP_2;


// ===== Normal.c : case STRT_1 (L2528) =====
	case STRT_1:
		do_range_set(now_range);
		//起動画面表示
		if (disp_STRTDSP() == DISP_OK)
		{
			timer_set(&range_hold_timer, 50);
			timer_set(&lcd_off_timer, 10);
			operation_mode = STRT_4;
			//ロガーデータ読込とエラー処理
			for (i = 0; i <= LOG_SEL_MAX; i++)
			{
				if (eep_read_logh(i) != EEP_OK)
				{
					f_IC_err |= ERR_EEPROM;
				}
			}
		}
		break;

		//ID-160Tに合わせて起動画面に操作説明追加 2021/08/19 柵木

// ===== Setting.c : case SETDSP_1 (L248) =====
	case SETDSP_1:		//起動画面表示
		if (disp_STRTDSP() == DISP_OK)
		{
			timer_set(&lcd_off_timer, 70);
		}
		operation_mode = SETDSP_2;
		break;
