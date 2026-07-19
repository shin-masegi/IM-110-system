// ===== Normal.c : case gui_INDI (L3559) =====
		case gui_INDI:
			//ガイダンス発生時の処理
			if (disp_GUIDE1(bar_flag) == DISP_OK)
			{
				operation_mode = ERRWAIT3;
			}
			break;