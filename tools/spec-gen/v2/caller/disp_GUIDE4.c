// ===== Normal.c : case gui_MEAS (L3580) =====
		case gui_MEAS:
			//ガイダンス発生時の処理
			if (disp_GUIDE4(bar_flag) == DISP_OK)
			{
				operation_mode = ERRWAIT3;
			}
			break;