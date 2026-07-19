// ===== Normal.c : case err_WAR (L3479) =====
		case err_WAR:
			//エラー発生時の処理
			if (disp_ERROR2_1(num, bar_flag) == DISP_OK)
			{
				operation_mode = ERRWAIT1;
				timer_set(&timer_errdisp, ERR_DISP);
			}
			break;