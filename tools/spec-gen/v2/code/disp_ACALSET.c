uint8_t disp_ACALSET(uint8_t select, uint8_t flg, uint8_t batvol, uint8_t fl)
{
	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}
	LS027_disp_icon(0, 1, (uint8_t *)t_acal_setting); // タイトル
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
		LS027_disp_icon(18, 7, ((select == 0) && (fl == 1)) ? (uint8_t *)b_acal_off : (uint8_t *)w_acal_off);  // "OFF"
		LS027_disp_icon(34, 7, ((select == 1) && (fl == 1)) ? (uint8_t *)b_acal_4am : (uint8_t *)w_acal_4am);  // "4時"
		LS027_disp_icon(18, 18, ((select == 2) && (fl == 1)) ? (uint8_t *)b_acal_5am : (uint8_t *)w_acal_5am); // "5時"
		LS027_disp_icon(34, 18, ((select == 3) && (fl == 1)) ? (uint8_t *)b_acal_6am : (uint8_t *)w_acal_6am); // "6時"
		break;
	case 1:
		LS027_disp_icon(15, 6, (uint8_t *)msg_acal_off_set);
		break;
	case 2:
		LS027_disp_icon(15, 6, (uint8_t *)msg_acal_4am_set);
		break;
	case 3:
		LS027_disp_icon(15, 6, (uint8_t *)msg_acal_5am_set);
		break;
	case 4:
		LS027_disp_icon(15, 6, (uint8_t *)msg_acal_6am_set);
		break;
	}

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}