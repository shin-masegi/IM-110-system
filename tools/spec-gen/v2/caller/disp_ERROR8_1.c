// ===== Normal.c : case err_CAL (L3523) =====
		case err_CAL:
			//エラー発生時の処理
			if (disp_ERROR8_1(num, bar_flag) == DISP_OK)
			{
				operation_mode = ERRWAIT1;
			}
			break;