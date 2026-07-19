uint8_t disp_CAL_SETTING(uint8_t select, uint8_t batvol, uint8_t fl)
{
	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}
	LS027_disp_icon(0, 1, (uint8_t *)t_cal_mode); // タイトル
	LS027_disp_battery_icon(batvol);		// 電池残量アイコン

	LS027_disp_datetime(); //日付時刻表示

	LS027_disp_icon(0, 18, (uint8_t *)icon_DISP);
	LS027_disp_icon(0, 23, (uint8_t *)icon_MEM);

	LS027_disp_icon(5, 18, (uint8_t *)icon_select);   //選択
	LS027_disp_icon(5, 23, (uint8_t *)icon_decision); //決定

	LS027_disp_icon(18, 5, ((select == 0) && (fl == 1)) ? (uint8_t *)b_zero_cal : (uint8_t *)w_zero_cal);  // "ゼロ校正"
	LS027_disp_icon(18, 13, ((select == 1) && (fl == 1)) ? (uint8_t *)b_2p_cal : (uint8_t *)w_2p_cal);  // "2点校正"
	LS027_disp_icon(18, 21, ((select == 2) && (fl == 1)) ? (uint8_t *)b_3p_cal : (uint8_t *)w_3p_cal); // "3点校正"

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}