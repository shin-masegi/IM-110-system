// ===== Normal.c : case err_OUT (L3658) =====
		case err_OUT:
			//エラー発生時の処理
			if (disp_ERROR3_2(num, bar_flag) == DISP_OK)
			{
				operation_mode = ERRWAIT2;
			}
			break;