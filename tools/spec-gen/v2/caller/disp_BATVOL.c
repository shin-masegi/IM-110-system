// ===== Adjust.c : default (L814) =====
	default:
		if (disp_BATVOL(Y2) == DISP_OK)
		{
			operation_mode = BAT_2;
			timer_set(&disp_timer, DISP_CYCLE);
		}
		break;
