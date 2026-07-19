// ===== Normal.c : case D_SET_2 (L625) =====
	case D_SET_2:		//スパン校正値設定画面表示
		operation_mode = D_SET_3;
		flash_calc(FLASH_BAT, NULL, NULL, NULL, 0, 1, NULL);
		if (disp_Depth_Setting(fl_flag, Interface_Threshold_tmp, span_num_sel, bar_flag) == DISP_OK)
		{
			fl_flag ^= 1;
			timer_set(&disp_timer, DISP_CYCLE);
		}

		break;