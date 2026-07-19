uint8_t disp_WAT2(uint16_t op_mode, float ftemp)
{
	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}
	switch (op_mode)
	{
	case WAT05C:				   // 水温5℃調整完了
		LS027_disp_icon(0, 1, (uint8_t *)t_temp5C); // タイトル
		break;
	case WAT20C:					// 水温5℃調整完了
		LS027_disp_icon(0, 1, (uint8_t *)t_temp20C); // タイトル
		break;
	case WAT35C:					// 水温5℃調整完了
		LS027_disp_icon(0, 1, (uint8_t *)t_temp35C); // タイトル
		break;
	default:						  // その他
		lcd_fill_box(0, 1, 15, 4, 0); // 黒塗り
		break;
	}
	LS027_disp_number(36, 10, 0, ftemp, 1); // 水温
	LS027_disp_icon(43, 18, (uint8_t *)m_temp);		 // "℃"

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}