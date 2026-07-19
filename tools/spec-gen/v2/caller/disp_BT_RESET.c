// ===== Setting.c : case BT_RESET_1 (L837) =====
		case BT_RESET_1:		//BTリセット前画面表示
			if (disp_BT_RESET(0, bar_flag) == DISP_OK)
			{
				timer_set(&flash_timer, DISP_CYCLE);
				operation_mode = BT_RESET_2;
			}
			break;

// ===== Setting.c : case BT_RESET_3 (L856) =====
		case BT_RESET_3:			//BT初期化中画面表示
			if (disp_BT_RESET(1, bar_flag) == DISP_OK)
			{
				timer_set(&flash_timer, DISP_CYCLE);
				ble_step = BLE00_00;
				operation_mode = BT_RESET_4;
			}
			break;

// ===== Setting.c : case BT_RESET_5 (L872) =====
		case BT_RESET_5:		//BTリセット後画面表示
			if (disp_BT_RESET(2, bar_flag) == DISP_OK)
			{
				timer_set(&flash_timer, DISP_CYCLE);
				timer_set(&lcd_off_timer, 60);
				operation_mode = BT_RESET_6;
			}
			break;