// ===== Normal.c : case err_BATT (L3516) =====
		case err_BATT:
			//エラー発生時の処理
			if (disp_ERROR7_1(num, bar_flag) == DISP_OK)
			{
				operation_mode = ERRWAIT1;
			}
			break;