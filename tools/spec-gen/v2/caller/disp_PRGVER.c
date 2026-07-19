// ===== Adjust.c : default (L138) =====
	default:
		if (disp_PRGVER(0) == DISP_OK)
		{
			operation_mode = PRG_2;
		}
		break;


// ===== Adjust.c : case PRG_3 (L177) =====
	case PRG_3:
		if (disp_PRGVER(1) == DISP_OK)
		{
			timer_set(&lcd_off_timer, 20);
			operation_mode = PRG_4;
		}
		break;