uint8_t disp_SETTIME(uint8_t select, uint8_t batvol, uint8_t fl)
{

	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}
	LS027_disp_icon(0, 1, (uint8_t *)t_set_time); // タイトル
	LS027_disp_battery_icon(batvol);   // 電池残量アイコン

	//操作表示仕様変更 2019/03/03 柵木
	//LS027_disp_datetime();						//日付時刻表示

	LS027_disp_icon(0, 18, (uint8_t *)icon_DISP);
	LS027_disp_icon(0, 23, (uint8_t *)icon_MEM);

	LS027_disp_icon(5, 18, (uint8_t *)icon_select);   //選択
	LS027_disp_icon(5, 23, (uint8_t *)icon_decision); //決定

	LS027_disp_icon(44, 10, ((select == 0) && (fl == 1)) ? (uint8_t *)b_plus : (uint8_t *)w_plus);		//+
	LS027_disp_icon(44, 16, ((select == 1) && (fl == 1)) ? (uint8_t *)b_minus : (uint8_t *)w_minus);		//-
	LS027_disp_icon(28, 21, ((select == 2) && (fl == 1)) ? (uint8_t *)b_set : (uint8_t *)w_set);			//セット
	LS027_disp_icon(39, 21, ((select == 3) && (fl == 1)) ? (uint8_t *)b_cancel : (uint8_t *)w_cancel);	//中止
	LS027_disp_cal(17, 5, 0);							//年月日
	LS027_disp_number(15, 9, 0, hour / 10 % 10, 0);	//時
	LS027_disp_number(21, 9, 0, hour % 10, 0);		//時
	LS027_disp_icon(27, 9, (uint8_t *)l_colon);		//:
	LS027_disp_number(31, 9, 0, min / 10 % 10, 0);	//分
	LS027_disp_number(37, 9, 0, min % 10, 0);			//分

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}