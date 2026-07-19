// ===== Normal.c : case err_NEND (L3494) =====
		case err_NEND:
			//エラー発生時の処理
			if (disp_ERROR4_1(num, bar_flag) == DISP_OK)
			{
				operation_mode = ERRWAIT1;
				timer_set(&timer_errdisp, ERR_DISP);
			}
			break;