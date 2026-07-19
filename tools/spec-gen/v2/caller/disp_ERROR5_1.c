// ===== Normal.c : case err_END (L3502) =====
		case err_END:
			//エラー発生時の処理
			if (disp_ERROR5_1(num, bar_flag) == DISP_OK)
			{
				operation_mode = ERRWAIT1;
			}
			break;