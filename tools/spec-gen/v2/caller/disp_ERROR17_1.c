// ===== Normal.c : case err_ACAL1 (L3537) =====
		case err_ACAL1:
			//エラー発生時の処理
			if (disp_ERROR17_1(num, bar_flag) == DISP_OK)
			{
				operation_mode = ERRWAIT1;
			}
			break;