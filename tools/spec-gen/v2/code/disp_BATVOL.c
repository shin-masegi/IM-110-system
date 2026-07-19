uint8_t disp_BATVOL(float fbatvol)
{
	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}
	LS027_disp_icon(0, 1, (uint8_t *)t_bat_vol);		   // タイトル
	LS027_disp_number(36, 10, 0, fbatvol, 2); // 電池電圧
	LS027_disp_icon(43, 18, (uint8_t *)m_Voltage);	  // "mV"

	LS027_disp_icon(0, 18, (uint8_t *)icon_DISP);
	LS027_disp_icon(5, 18, (uint8_t *)icon_next); //次へ

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}