uint8_t disp_EEP3(uint8_t ok)
{
	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}
	LS027_disp_icon(0, 1, (uint8_t *)t_EEP);							   // タイトル
	LS027_disp_icon(25, 11, (ok == OK) ? (uint8_t *)icon_OK : (uint8_t *)icon_NG); // "OK" or "NG"
	LS027_disp_bar_icon(100.0);

	LS027_disp_icon(0, 18, (uint8_t *)icon_DISP);
	LS027_disp_icon(0, 23, (uint8_t *)icon_MEM);

	LS027_disp_icon(5, 18, (uint8_t *)icon_next);  //次へ
	LS027_disp_icon(5, 23, (uint8_t *)icon_start); //開始

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}