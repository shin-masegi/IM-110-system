// ===== Normal.c : case err_OUT (L3487) =====
		case err_OUT:
			//エラー発生時の処理
			if (disp_ERROR3_1(num, bar_flag) == DISP_OK)
			{
				operation_mode = ERRWAIT1;
			}
			break;