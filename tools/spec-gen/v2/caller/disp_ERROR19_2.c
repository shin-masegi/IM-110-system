// ===== Normal.c : case err_ACAL3 (L3714) =====
		case err_ACAL3:
			//エラー発生時の処理
			if (disp_ERROR19_2(num, bar_flag) == DISP_OK)
			{
				operation_mode = ERRWAIT2;
			}
			break;