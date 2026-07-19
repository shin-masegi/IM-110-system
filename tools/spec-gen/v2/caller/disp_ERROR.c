// ===== Normal.c : default (L3594) =====
		default:
			if (disp_ERROR(num, bar_flag) == DISP_OK)
			{
				operation_mode = ERRWAIT1;
			}
			break;
		}
		break;


// ===== Normal.c : default (L3721) =====
		default:
			if (disp_ERROR(num, bar_flag) == DISP_OK)
			{
				operation_mode = ERRWAIT2;
			}
			break;
		}
		if (!flash_timer)
		{
			if (flash_calc(FLASH_BAT, NULL, NULL, NULL, 1, 0, NULL) == DISP_OK)
			{										 // 点滅制御
				timer_set(&flash_timer, DISP_CYCLE); // 表示更新タイマ再設定(500ms)
			}
		}
		break;
