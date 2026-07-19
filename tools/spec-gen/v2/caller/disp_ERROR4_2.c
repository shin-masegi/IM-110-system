// ===== Normal.c : case err_NEND (L3665) =====
		case err_NEND:
			//エラー発生時の処理
			if (disp_ERROR4_2(num, bar_flag) == DISP_OK)
			{
				operation_mode = ERRWAIT2;
			}
			break;