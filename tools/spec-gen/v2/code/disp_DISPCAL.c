uint8_t disp_DISPCAL(uint16_t index, uint8_t erase, uint8_t batvol)
{
	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}
	LS027_disp_icon(0, 1, (uint8_t *)t_calhis); // タイトル
	LS027_disp_battery_icon(batvol);			  // 電池残量アイコン

	//操作表示仕様変更 2019/03/03 柵木
	LS027_disp_datetime(); //日付時刻表示

	//スイッチ表示
	LS027_disp_icon(0, 18, (uint8_t *)icon_DISP);
	LS027_disp_icon(0, 23, (uint8_t *)icon_MEM);

	LS027_disp_icon(5, 18, (uint8_t *)icon_change);	//切替
	LS027_disp_icon(5, 23, (uint8_t *)icon_next);		//次へ

	disp_cal_data(16, 6, index, CAL_NUM);	  //"履歴データ1つめ表示"
	LS027_disp_icon(16, 13, (uint8_t *)icon_line);				//区切り
	disp_cal_data(16, 14, index + 1, CAL_NUM); //"履歴データ2つめ表示"
	LS027_disp_icon(16, 21, (uint8_t *)icon_line);				//区切り
	disp_cal_data(16, 22, index + 2, CAL_NUM); //"履歴データ3つめ表示"

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}