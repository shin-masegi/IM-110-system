uint8_t disp_MEASMODE(uint8_t select, uint8_t flg, uint8_t batvol, uint8_t fl)
{
	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}
	LS027_disp_icon(0, 1, (uint8_t *)t_measmode); // タイトル
	LS027_disp_battery_icon(batvol);		// 電池残量アイコン

	LS027_disp_datetime(); //日付時刻表示

	LS027_disp_icon(0, 18, (uint8_t *)icon_DISP);
	LS027_disp_icon(0, 23, (uint8_t *)icon_MEM);

	LS027_disp_icon(5, 18, (uint8_t *)icon_select);   //選択
	LS027_disp_icon(5, 23, (uint8_t *)icon_decision); //決定

	switch(flg)
	{
	case 0:
	default:
		LS027_disp_icon(18, 7, ((select == 0) && (fl == 1)) ? (uint8_t *)b_normal : (uint8_t *)w_normal);  // "通常"
		LS027_disp_icon(18, 19, ((select == 1) && (fl == 1)) ? (uint8_t *)b_logger : (uint8_t *)w_logger); // "ロガー"
		break;
	case 1:
		LS027_disp_icon(15, 6, (uint8_t *)msg_normal_set);
		break;
	case 2:
		LS027_disp_icon(15, 6, (uint8_t *)msg_logger_set);
		break;
	}

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}