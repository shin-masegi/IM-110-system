uint8_t disp_LOG5(uint8_t batvol)
{
	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}
	if (tansui_sw_flag)
	{
		LS027_disp_icon(0, 1, (uint8_t *)t_meas_s); //海水タイトル
	}
	else
	{
		LS027_disp_icon(0, 1, (uint8_t *)t_measure); //淡水タイトル
	}
	LS027_disp_battery_icon(batvol); // 電池残量アイコン
	LS027_disp_datetime(); //日付時刻表示

	if (!App_DL_disp_flag) {
		LS027_disp_icon(0, 19, (uint8_t *)icon_DISP);	//DISP
		LS027_disp_icon(5, 19, (uint8_t *)icon_hidden);	//以後は非表示
		LS027_disp_icon(0, 24, (uint8_t *)icon_MEM);		//MEM
		LS027_disp_icon(4, 24, (uint8_t *)icon_release);	//解除

		LS027_disp_icon(5, 9, (uint8_t *)msg_reader_dl); 			//Reader dlメッセージ
		qrcode_err_disp(gui_READER_DL);			//QRコード ID-200T Reader DL
	}
	LS027_disp_icon(2, 5, (uint8_t *)msg_change_logermode);	//ロガーモード切替メッセージ

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}