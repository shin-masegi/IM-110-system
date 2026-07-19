uint8_t disp_PRGVER(uint8_t del_flag)
{
	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}

	LS027_disp_icon(0, 0, (uint8_t *)t_prog_Ver);		//タイトル
	LS027_disp_number(36, 9, 0, (float)FW_VER1 / 100.0, 2);	//プログラムVer.
	LS027_disp_icon(0, 17, (uint8_t *)icon_DISP);
	LS027_disp_icon(5, 17, (uint8_t *)icon_next); //次へ

	if (!del_flag)
	{
			LS027_disp_icon(0,22,(uint8_t *)icon_MEM);
			LS027_disp_icon(5,22,(uint8_t *)icon_l_erase);	//長押しで消去
	}
	else
	{
			LS027_disp_icon(0,22,(uint8_t *)icon_erase);	//消去完了
	}

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}