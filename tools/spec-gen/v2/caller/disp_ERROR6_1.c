// ===== Normal.c : case err_METER (L3509) =====
		case err_METER:
			//エラー発生時の処理
			if (disp_ERROR6_1(num, bar_flag) == DISP_OK)
			{
				operation_mode = ERRWAIT1;
			}
			break;