// ===== Normal.c : case DISPHIS_2 (L2418) =====
	case DISPHIS_2:
		//測定履歴表示
		if (disp_DISPHIS(index, 0, bar_flag) == DISP_OK)
		{
			cmpdisp = 1;
			operation_mode = DISPHIS_3;
		}
		break;

// ===== Normal.c : case DISPHIS_5 (L2488) =====
	case DISPHIS_5:
		//記録削除画面表示
		if (disp_DISPHIS(index, 1, bar_flag) == DISP_OK)
		{
			timer_set(&lcd_off_timer, 20);
			operation_mode = DISPHIS_6;
		}
		//スイッチ動作
		if (MEM_sw_check() >= 3)
		{
			MEM_sw_step = 0;
		}
		break;