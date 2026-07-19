// ===== Normal.c : case gui_STORE (L3587) =====
		case gui_STORE:
			//ガイダンス発生時の処理
			if (disp_GUIDE5(bar_flag) == DISP_OK)
			{
				operation_mode = ERRWAIT3;
			}
			break;

// ===== mainSub.c : default (L117) =====
	default:
		return ope_mode;
		break;
	}
}


/**
 * @brief 			ワグニット関連のエラー判定とエラー表示切替を行う
 * @detail			センサー寿命、使用不可、保証期限等のエラー判定を行い、
 * 					operation_modeをERRMSGに切り替える
 * @attention
 */
void Ident_Wagnit_Err(void) {
	//自動解除しないエラーメッセージ
#if DEBUG
	SendSerialData((uint8_t *)"Ident_Wagnit_Err\n");	//デバッグ用 2025/11/29 柵木
#endif
	//1WIREテストOK表示（社内検査用）
	if ((wire_test_flag == 1) && (!DAFG_Flag))
	{
		wire_test_flag = 0;
		if ((operation_mode & 0xF000) != ERRMSG) {
			ret_mode = operation_mode;
		}
		oc_err_num = test_WIRE;
		operation_mode = ERRMSG;
#if DEBUG
		SendSerialData((uint8_t *)"test_WIRE\n");	//デバッグ用 2025/11/29 柵木
#endif
	}
	//センサー寿命判断
	else if ((WAG_Flag_SEnd == 10)
			&& (((operation_mode & 0xFF00) == LOGGER)||((operation_mode & 0xFF00) == MEASURE)
			||((operation_mode & 0xF000) == ERRMSG)))
	{
		wire_test_flag = 0;
		if ((operation_mode & 0xF000) != ERRMSG) {
			ret_mode = Check_ret_mode(operation_mode);
		}
		oc_err_num = err_END;
		operation_mode = ERRMSG;
#if DEBUG
		SendSerialData((uint8_t *)"err_END\n");	//デバッグ用 2025/11/29 柵木
#endif
	}
	//センサー使用不可判断
	else if (WAG_Flag_SEnd == 15)
	{
		wire_test_flag = 0;
		if ((operation_mode & 0xF000) != ERRMSG) {
			ret_mode = Check_ret_mode(operation_mode);	//センサー差し替えでOKの時復帰可能に変更 2025/10/28 柵木
		}
		//ret_mode = ERRMSG;
		oc_err_num = err_DISABLE;
		operation_mode = ERRMSG;
#if DEBUG
		SendSerialData((uint8_t *)"err_DISABLE\n");	//デバッグ用 2025/11/29 柵木
#endif
	}
	//自動解除エラーメッセージ
	//保証期限チェック
	else if ((WAG_RestDay > 0) && (WAG_RestDay <= 30)
			&& (((operation_mode & 0xFF00) == LOGGER)||((operation_mode & 0xFF00) == MEASURE)
			||((operation_mode & 0xF000) == ERRMSG)))
	{
		//有効期限まで30日以内
		if ((operation_mode & 0xF000) != ERRMSG) {
			ret_mode = Check_ret_mode(operation_mode);
		}
		oc_err_num = err_DAYS;
		operation_mode = ERRMSG;
#if DEBUG
		SendSerialData((uint8_t *)"err_DAYS\n");	//デバッグ用 2025/11/29 柵木
#endif
	}
	//センサー保証期限切れ判断
	else if ((WAG_Flag_SEnd == 1)
			&& (((operation_mode & 0xFF00) == LOGGER)||((operation_mode & 0xFF00) == MEASURE)
			||((operation_mode & 0xF000) == ERRMSG)))
	{
		if ((operation_mode & 0xF000) != ERRMSG) {
			ret_mode = Check_ret_mode(operation_mode);
		}
		oc_err_num = err_WAR;
		operation_mode = ERRMSG;
#if DEBUG
		SendSerialData((uint8_t *)"err_WAR\n");	//デバッグ用 2025/11/29 柵木
#endif
	}
	//センサー寿命予告判断
	else if (((WAG_Flag_SEnd == 2)||(WAG_Flag_SEnd == 5))
			&& (((operation_mode & 0xFF00) == LOGGER)||((operation_mode & 0xFF00) == MEASURE)
			||((operation_mode & 0xF000) == ERRMSG)))
	{
		if ((operation_mode & 0xF000) != ERRMSG) {
			ret_mode = Check_ret_mode(operation_mode);
		}
		oc_err_num = err_NEND;
#if DEBUG
		SendSerialData((uint8_t *)"err_NEND\n");	//デバッグ用 2025/11/29 柵木
#endif
		operation_mode = ERRMSG;
	}
	//エラー表示中に正常センサー識別
	else if ((operation_mode & 0xF000) == ERRMSG) {
		oc_err_num = err_NONE;
#if DEBUG
		SendSerialData((uint8_t *)"err_NONE\n");	//デバッグ用 2025/11/29 柵木
#endif
		operation_mode = ret_mode;
	}
	else {
		operation_mode = Check_ret_mode(operation_mode);	//エラー無しの戻り先も判定する 2025/12/20 柵木
	}
}


/**
 * @brief 			ワグニットの差し替え（変更）を検出する
// ...(case block truncated)