// ===== Normal.c : case MCAL_4 (L1459) =====
	case MCAL_4:
		//スパン校正中画面表示
		if (disp_timer == 0)
		{
			flash_calc(FLASH_ALL, NULL, NULL, NULL, 0, 0, NULL);
			if (disp_MLSS_MCal_2(fl_flag, get_stable_prog(), bar_flag) == DISP_OK)
			{
				timer_set(&disp_timer, DISP_CYCLE);
				fl_flag ^= 1;
			}
		}
		operation_mode = MCAL_3;
		//スイッチ動作
		if ((MEM_sw_check() >= 3))
		{
			MEM_sw_step = 0;
			//OKスイッチでスパン校正中止
			fl_flag = 0;
			operation_mode = MCAL_1;
		}
		else if (!cal_timer)
		{
			//校正タイムアウト(60sec経過した)
			Set_CalData(2, calexec_time);
			//センサー不安定
			oc_err_num = err_OUT;
			ret_mode = MCAL;
			operation_mode = ERRMSG;
		}
		break;
