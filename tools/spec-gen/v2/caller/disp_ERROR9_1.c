// ===== Normal.c : case err_DISABLE (L3530) =====
		case err_DISABLE:
			//エラー発生時の処理
			if (disp_ERROR9_1(num, bar_flag) == DISP_OK)
			{
				operation_mode = ERRWAIT1;
			}
			break;