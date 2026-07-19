// ===== Normal.c : case LOG_9 (L3200) =====
	case LOG_9:
		//省電力測定中画面表示
		if ((lcd_on_flag))
		{
			if (disp_ECOMODE() == DISP_OK)
			{
				//1秒後にLCD電源OFF
				if (!ecomode_timer)
				{
					lcd_off_cmd = 1;
					operation_mode = LOG_6;
				}
			}
			else{
				//何もしない
			}
		}
		break;
