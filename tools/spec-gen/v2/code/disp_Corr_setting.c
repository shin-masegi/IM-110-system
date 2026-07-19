uint8_t disp_Corr_setting(uint8_t select, uint8_t batvol, uint8_t fl)
{
	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}
	LS027_disp_icon(0, 1, (uint8_t *)t_correlation); // タイトル
	LS027_disp_battery_icon(batvol);		// 電池残量アイコン

	LS027_disp_datetime(); //日付時刻表示

	LS027_disp_icon(0, 18, (uint8_t *)icon_DISP);
	LS027_disp_icon(0, 23, (uint8_t *)icon_MEM);

	LS027_disp_icon(5, 18, (uint8_t *)icon_select);   //選択
	LS027_disp_icon(5, 23, (uint8_t *)icon_decision); //決定

	LS027_disp_icon(18, 6, ((select == 0) && (fl == 1)) ? (uint8_t *)b_m_1 : (uint8_t *)m_num[1]);  // "1"
	LS027_disp_icon(24, 6, ((select == 1) && (fl == 1)) ? (uint8_t *)b_m_2 : (uint8_t *)m_num[2]);  // "2"
	LS027_disp_icon(30, 6, ((select == 2) && (fl == 1)) ? (uint8_t *)b_m_3 : (uint8_t *)m_num[3]); 	// "3"
	LS027_disp_icon(36, 6, ((select == 3) && (fl == 1)) ? (uint8_t *)b_m_4 : (uint8_t *)m_num[4]);  // "4"
	LS027_disp_icon(42, 6, ((select == 4) && (fl == 1)) ? (uint8_t *)b_m_5 : (uint8_t *)m_num[5]); 	// "5"

	LS027_disp_icon(18, 14, ((select == 5) && (fl == 1)) ? (uint8_t *)b_m_6 : (uint8_t *)m_num[6]);  // "6"
	LS027_disp_icon(24, 14, ((select == 6) && (fl == 1)) ? (uint8_t *)b_m_7 : (uint8_t *)m_num[7]);  // "7"
	LS027_disp_icon(30, 14, ((select == 7) && (fl == 1)) ? (uint8_t *)b_m_8 : (uint8_t *)m_num[8]);  // "8"
	LS027_disp_icon(36, 14, ((select == 8) && (fl == 1)) ? (uint8_t *)b_m_9 : (uint8_t *)m_num[9]);  // "9"

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}