uint8_t disp_DISPLOG(uint8_t index1, uint8_t index2, uint8_t index3, uint8_t batvol)
{
	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}
	LS027_disp_icon(0, 1, (uint8_t *)t_logger_disp); // タイトル
	LS027_disp_battery_icon(batvol);		// 電池残量アイコン
	LS027_disp_datetime(); //日付時刻表示

	LS027_disp_icon(0, 18, (uint8_t *)icon_DISP);
	LS027_disp_icon(5, 18, (uint8_t *)icon_change); //切替
	//LS027_disp_icon(0, 23, (uint8_t *)icon_MEM);
	//LS027_disp_icon(5, 23, (uint8_t *)icon_next);   //次へ

	//No.
	LS027_disp_number(17, 6, 2, 0, 0);	//0
	LS027_disp_number(19, 6, 2, 1, 0);	//1
	//開始日時
	LS027_disp_number(22, 6, 2, log_head[index1].year / 10, 0);	//年
	LS027_disp_number(24, 6, 2, log_head[index1].year % 10, 0);	//年
	LS027_disp_icon(26, 6, (uint8_t *)s_slash); 			// /
	LS027_disp_number(28, 6, 2, log_head[index1].month / 10, 0);	//月
	LS027_disp_number(30, 6, 2, log_head[index1].month % 10, 0);	//月
	LS027_disp_icon(32, 6, (uint8_t *)s_slash); 			// /
	LS027_disp_number(34, 6, 2, log_head[index1].day / 10, 0);	//日
	LS027_disp_number(36, 6, 2, log_head[index1].day % 10, 0);	//日
	LS027_disp_number(39, 6, 2, log_head[index1].hour / 10, 0);	//時
	LS027_disp_number(41, 6, 2, log_head[index1].hour % 10, 0);	//時
	LS027_disp_icon(43, 6, (uint8_t *)s_colon);			//:
	LS027_disp_number(45, 6, 2, log_head[index1].min / 10, 0);	//分
	LS027_disp_number(47, 6, 2, log_head[index1].min % 10, 0);	//分
	//測定間隔
	LS027_disp_number(31, 10, 2, log_head[index1].sampling, 0);
	LS027_disp_icon(34, 10, (uint8_t *)s_minute);			//分
	//サンプル数
	if (log_head[index1].sampling > 0)
	{
		LS027_disp_number(43, 10, 2, log_head[index1].num, 0);	// No.
	}
	else
	{
		LS027_disp_number(43, 10, 2, 0, 0);						// 0
	}
	LS027_disp_icon(46, 10, (uint8_t *)s_num2); 			//点

	//No.
	LS027_disp_number(17, 14, 2, 0, 0);	//0
	LS027_disp_number(19, 14, 2, 2, 0);	//2
	//開始日時
	LS027_disp_number(22, 14, 2, log_head[index2].year / 10, 0);	//年
	LS027_disp_number(24, 14, 2, log_head[index2].year % 10, 0);	//年
	LS027_disp_icon(26, 14, (uint8_t *)s_slash); 			// /
	LS027_disp_number(28, 14, 2, log_head[index2].month / 10, 0);	//月
	LS027_disp_number(30, 14, 2, log_head[index2].month % 10, 0);	//月
	LS027_disp_icon(32, 14, (uint8_t *)s_slash); 			// /
	LS027_disp_number(34, 14, 2, log_head[index2].day / 10, 0);	//日
	LS027_disp_number(36, 14, 2, log_head[index2].day % 10, 0);	//日
	LS027_disp_number(39, 14, 2, log_head[index2].hour / 10, 0);	//時
	LS027_disp_number(41, 14, 2, log_head[index2].hour % 10, 0);	//時
	LS027_disp_icon(43, 14, (uint8_t *)s_colon);			//:
	LS027_disp_number(45, 14, 2, log_head[index2].min / 10, 0);	//分
	LS027_disp_number(47, 14, 2, log_head[index2].min % 10, 0);	//分
	//測定間隔
	LS027_disp_number(31, 18, 2, log_head[index2].sampling, 0);
	LS027_disp_icon(34, 18, (uint8_t *)s_minute);			//分
	//サンプル数
	if (log_head[index2].sampling > 0)
	{
		LS027_disp_number(43, 18, 2, log_head[index2].num , 0);	// No.
	}
	else
	{
		LS027_disp_number(43, 18, 2, 0, 0);						// 0
	}
	LS027_disp_icon(46, 18, (uint8_t *)s_num2); 			//点

	//No.
	LS027_disp_number(17, 22, 2, 0, 0);	//0
	LS027_disp_number(19, 22, 2, 3, 0);	//3
	//開始日時
	LS027_disp_number(22, 22, 2, log_head[index3].year / 10, 0);	//年
	LS027_disp_number(24, 22, 2, log_head[index3].year % 10, 0);	//年
	LS027_disp_icon(26, 22, (uint8_t *)s_slash); 			// /
	LS027_disp_number(28, 22, 2, log_head[index3].month / 10, 0);	//月
	LS027_disp_number(30, 22, 2, log_head[index3].month % 10, 0);	//月
	LS027_disp_icon(32, 22, (uint8_t *)s_slash); 			// /
	LS027_disp_number(34, 22, 2, log_head[index3].day / 10, 0);	//日
	LS027_disp_number(36, 22, 2, log_head[index3].day % 10, 0);	//日
	LS027_disp_number(39, 22, 2, log_head[index3].hour / 10, 0);	//時
	LS027_disp_number(41, 22, 2, log_head[index3].hour % 10, 0);	//時
	LS027_disp_icon(43, 22, (uint8_t *)s_colon);			//:
	LS027_disp_number(45, 22, 2, log_head[index3].min / 10, 0);	//分
	LS027_disp_number(47, 22, 2, log_head[index3].min % 10, 0);	//分
	//測定間隔
	LS027_disp_number(31, 26, 2, log_head[index3].sampling, 0);
	LS027_disp_icon(34, 26, (uint8_t *)s_minute);			//分
	//サンプル数
	if (log_head[index3].sampling > 0)
	{
		LS027_disp_number(43, 26, 2, log_head[index3].num, 0);	// No.
	}
	else
	{
		LS027_disp_number(43, 26, 2, 0, 0);						// 0
	}
	LS027_disp_icon(46, 26, (uint8_t *)s_num2); 			//点

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}