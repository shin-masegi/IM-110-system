// ===== Normal.c : case err_METER (L3679) =====
		case err_METER:
			//エラー発生時の処理
			if (disp_ERROR6_2(num, bar_flag) == DISP_OK)
			{
				operation_mode = ERRWAIT2;
			}
			break;