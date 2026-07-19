uint8_t disp_QR(uint8_t qr_flg, uint8_t batvol)
{
	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}
	LS027_disp_icon(0, 1, (uint8_t *)t_QR);					// タイトル
	LS027_disp_battery_icon(batvol);  // 電池残量アイコン
	LS027_disp_datetime();						//日付時刻表示

	if (!qr_flg)
	{
		//情報表示
		LS027_disp_icon(0, 18, (uint8_t *)icon_DISP);
		LS027_disp_icon(5, 18, (uint8_t *)icon_return);		//戻る

		LS027_disp_icon(18, 24, (uint8_t *)icon_info_err);	//最新エラー番号
		LS027_disp_number(46, 24, 2, ErrNo, 0);
		LS027_disp_icon(18, 27, (uint8_t *)icon_line);
	}
	else
	{
		//QR表示
		LS027_disp_icon(0, 18, (uint8_t *)icon_DISP);
		LS027_disp_icon(5, 18, (uint8_t *)icon_return);		//戻る
		LS027_disp_icon(0, 23, (uint8_t *)icon_MEM);
		LS027_disp_icon(5, 23, (uint8_t *)icon_info_disp);	//情報表示

		qrcode_info_disp();					//QRコード
	}

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}