uint8_t disp_MLSS_Meas_4(uint8_t batvol)
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

	LS027_disp_icon(2, 5, (uint8_t *)msg_change_normalmode);	//測定モード切替メッセージ

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}