uint8_t disp_M_MENU(uint8_t select, uint8_t batvol, uint8_t fl)
{
	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}
	LS027_disp_icon(0, 1, (uint8_t *)t_meas_menu); // タイトル
	LS027_disp_battery_icon(batvol);		// 電池残量アイコン

	LS027_disp_datetime(); //日付時刻表示

	LS027_disp_icon(0, 13, (uint8_t *)icon_DISP);
	LS027_disp_icon(0, 18, (uint8_t *)icon_DISP);
	LS027_disp_icon(0, 23, (uint8_t *)icon_MEM);

	LS027_disp_icon(5, 13, (uint8_t *)icon_l_return); //長押しで前の画面へ
	LS027_disp_icon(5, 18, (uint8_t *)icon_select);   //選択
	LS027_disp_icon(5, 23, (uint8_t *)icon_decision); //決定

	LS027_disp_icon(17, 7, ((select == 0) && (fl == 1)) ? (uint8_t *)b_measmode_menu : (uint8_t *)w_measmode_menu);  // "測定モード"
	//IM-110: 界面測定モード (Meas_Mode == MEAS_MODE_DEPTH) の場合、相関式は使わないので
	//「界面設定」アイコンに差し替える。
	if (Meas_Mode == MEAS_MODE_DEPTH)
	{
		LS027_disp_icon(34, 7, ((select == 1) && (fl == 1)) ? (uint8_t *)b_depth_setting : (uint8_t *)w_depth_setting);  // "界面設定"
	}
	else
	{
		LS027_disp_icon(34, 7, ((select == 1) && (fl == 1)) ? (uint8_t *)b_correlation : (uint8_t *)w_correlation);  // "相関式"
	}
	LS027_disp_icon(17, 17, ((select == 2) && (fl == 1)) ? (uint8_t *)b_calmode_s : (uint8_t *)w_calmode_s); // "校正"
	LS027_disp_icon(34, 17, ((select == 3) && (fl == 1)) ? (uint8_t *)b_setmode_s : (uint8_t *)w_setmode_s); // "設定"

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}