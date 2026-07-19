uint8_t disp_S_MENU(uint8_t select, uint8_t batvol, uint8_t fl)
{
	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}
	LS027_disp_icon(0, 1, (uint8_t *)t_setting_menu); // タイトル
	LS027_disp_battery_icon(batvol);		// 電池残量アイコン

	LS027_disp_datetime(); //日付時刻表示

	LS027_disp_icon(0, 13, (uint8_t *)icon_DISP);
	LS027_disp_icon(0, 18, (uint8_t *)icon_DISP);
	LS027_disp_icon(0, 23, (uint8_t *)icon_MEM);

	LS027_disp_icon(5, 13, (uint8_t *)icon_l_return); //長押しで前の画面へ
	LS027_disp_icon(5, 18, (uint8_t *)icon_select);   //選択
	LS027_disp_icon(5, 23, (uint8_t *)icon_decision); //決定

	LS027_disp_icon(17, 7, ((select == 0) && (fl == 1)) ? (uint8_t *)b_settime_s : (uint8_t *)w_settime_s);  // "時刻設定"
	LS027_disp_icon(34, 7, ((select == 1) && (fl == 1)) ? (uint8_t *)b_reset_s : (uint8_t *)w_reset_s);  // "初期化"
	LS027_disp_icon(17, 17, ((select == 2) && (fl == 1)) ? (uint8_t *)b_info_s : (uint8_t *)w_info_s); // "製品情報"
	LS027_disp_icon(34, 17, ((select == 3) && (fl == 1)) ? (uint8_t *)b_depth_setting : (uint8_t *)w_depth_setting); // "界面しきい値設定" (旧 [測定]、長押し戻りと機能重複していたため置換)

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}