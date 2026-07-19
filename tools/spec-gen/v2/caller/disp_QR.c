// ===== Setting.c : case QR_1 (L134) =====
		case QR_1:		//情報表示
			if (disp_QR(0, bar_flag) == DISP_OK)
			{
				WAG_flash_flag ^= 1;
				timer_set(&flash_timer, DISP_CYCLE);
				operation_mode = QR_2;
			}
			break;

// ===== Setting.c : case QR_3 (L161) =====
		case QR_3:		//QR表示（未使用）
			if (disp_QR(1, bar_flag) == DISP_OK)
			{
				operation_mode = QR_4;
			}
			break;