// ===== Normal.c : case gui_SPAN (L3566) =====
		case gui_SPAN:
			//ガイダンス発生時の処理
			if (disp_GUIDE2(bar_flag) == DISP_OK)
			{
				operation_mode = ERRWAIT3;
			}
			break;