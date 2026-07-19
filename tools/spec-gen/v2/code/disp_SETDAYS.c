uint8_t disp_SETDAYS(uint8_t select, uint8_t batvol, uint8_t fl, uint8_t en)
{

	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}
	LS027_disp_icon(0, 1, (uint8_t *)t_set_days); // タイトル

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

	LS027_disp_icon(16, 2, ((select == 0) && (fl == 1)) ? (uint8_t *)b_plus : (uint8_t *)w_plus);	 // "+"
	LS027_disp_icon(16, 18, ((select == 1) && (fl == 1)) ? (uint8_t *)b_minus : (uint8_t *)w_minus);   // "-"
	LS027_disp_icon(22, 2, ((select == 2) && (fl == 1)) ? (uint8_t *)b_plus : (uint8_t *)w_plus);	 // "+"
	LS027_disp_icon(22, 18, ((select == 3) && (fl == 1)) ? (uint8_t *)b_minus : (uint8_t *)w_minus);   // "-"

	LS027_disp_icon(32, 2, ((select == 4) && (fl == 1)) ? (uint8_t *)b_plus : (uint8_t *)w_plus);	 // "+"
	LS027_disp_icon(32, 18, ((select == 5) && (fl == 1)) ? (uint8_t *)b_minus : (uint8_t *)w_minus);   // "-"
	LS027_disp_icon(38, 2, ((select == 6) && (fl == 1)) ? (uint8_t *)b_plus : (uint8_t *)w_plus);	 // "+"
	LS027_disp_icon(38, 18, ((select == 7) && (fl == 1)) ? (uint8_t *)b_minus : (uint8_t *)w_minus);   // "-"

	LS027_disp_icon(28, 22, ((select == 8) && (fl == 1)) ? (uint8_t *)b_cancel : (uint8_t *)w_cancel); // "中止"
	LS027_disp_icon(39, 22, ((select == 9) && (fl == 1)) ? (uint8_t *)b_set : (uint8_t *)w_set);	   // "セット"

	LS027_disp_number(16, 6, 0, month / 10 % 10, 0);	//月
	LS027_disp_number(22, 6, 0, month % 10, 0);		//月
	LS027_disp_icon(28, 6, (uint8_t *)l_slash);		//／
	LS027_disp_number(32, 6, 0, day / 10 % 10, 0);	//日
	LS027_disp_number(38, 6, 0, day % 10, 0);			//日

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}