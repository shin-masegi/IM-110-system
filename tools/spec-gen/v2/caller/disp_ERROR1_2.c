// ===== Normal.c : case err_DAYS (L3644) =====
		case err_DAYS:
			//エラー発生時の処理
			if (disp_ERROR1_2(num, bar_flag) == DISP_OK)
			{
				operation_mode = ERRWAIT2;
			}
			break;