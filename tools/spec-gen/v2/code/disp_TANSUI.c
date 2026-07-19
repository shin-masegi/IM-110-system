uint8_t disp_TANSUI(uint8_t select, uint8_t flg, uint8_t batvol, uint8_t fl)
{
	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}
	LS027_disp_icon(0, 1, (uint8_t *)t_fresh_ocean); // タイトル
	LS027_disp_battery_icon(batvol);		// 電池残量アイコン

	//操作表示仕様変更 2019/03/03 柵木
	LS027_disp_datetime(); //日付時刻表示

	LS027_disp_icon(0, 18, (uint8_t *)icon_DISP);
	LS027_disp_icon(0, 23, (uint8_t *)icon_MEM);

	LS027_disp_icon(5, 18, (uint8_t *)icon_select);   //選択
	LS027_disp_icon(5, 23, (uint8_t *)icon_decision); //決定

	switch(flg)
	{
	case 0:
	default:
		LS027_disp_icon(18, 7, ((select == 0) && (fl == 1)) ? (uint8_t *)b_fresh : (uint8_t *)w_fresh);  // "淡水"
		LS027_disp_icon(18, 19, ((select == 1) && (fl == 1)) ? (uint8_t *)b_ocean : (uint8_t *)w_ocean); // "海水"
		break;
	case 1:
		LS027_disp_icon(15, 6, (uint8_t *)msg_set_fresh);
		break;
	case 2:
		LS027_disp_icon(15, 6, (uint8_t *)msg_set_ocean);
		break;
	}

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}