uint8_t disp_MLSS_SCal_2(uint8_t flash_flg, float bar, uint8_t batvol)
{
	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}
	LS027_disp_icon(0, 1, (uint8_t *)t_2p_cal);	//2点校正
	LS027_disp_battery_icon(batvol);  // 電池残量アイコン
	LS027_disp_datetime();				   //日付時刻表示

	LS027_disp_icon(3, 8, (flash_flg) ? (uint8_t *)icon_wait1 : (uint8_t *)icon_wait2); // 矢印アイコン

	switch(Meas_Mode) {
	case MEAS_MODE_MLSS:
	case MEAS_MODE_DEPTH:
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
		LS027_disp_number(41, 18, 1, MLSS_Cal_SetVal_1, 0); //スパン校正設定値
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
		LS027_disp_number(41, 18, 1, SS_Cal_SetVal_1, 0); //スパン校正設定値
		LS027_disp_number(36, 25, 2, SS_MODE + 1, 0); //SS_MODE
		break;

	case MEAS_MODE_TR:
		if (((Transparency == FLT_MAX)||(Transparency >= 100.0f)||(Transparency <= -9.9))&& (flash_flg == 1))
		{
			lcd_fill_box(15, 5, 30, 12, 1); // 白塗り
		}
		else
		{
			if (Transparency > 100.0f) {
				Transparency = 100.;
			}
			else if (Transparency < -9.9) {
				Transparency = -9.9;
			}
			LS027_disp_number(38, 5, 0, Transparency, 1); //透視度
		}
		LS027_disp_icon(44, 11, (uint8_t *)m_cm); // "cm"
		LS027_disp_number(41, 18, 1, TR_Cal_SetVal_1, 1); //スパン校正設定値 (小数点1桁)
		LS027_disp_number(36, 25, 2, TR_MODE + 1, 0); //TR_MODE
		break;

	}

	LS027_disp_icon(25, 25, (uint8_t *)icon_mode); // "モード"
	LS027_disp_icon(38, 25, (uint8_t *)s_slash); // "/"
	LS027_disp_number(42, 25, 2, 30, 0); //"30"

	LS027_disp_icon(0, 23, (uint8_t *)icon_MEM);
	LS027_disp_icon(5, 23, (uint8_t *)icon_abort);  //中止

	LS027_disp_bar_icon(bar);		   // 進捗バー

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}