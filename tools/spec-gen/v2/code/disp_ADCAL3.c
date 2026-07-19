uint8_t disp_ADCAL3(float fdo, uint8_t ddo, float temp, uint8_t batvol, uint8_t msg_flag)
{
	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}
	LS027_disp_icon(0, 1, (uint8_t *)t_cal);   // タイトル
	LS027_disp_battery_icon(batvol); // 電池残量アイコン

	//操作表示仕様変更 2019/03/03 柵木
	LS027_disp_datetime();				   //日付時刻表示
	LS027_disp_airbar(AIR_PER);			   //空気飽和率バー表示
	LS027_disp_icon(25, 5, (uint8_t *)icon_airpercent); //空気飽和率
	LS027_disp_number(41, 5, 2, AIR_PER, 0); //空気飽和率％
	LS027_disp_icon(44, 5, (uint8_t *)s_percent);	   //%

	//校正後メッセージ表示処理
	if (!msg_flag)
	{
		LS027_disp_icon(0, 13, (uint8_t *)icon_DISP);
		LS027_disp_icon(0, 18, (uint8_t *)icon_POWER);
		LS027_disp_icon(0, 23, (uint8_t *)icon_MEM);

		LS027_disp_icon(5, 13, (uint8_t *)icon_l_calhis);		//長押しで構成履歴
		LS027_disp_icon(5, 18, (uint8_t *)icon_pow_reset); 	//入れ直し測定
		LS027_disp_icon(5, 23, (uint8_t *)icon_cal_start);	//校正開始

		if (fdo == FLT_MAX)
		{
			lcd_fill_box(15, 9, 30, 12, 1); // 白塗り
		}
		else
		{
			//表示桁ズレ対策 2019/03/03 柵木
			if (fdo > 20.0)
			{
				fdo = 20.0;
			}
			else if (fdo < -0.99)
			{
				fdo = -0.99;
			}
			LS027_disp_number(38, 9, 0, fdo, ddo); // 酸素濃度表示
		}
		LS027_disp_icon(44, 15, (uint8_t *)l_mgl); // "mg/L"
		if (temp == FLT_MAX)
		{
			lcd_fill_box(30, 22, 14, 6, 1); // 白塗り
		}
		else
		{
			//表示桁ズレ対策 2019/03/12 柵木
			if (temp > 50.0)
			{
				temp = 50.0;
			}
			else if (temp < -9.9)
			{
				temp = -9.9;
			}
			LS027_disp_number(41, 22, 1, temp, 1); // 温度表示
		}
		LS027_disp_icon(45, 25, (uint8_t *)m_temp); // "℃"
	}
	else
	{
		//センサーメッセージ表示
		LS027_disp_icon(20, 11, (uint8_t *)msg_cal_comp);
	}

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}