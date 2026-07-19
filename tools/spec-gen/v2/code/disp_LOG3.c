uint8_t disp_LOG3(uint8_t waitLR, float fdo, uint8_t ddo, float temp, float bar, uint8_t batvol, uint8_t bt_flag, uint8_t log_stop_flag)
{
	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}
	if (tansui_sw_flag)
	{
		LS027_disp_icon(0, 1, (uint8_t *)t_logger_s); //海水タイトル
	}
	else
	{
		LS027_disp_icon(0, 1, (uint8_t *)t_logger); //淡水タイトル
	}
	LS027_disp_battery_icon(batvol); // 電池残量アイコン
	LS027_disp_datetime(); //日付時刻表示

	//アニメアイコントグル
	if (waitLR)
	{
		LS027_disp_icon(19, 18, (uint8_t *)icon_wait1);
	}
	else
	{
		LS027_disp_icon(19, 18, (uint8_t *)icon_wait2);
	}

	LS027_disp_icon(0, 23, (uint8_t *)icon_MEM);
	LS027_disp_icon(5, 23, (uint8_t *)icon_log_stop);  //長押しで停止

	if (log_stop_flag) {
		LS027_disp_icon(0, 18, (uint8_t *)icon_DISP);
		LS027_disp_icon(5, 18, (uint8_t *)icon_return);  //戻る
		LS027_disp_icon(15, 6, (uint8_t *)msg_loger_stop);		//停止確認
	}
	else {
		//開始日時
		LS027_disp_icon(1, 5, (uint8_t *)icon_start_date);		//開始日時
		LS027_disp_number(17, 5, 2, log_start_date[0] / 10, 0);	//年
		LS027_disp_number(19, 5, 2, log_start_date[0] % 10, 0);	//年
		LS027_disp_icon(21, 5, (uint8_t *)s_slash); 				// /
		LS027_disp_number(23, 5, 2, log_start_date[1] / 10, 0);	//月
		LS027_disp_number(25, 5, 2, log_start_date[1] % 10, 0);	//月
		LS027_disp_icon(27, 5, (uint8_t *)s_slash); 				// /
		LS027_disp_number(29, 5, 2, log_start_date[2] / 10, 0);	//日
		LS027_disp_number(31, 5, 2, log_start_date[2] % 10, 0);	//日

		LS027_disp_number(34, 5, 2, log_start_time[0] / 10, 0);	//時
		LS027_disp_number(36, 5, 2, log_start_time[0] % 10, 0);	//時
		LS027_disp_icon(38, 5, (uint8_t *)s_colon); 				//:
		LS027_disp_number(40, 5, 2, log_start_time[1] / 10, 0);	//分
		LS027_disp_number(42, 5, 2, log_start_time[1] % 10, 0);	//分

		//測定間隔
		LS027_disp_icon(1, 9, (uint8_t *)icon_sample_gap);		//測定間隔
		LS027_disp_number(42, 9, 2, log_sampling_gap, 0);
		LS027_disp_icon(45, 9, (uint8_t *)s_minute);				//分

		//サンプル数
		LS027_disp_icon(1, 13, (uint8_t *)icon_sample_num);		//サンプル数
		LS027_disp_number(32, 13, 2, log_sampling_num, 0);		// No.
		LS027_disp_icon(34, 13, (uint8_t *)s_slash); 				// /
		LS027_disp_number(42, 13, 2, 2000, 0); 					//2000
		LS027_disp_icon(45, 13, (uint8_t *)s_num2); 				//点
	}

	LS027_disp_number(41, 17, 1, fdo, ddo); 					//DO
	LS027_disp_icon(45, 20, (uint8_t *)m_mgl); 				// "mg/L"
	LS027_disp_number(41, 23, 1, temp, 1); 					//水温
	LS027_disp_icon(45, 26, (uint8_t *)m_temp); 				// "℃"

	if (bt_flag){
		LS027_disp_icon(46, 5, (uint8_t *)BT); 				//BT
	}

	SPI_transmitLCD(); // LCD表示
	return DISP_OK;
}