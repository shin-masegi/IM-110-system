// ===== Adjust.c : case ADJ_4 (L1007) =====
	case ADJ_4:		//調整値ホールド画面表示
		if (disp_WAT2((operation_mode & 0xFF00), temp) == DISP_OK)
		{
			timer_set(&disp_hold_timer, HOLD_DISP);
			operation_mode = (operation_mode & 0xFF00) | ADJ_5;
		}
		break;