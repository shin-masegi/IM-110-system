// ===== Adjust.c : case EEP_9 (L530) =====
	case EEP_9:		//結果表示
		if (disp_EEP3(result) == DISP_OK)
		{
			init_param_set();
			Default_InfoData_Set();
			eep_write_adjust();
			eep_write_1wireinfo();
			eep_write_1wirebk();
			operation_mode = EEP_A;
		}
		break;
