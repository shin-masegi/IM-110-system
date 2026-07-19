// ===== Adjust.c : default (L889) =====
	default:
		adj_calc(operation_mode);	//計算値を更新
		if (disp_RANGE1((operation_mode & 0xFF00), AD) == DISP_OK)
		{
			timer_set(&disp_timer, DISP_CYCLE);
			operation_mode = (operation_mode & 0xFF00) | ADJ_2;
		}
		break;