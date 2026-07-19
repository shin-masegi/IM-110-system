uint8_t disp_CAL_HSEL(uint16_t index, uint16_t cursor, uint16_t count, uint8_t flash_flg, uint8_t batvol)
{
	uint8_t r;
	const uint8_t row_y[3] = {6, 14, 22};

	if (lcd_init(0) != LCD_OK)
	{
		return DISP_NG;
	}
	LS027_disp_icon(0, 1, (uint8_t *)t_disp_meas_history); // タイトル (履歴流用)
	LS027_disp_battery_icon(batvol);
	LS027_disp_datetime();

	//スイッチ表示: DISP 短押し=No.送り(次へ) / MEM 短押し=校正 / DISP 長押し=校正画面へ戻る
	LS027_disp_icon(0, 18, (uint8_t *)icon_DISP);
	LS027_disp_icon(5, 18, (uint8_t *)icon_next);		//次へ (No.送り)
	LS027_disp_icon(0, 23, (uint8_t *)icon_MEM);
	LS027_disp_icon(5, 23, (uint8_t *)icon_measure);	//校正 (アイコン流用)

	for (r = 0; r < 3; r++)
	{
		uint16_t idx = index + r;
		if (idx < count)
		{
			//選択行は flash_flg に従い No. を点滅、それ以外は常に表示
			uint8_t show_no = (idx == cursor) ? flash_flg : 1;
			disp_cal_hsel_row(16, row_y[r], idx, show_no);
		}
		if (r < 2)
		{
			LS027_disp_icon(16, row_y[r] + 7, (uint8_t *)icon_line);	//区切り
		}
	}

	SPI_transmitLCD();
	return DISP_OK;
}