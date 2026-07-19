// ===== Normal.c : case err_DAYS (L3471) =====
		case err_DAYS:
			//エラー発生時の処理
			if (disp_ERROR1_1(num, bar_flag) == DISP_OK)
			{
				operation_mode = ERRWAIT1;
				timer_set(&timer_errdisp, ERR_DISP);
			}
			break;