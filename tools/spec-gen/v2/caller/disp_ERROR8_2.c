// ===== Normal.c : case err_CAL (L3693) =====
		case err_CAL:
			//エラー発生時の処理
			if (disp_ERROR8_2(num, bar_flag) == DISP_OK)
			{
				operation_mode = ERRWAIT2;
			}
			break;