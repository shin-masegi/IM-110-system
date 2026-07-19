// ===== Normal.c : case gui_ACALCOMP (L3551) =====
		case gui_ACALCOMP:
			//エラー発生時の処理
			if (disp_AUTOCAL_COMP(bar_flag) == DISP_OK)
			{
				operation_mode = ERRWAIT1;
				timer_set(&timer_errdisp, ERR_DISP);
			}
			break;