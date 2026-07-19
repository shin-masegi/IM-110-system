uint8_t disp_Depth_Setting(uint8_t flash_flg,float Interface_Threshold_tmp, uint8_t num_sel, uint8_t batvol)
{
	static uint8_t num[5] = {0, 0, 0, 0, 0}; // 表示中の数値(整数部5桁)
	num[0] = (uint8_t)(Interface_Threshold_tmp / 10000);
	num[1] = (uint8_t)((Interface_Threshold_tmp - (num[0] * 10000)) / 1000);
	num[2] = (uint8_t)((Interface_Threshold_tmp - (num[0] * 10000) - (num[1] * 1000)) / 100);
	num[3] = (uint8_t)((Interface_Threshold_tmp - (num[0] * 10000) - (num[1] * 1000) - (num[2] * 100)) / 10);
	num[4] = (uint8_t)(Interface_Threshold_tmp - (num[0] * 10000) - (num[1] * 1000) - (num[2] * 100) - (num[3] * 10));


	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}
	LS027_disp_icon(0, 1, (uint8_t *)t_depth_setting);	//界面設定
	LS027_disp_battery_icon(batvol);  // 電池残量アイコン
	LS027_disp_datetime();				   //日付時刻表示

	//5 桁の Interface_Threshold 設定値を大サイズで上段に表示
	//1 桁あたり 6 ユニット幅 (l_num = 48×96px)、x=14,20,26,32,38、y=5
	{
		const uint8_t x_pos[5] = {14, 20, 26, 32, 38};
		uint8_t i;
		for (i = 0; i < 5; i++)
		{
			if ((flash_flg) && (i == num_sel))
			{
				//選択桁は反転で点滅
				LS027_disp_icon(x_pos[i], 5, (uint8_t *)b_l_num[num[i]]);
			}
			else
			{
				LS027_disp_icon(x_pos[i], 5, (uint8_t *)l_num[num[i]]);
			}
		}
	}
	LS027_disp_icon(44, 11, (uint8_t *)l_mgl);		// "mg/L"

	LS027_disp_icon(0, 13, (uint8_t *)icon_DISP);
	LS027_disp_icon(0, 18, (uint8_t *)icon_DISP);
	LS027_disp_icon(0, 23, (uint8_t *)icon_MEM);

	LS027_disp_icon(5, 13, (uint8_t *)icon_l_return);	//長押しで元の画面
	LS027_disp_icon(5, 18, (uint8_t *)icon_next);	//次へ
	LS027_disp_icon(5, 23, (uint8_t *)icon_change);	//切替

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}