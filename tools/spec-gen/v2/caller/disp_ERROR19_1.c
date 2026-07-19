// ===== Normal.c : case err_ACAL3 (L3544) =====
		case err_ACAL3:
			//エラー発生時の処理
			if (disp_ERROR19_1(num, bar_flag) == DISP_OK)
			{
				operation_mode = ERRWAIT1;
			}
			break;