// ===== Normal.c : case DISPLOG_2 (L3386) =====
	case DISPLOG_2: // データロガー表示-----------------------------------------------------------------
		if (disp_DISPLOG(log_sort_index[0], log_sort_index[1], log_sort_index[2], bar_flag) == DISP_OK)
		{ // 表示更新
			timer_set(&disp_timer, DISP_CYCLE);
			operation_mode = DISPLOG_3; // [データロガー表示中]に遷移
		}
		break;