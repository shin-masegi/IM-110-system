uint8_t disp_ERROR(err_num num, uint8_t batvol)
{
	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}

	LS027_disp_number(44, 5, 0, (float)num, 0); // 番号のみ表示
	LS027_disp_battery_icon(batvol); // 電池残量アイコン

	LS027_disp_icon(0, 23, (uint8_t *)icon_MEM);
	LS027_disp_icon(5, 23, (uint8_t *)icon_release); //解除

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}