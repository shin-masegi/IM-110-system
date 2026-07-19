uint8_t disp_STRTDSP2(void)
{

	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}

	//校正
	LS027_disp_icon(1, 1, (uint8_t *)msg_cal_menu);
	//画面へ
	LS027_disp_icon(14, 1, (uint8_t *)msg_toscr);
	//icon_MEM
	LS027_disp_icon(3, 9, (uint8_t *)icon_MEM);
	//を押しながら、
	LS027_disp_icon(9, 9, (uint8_t *)msg_push1);
	//icon_POWER
	LS027_disp_icon(26, 9, (uint8_t *)icon_POWER);
	//を押しながら、
	LS027_disp_icon(32, 9, (uint8_t *)msg_push2);

	//設定メニュー
	LS027_disp_icon(1, 16, (uint8_t *)msg_setting_menu);
	//画面へ
	LS027_disp_icon(29, 16, (uint8_t *)msg_toscr);
	//icon_DISP
	LS027_disp_icon(3, 24, (uint8_t *)icon_DISP);
	//を押しながら、
	LS027_disp_icon(9, 24, (uint8_t *)msg_push1);
	//icon_POWER
	LS027_disp_icon(26, 24, (uint8_t *)icon_POWER);
	//を押しながら、
	LS027_disp_icon(32, 24, (uint8_t *)msg_push2);

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}