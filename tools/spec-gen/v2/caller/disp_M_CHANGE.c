// ===== Normal.c : case M_CHANGE_1 (L2136) =====
	case M_CHANGE_1:		//測定モード切替画面表示
		operation_mode = M_CHANGE_2;
		flash_calc(FLASH_BAT, NULL, NULL, NULL, 0, 1, NULL);
		if (disp_M_CHANGE(Meas_Mode, 0, bar_flag, fl_flag) == DISP_OK)
		{
			if (fl_flag == 0)
			{
				fl_flag = 1;
			}
			else
			{
				fl_flag = 0;
			}
			timer_set(&disp_timer, DISP_CYCLE);
		}

		break;

// ===== Normal.c : case M_CHANGE_3 (L2178) =====
	case M_CHANGE_3:		//セット後のメッセージ表示
		flash_calc(FLASH_BAT, NULL, NULL, NULL, 0, 1, NULL);
		if (disp_M_CHANGE(Meas_Mode, Meas_Mode + 1, bar_flag, fl_flag) == DISP_OK)
		{
			timer_set(&lcd_off_timer, 60);
			timer_set(&flash_timer, DISP_CYCLE);
			//モード切替処理
			Change_Meas_Mode(Meas_Mode);
			//切替後の測定モードに対応する履歴バンクを his[] に読み込む
			eep_read_history();
			reset_stable();	//安定判断リセット
			operation_mode = M_CHANGE_4;
		}
		break;