uint8_t disp_STRTDSP(void)
{
	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}

	//IM-110
	LS027_disp_icon(4, 7, (uint8_t *)msg_IM110);
	//FW_VER1
	LS027_disp_number(43, 16, 2, (float)FW_VER1 / 100, 2);
	//Ver.
	LS027_disp_icon(31, 16, (uint8_t *)msg_Ver);
	//飯島電子工業株式会社
	LS027_disp_icon(21, 24, (uint8_t *)msg_iijima);

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}