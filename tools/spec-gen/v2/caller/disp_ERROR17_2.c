// ===== Normal.c : case err_ACAL1 (L3707) =====
		case err_ACAL1:
			//エラー発生時の処理
			if (disp_ERROR17_2(num, bar_flag) == DISP_OK)
			{
				operation_mode = ERRWAIT2;
			}
			break;