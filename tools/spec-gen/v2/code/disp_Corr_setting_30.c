uint8_t disp_Corr_setting_30(uint8_t corr_val_disp, uint8_t num_sel, uint8_t batvol, uint8_t fl)
{
	//相関式 No. を 2 桁インクリメント入力で受け付ける画面 (01〜30)。
	//corr_val_disp = 表示値 (0..30、内部値 + 1)、num_sel = 編集中の桁 (0=10の位、1=1の位)
	//l_num サイズで横中央寄せ (x=22 / x=28、6 ユニット幅)、点滅は b_l_num
	uint8_t num[2];
	num[0] = corr_val_disp / 10;	//10の位 (0..3)
	num[1] = corr_val_disp % 10;	//1の位 (0..9)

	if (lcd_init(0) != LCD_OK)
	{
		return DISP_NG;
	}
	LS027_disp_icon(0, 1, (uint8_t *)t_correlation); // タイトル "相関式"
	LS027_disp_battery_icon(batvol);				 // 電池残量アイコン
	LS027_disp_datetime();							 // 日付時刻表示

	//大サイズ数字 2 桁を表示 (1 桁 = 6 ユニット、x=24/30、y=9)
	//点滅桁は b_l_num (反転)、それ以外は l_num (通常)
	{
		const uint8_t x_pos[2] = {24, 30};
		uint8_t i;
		for (i = 0; i < 2; i++)
		{
			if (fl && (i == num_sel))
			{
				LS027_disp_icon(x_pos[i], 9, (uint8_t *)b_l_num[num[i]]);
			}
			else
			{
				LS027_disp_icon(x_pos[i], 9, (uint8_t *)l_num[num[i]]);
			}
		}
	}

	//左列: SW アイコン (3 段構成 = 長押し戻る / 次の桁 / 桁値変更)
	LS027_disp_icon(0, 13, (uint8_t *)icon_DISP);
	LS027_disp_icon(0, 18, (uint8_t *)icon_DISP);
	LS027_disp_icon(0, 23, (uint8_t *)icon_MEM);

	LS027_disp_icon(5, 13, (uint8_t *)icon_l_return);	//長押しで戻る
	LS027_disp_icon(5, 18, (uint8_t *)icon_next);		//次へ
	LS027_disp_icon(5, 23, (uint8_t *)icon_change);		//切替

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}