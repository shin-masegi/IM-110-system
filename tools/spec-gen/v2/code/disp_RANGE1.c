uint8_t disp_RANGE1(uint16_t op_mode, uint16_t range_mv)
{
	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}
	switch (op_mode)
	{
	case LOWRANGE:							//低レンジ調整待機中
		LS027_disp_icon(0, 1, (uint8_t *)t_range_low);		//タイトル
		break;
	case MIDRANGE:							//中レンジ調整待機中
		LS027_disp_icon(0, 1, (uint8_t *)t_range_middle);	//タイトル
		break;
	case HIRANGE:							//高レンジ調整待機中
		LS027_disp_icon(0, 1, (uint8_t *)t_range_high);		//タイトル
		break;
	case LOWRANGE0:							//低レンジゼロ調整待機中
		LS027_disp_icon(0, 1, (uint8_t *)t_range0_low);		//タイトル
		break;
	case MIDRANGE0:							//中レンジゼロ調整待機中
		LS027_disp_icon(0, 1, (uint8_t *)t_range0_middle);	//タイトル
		break;
	case HIRANGE0:							//高レンジゼロ調整待機中
		LS027_disp_icon(0, 1, (uint8_t *)t_range0_high);		//タイトル
		break;
	default:								//その他
		lcd_fill_box(0, 1, 15, 4, 0);		//黒塗り
		break;
	}
	LS027_disp_number(36, 6, 0, (float)range_mv, 0);	//電圧値
	LS027_disp_icon(43, 14, (uint8_t *)m_mVoltage);	//mV

	LS027_disp_icon(0, 18, (uint8_t *)icon_DISP);
	LS027_disp_icon(0, 23, (uint8_t *)icon_MEM);

	LS027_disp_icon(5, 18, (uint8_t *)icon_next);  //次へ
	LS027_disp_icon(5, 23, (uint8_t *)icon_start); //開始

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}