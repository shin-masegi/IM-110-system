uint8_t disp_SETYEAR(uint8_t select, uint8_t batvol, uint8_t fl, uint8_t en)
{

	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}
	LS027_disp_icon(0, 1, (uint8_t *)t_set_year); // タイトル

	LS027_disp_icon(0, 18, (uint8_t *)icon_DISP);
	LS027_disp_icon(0, 23, (uint8_t *)icon_MEM);
	if (en)
	{
		//編集状態表示
		LS027_disp_icon(5, 18, (uint8_t *)icon_select);   //選択
		LS027_disp_icon(5, 23, (uint8_t *)icon_decision); //決定
	}
	else
	{
		//編集前表示
		LS027_disp_icon(5, 18, (uint8_t *)icon_next);		//次へ
		LS027_disp_icon(5, 23, (uint8_t *)icon_start);	//開始
	}

	LS027_disp_icon(44, 10, ((select == 0) && (fl == 1)) ? (uint8_t *)b_plus : (uint8_t *)w_plus);	 // "+"
	LS027_disp_icon(44, 16, ((select == 1) && (fl == 1)) ? (uint8_t *)b_minus : (uint8_t *)w_minus);   // "-"
	LS027_disp_icon(28, 21, ((select == 2) && (fl == 1)) ? (uint8_t *)b_cancel : (uint8_t *)w_cancel); // "中止"
	LS027_disp_icon(39, 21, ((select == 3) && (fl == 1)) ? (uint8_t *)b_set : (uint8_t *)w_set);	   // "セット"
	LS027_disp_number(37, 9, 0, year, 0);			//年

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}