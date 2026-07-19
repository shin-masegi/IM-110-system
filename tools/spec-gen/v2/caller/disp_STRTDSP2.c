// ===== Normal.c : case STRT_5 (L2555) =====
		case STRT_5:
			if (disp_STRTDSP2() == DISP_OK)
			{
				timer_set(&lcd_off_timer, 60);
				operation_mode = STRT_2;
			}
			break;
