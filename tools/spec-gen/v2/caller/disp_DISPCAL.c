// ===== Normal.c : case DISPCAL_2 (L2634) =====
	case DISPCAL_2:
		//測定履歴表示
		if (disp_DISPCAL(index, 0, bar_flag) == DISP_OK)
		{
			cmpdisp = 1;
			operation_mode = DISPCAL_3;
		}
		break;

// ===== Normal.c : case DISPCAL_5 (L2714) =====
	case DISPCAL_5:
		//記録削除画面表示
		if (disp_DISPCAL(index, 1, bar_flag) == DISP_OK)
		{
			timer_set(&lcd_off_timer, 20);
			operation_mode = DISPCAL_6;
		}
		//スイッチ動作
		if (MEM_sw_check() >= 3)
		{
			MEM_sw_step = 0;
		}
		break;