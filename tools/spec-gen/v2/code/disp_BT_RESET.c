uint8_t disp_BT_RESET(uint8_t msg_no, uint8_t batvol)
{
	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}
	LS027_disp_icon(0, 1, (uint8_t *)t_bt_reset); // タイトル
	LS027_disp_battery_icon(batvol);  // 電池残量アイコン
	LS027_disp_datetime(); //日付時刻表示

	switch (msg_no)
	{
	case 0:
		//リセット前のメッセージ表示
		LS027_disp_icon(0, 18, (uint8_t *)icon_DISP);
		LS027_disp_icon(0, 23, (uint8_t *)icon_MEM);
		LS027_disp_icon(5, 18, (uint8_t *)icon_return);   //戻る
		LS027_disp_icon(5, 23, (uint8_t *)icon_decision); //決定
		LS027_disp_icon(18, 6, (uint8_t *)msg_bt_reset);
		break;
	case 1:
		//リセット後のメッセージ表示
		//LS027_disp_icon(0, 23, (uint8_t *)icon_MEM);
		//LS027_disp_icon(5, 23, (uint8_t *)icon_stop); //中止
		LS027_disp_icon(18, 6, (uint8_t *)msg_bt_reset2);
		break;
	case 2:
		//リセット後のメッセージ表示
		LS027_disp_icon(0, 18, (uint8_t *)icon_DISP);
		LS027_disp_icon(5, 18, (uint8_t *)icon_return); //戻る
		LS027_disp_icon(18, 6, (uint8_t *)msg_reset_comp);
		break;
	}

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}