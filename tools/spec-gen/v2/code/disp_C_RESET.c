uint8_t disp_C_RESET(uint8_t msg_flg, uint8_t corr_num, uint8_t batvol)
{
	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}
	LS027_disp_icon(0, 1, (uint8_t *)t_reset); // タイトル
	LS027_disp_battery_icon(batvol);  // 電池残量アイコン
	LS027_disp_datetime(); //日付時刻表示

	if (!msg_flg)
	{
		//リセット前のメッセージ表示
		LS027_disp_icon(0, 18, (uint8_t *)icon_DISP);
		LS027_disp_icon(0, 23, (uint8_t *)icon_MEM);

		LS027_disp_icon(5, 18, (uint8_t *)icon_return);   //戻る
		LS027_disp_icon(5, 23, (uint8_t *)icon_decision); //決定

		LS027_disp_icon(18, 6, (uint8_t *)msg_corr_No);	// "相関式No."
		LS027_disp_number(36, 6, 2, corr_num, 0);			// 相関式番号
		LS027_disp_icon(18, 10, (uint8_t *)msg_corr_reset);
	}
	else
	{
		//リセット後のメッセージ表示
		LS027_disp_icon(0, 18, (uint8_t *)icon_DISP);
		LS027_disp_icon(5, 18, (uint8_t *)icon_return); //戻る

		LS027_disp_icon(18, 6, (uint8_t *)msg_corr_No);	// "相関式No."
		LS027_disp_number(36, 6, 2, corr_num, 0);			// 相関式番号
		LS027_disp_icon(18, 10, (uint8_t *)msg_corr_reset_complete);	
	}

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}