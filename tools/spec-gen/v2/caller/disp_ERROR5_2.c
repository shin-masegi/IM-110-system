// ===== Normal.c : case err_END (L3672) =====
		case err_END:
			//エラー発生時の処理
			if (disp_ERROR5_2(num, bar_flag) == DISP_OK)
			{
				operation_mode = ERRWAIT2;
			}
			break;