// ===== Adjust.c : case ADJ_4 (L986) =====
	case ADJ_4:		//調整値ホールド画面表示
		if (disp_RANGE2((operation_mode & 0xFF00), domv_thr) == DISP_OK)
		{
			timer_set(&disp_hold_timer, HOLD_DISP);
			operation_mode = (operation_mode & 0xFF00) | ADJ_5;
		}
		break;