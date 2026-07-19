uint8_t disp_EEP2(uint8_t waitLR, float bar)
{
	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}
	LS027_disp_icon(0, 1, (uint8_t *)t_EEP);									  // タイトル
	LS027_disp_icon(3, 8, (waitLR) ? (uint8_t *)icon_wait1 : (uint8_t *)icon_wait2); // 矢印アイコン
	LS027_disp_bar_icon(bar);

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}