uint8_t disp_DISPHIS(uint16_t index, uint8_t erase, uint8_t batvol)
{
	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}
	LS027_disp_icon(0, 1, (uint8_t *)t_disp_meas_history); // タイトル
	LS027_disp_battery_icon(batvol);			  // 電池残量アイコン

	//操作表示仕様変更 2019/03/03 柵木
	LS027_disp_datetime(); //日付時刻表示

	//スイッチ表示
	if (index < 3)
	{
		if (erase)
		{
			LS027_disp_icon(0, 13, (uint8_t *)icon_erase);
		}
		else
		{
			LS027_disp_icon(0, 13, (uint8_t *)icon_DISP);
		}
		LS027_disp_icon(0, 23, (uint8_t *)icon_MEM);

		if (!erase)
		{
			LS027_disp_icon(5, 13, (uint8_t *)icon_measure);	//測定
		}
		LS027_disp_icon(0, 18, (uint8_t *)icon_l_erase);	//長押し消去
		LS027_disp_icon(5, 23, (uint8_t *)icon_next);		//次へ
	}
	else
	{
		LS027_disp_icon(0, 18, (uint8_t *)icon_DISP);
		LS027_disp_icon(0, 23, (uint8_t *)icon_MEM);

		LS027_disp_icon(5, 18, (uint8_t *)icon_measure); //測定
		LS027_disp_icon(5, 23, (uint8_t *)icon_next);	//次へ
	}

	disp_his_data(16, 6, index, HIS_NUM);	  //"履歴データ1つめ表示"
	LS027_disp_icon(16, 13, (uint8_t *)icon_line);				//区切り
	disp_his_data(16, 14, index + 1, HIS_NUM); //"履歴データ2つめ表示"
	LS027_disp_icon(16, 21, (uint8_t *)icon_line);				//区切り
	disp_his_data(16, 22, index + 2, HIS_NUM); //"履歴データ3つめ表示"

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}