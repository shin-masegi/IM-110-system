uint8_t disp_APPDL(uint8_t batvol)
{
	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}
	LS027_disp_icon(0, 1, (uint8_t *)t_app_dl); //タイトル

	LS027_disp_battery_icon(batvol); // 電池残量アイコン
	LS027_disp_datetime(); //日付時刻表示

	LS027_disp_icon(0, 19, (uint8_t *)icon_DISP);	//DISP
	LS027_disp_icon(5, 19, (uint8_t *)icon_return);	//戻る

	LS027_disp_icon(2, 6, (uint8_t *)msg_reader_dl); 			//Reader dlメッセージ
	qrcode_err_disp(gui_READER_DL);			//QRコード ID-200T Reader DL

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}