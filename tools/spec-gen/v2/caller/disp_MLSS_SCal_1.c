// ===== Normal.c : case C_S_SET_2 (L266) =====
	case C_S_SET_2:		//スパン校正値設定画面表示
		operation_mode = C_S_SET_3;
		flash_calc(FLASH_BAT, NULL, NULL, NULL, 0, 1, NULL);
		if (disp_MLSS_SCal_1(fl_flag, Cal_SetVal_1_tmp, span_num_sel, bar_flag) == DISP_OK)
		{
			fl_flag ^= 1;
			timer_set(&disp_timer, DISP_CYCLE);
		}

		break;

// ===== Normal.c : case C_M_SET_2 (L412) =====
	case C_M_SET_2:		//中間濃度設定画面表示
		operation_mode = C_M_SET_3;
		flash_calc(FLASH_BAT, NULL, NULL, NULL, 0, 1, NULL);
		if (disp_MLSS_SCal_1(fl_flag, Cal_SetVal_2_tmp, span_num_sel, bar_flag) == DISP_OK)
		{
			fl_flag ^= 1;
			timer_set(&disp_timer, DISP_CYCLE);
		}
		break;