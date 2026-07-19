uint8_t disp_ECOMODE(void)
{

	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}

	//メッセージ表示のみ
	LS027_disp_icon(14, 6, (uint8_t *)msg_loging);

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}