// ===== Normal.c : case err_DISABLE (L3700) =====
		case err_DISABLE:
			//エラー発生時の処理
			if (disp_ERROR9_2(num, bar_flag) == DISP_OK)
			{
				operation_mode = ERRWAIT2;
			}
			break;