uint8_t disp_MLSS_ZCal_3(uint8_t flash_flg, uint8_t msg_flag, uint8_t batvol)
{
	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}
	LS027_disp_icon(0, 1, (uint8_t *)t_zero_cal);	//ゼロ校正
	LS027_disp_battery_icon(batvol);  // 電池残量アイコン
	LS027_disp_datetime();				   //日付時刻表示

	if (msg_flag) {
		LS027_disp_icon(20, 11, (uint8_t *)msg_cal_comp); // 校正完了アイコン
	}
	else {
		switch(Meas_Mode) {
		case MEAS_MODE_MLSS:
		default:
			if (((MLSS == FLT_MAX)||(MLSS >= 20000)||(MLSS <= -99))&& (flash_flg == 1))
			{
				lcd_fill_box(15, 5, 30, 12, 1); // 白塗り
			}
			else
			{
				if (MLSS > 20000) {
					MLSS = 20000;
				}
				else if (MLSS < -99) {
					MLSS = -99;
				}
				LS027_disp_number(38, 5, 0, MLSS, 0); //MLSS
			}
			LS027_disp_icon(44, 11, (uint8_t *)l_mgl); // "mg/L"
			LS027_disp_number(36, 25, 2, MLSS_MODE + 1, 0); //MLSS_MODE
			break;

		case MEAS_MODE_SS:
			if (((SS == FLT_MAX)||(SS >= 1000)||(SS <= -99))&& (flash_flg == 1))
			{
				lcd_fill_box(15, 5, 30, 12, 1); // 白塗り
			}
			else
			{
				if (SS > 1000) {
					SS = 1000;
				}
				else if (SS < -99) {
					SS = -99;
				}
				LS027_disp_number(38, 5, 0, SS, 0); //SS
			}
			LS027_disp_icon(44, 11, (uint8_t *)l_mgl); // "mg/L"
			LS027_disp_number(36, 25, 2, SS_MODE + 1, 0); //SS_MODE
			break;

		case MEAS_MODE_TR:
			if (((Transparency == FLT_MAX)||(Transparency >= 100.0f)||(Transparency <= -9.9))&& (flash_flg == 1))
			{
				lcd_fill_box(15, 5, 30, 12, 1); // 白塗り
			}
			else
			{
				if (Transparency > 100.0) {
					Transparency = 100.0;
				}
				else if (Transparency < -9.9) {
					Transparency = -9.9;
				}
				LS027_disp_number(38, 5, 0, Transparency, 1); //透視度
			}
			LS027_disp_icon(44, 11, (uint8_t *)m_cm); // "cm"
			LS027_disp_number(36, 25, 2, TR_MODE + 1, 0); //TR_MODE
			break;

		case MEAS_MODE_DEPTH:
			//界面ゼロ校正: 測定画面と同じ位置に Depth + "m" のみ。下段の MLSS / mg/L は表示しない
			if (((Depth == FLT_MAX)||(Depth >= 6.0)||(Depth <= -0.99))&& (flash_flg == 1))
			{
				lcd_fill_box(15, 5, 30, 12, 1); // 白塗り
			}
			else
			{
				if (Depth > 6.0) {
					Depth = 6.0;
				}
				else if (Depth < -0.99) {
					Depth = -0.99;
				}
				LS027_disp_number(38, 5, 0, Depth, 2); //Depth
			}
			LS027_disp_icon(45, 11, (uint8_t *)m_m); // "m"
			break;
		}
		if (Meas_Mode != MEAS_MODE_DEPTH) {
			//測定モード No / 30 表示 (MLSS/SS/TR のみ、DEPTH は無し)
			LS027_disp_icon(25, 25, (uint8_t *)icon_mode); // "モード"
			LS027_disp_icon(38, 25, (uint8_t *)s_slash); // "/"
			LS027_disp_number(42, 25, 2, 30, 0); //"30"
		}
	}

	//LS027_disp_icon(0, 18, (uint8_t *)icon_DISP);
	LS027_disp_icon(0, 23, (uint8_t *)icon_MEM);

	//LS027_disp_icon(5, 18, (uint8_t *)icon_change); 	//切替
	LS027_disp_icon(5, 23, (uint8_t *)icon_cal_start);	//校正開始

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}