// ===== Normal.c : case gui_ZERO (L3573) =====
		case gui_ZERO:
			//ガイダンス発生時の処理
			if (disp_GUIDE3(bar_flag) == DISP_OK)
			{
				operation_mode = ERRWAIT3;
			}
			break;