// ===== Normal.c : case err_WAR (L3651) =====
		case err_WAR:
			//エラー発生時の処理
			if (disp_ERROR2_2(num, bar_flag) == DISP_OK)
			{
				operation_mode = ERRWAIT2;
			}
			break;