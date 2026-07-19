// ===== Normal.c : case err_BATT (L3686) =====
		case err_BATT:
			//エラー発生時の処理
			if (disp_ERROR7_2(num, bar_flag) == DISP_OK)
			{
				operation_mode = ERRWAIT2;
			}
			break;