uint8_t disp_AUTOCAL_COMP(uint8_t batvol)
{
	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}
	LS027_disp_battery_icon(batvol);		//電池残量アイコン

	LS027_disp_icon(1, 8, (uint8_t *)msg_autocal_comp);	//メッセージ
	disp_cal_datetime(10, 14);					//校正日時表示yyyy/mm/dd hh:mm

	LS027_disp_icon(0, 24, (uint8_t *)icon_MEM);		//MEM
	LS027_disp_icon(5, 24, (uint8_t *)icon_next);		//次へ
	LS027_disp_icon(16, 20, (uint8_t *)err_icon1);	//エラーアイコン

	SPI_transmitLCD();				//LCD表示

	return DISP_OK;
}