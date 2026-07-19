uint8_t disp_MLSS_SCal_1(uint8_t flash_flg,float Cal_SetVal_1_tmp, uint8_t num_sel, uint8_t batvol)
{
	//MLSS/DEPTH: 5桁 (10000/1000/100/10/1 の位)、SS: 4桁右寄せ (1000/100/10/1 の位、num[0] 未使用)
	//TR: 4桁 + 小数点 (100/10/1/0.1 の位、num[1..4] 使用、x=29/32/35/41 に配置し x=38 に "." アイコン)
	static uint8_t num[5] = {0, 0, 0, 0, 0};
	if (Meas_Mode == MEAS_MODE_SS)
	{
		num[0] = 0;	//SS は 4桁固定、10000 の位は未使用
		num[1] = (uint8_t)(Cal_SetVal_1_tmp / 1000);
		num[2] = (uint8_t)((Cal_SetVal_1_tmp - (num[1] * 1000)) / 100);
		num[3] = (uint8_t)((Cal_SetVal_1_tmp - (num[1] * 1000) - (num[2] * 100)) / 10);
		num[4] = (uint8_t)(Cal_SetVal_1_tmp - (num[1] * 1000) - (num[2] * 100) - (num[3] * 10));
	}
	else if (Meas_Mode == MEAS_MODE_TR)
	{
		//0.1 刻みで丸めて整数化 (tenths = 1 〜 2000 を想定)
		uint16_t tenths = (uint16_t)(Cal_SetVal_1_tmp * 10.0f + 0.5f);
		num[0] = 0;
		num[1] = tenths / 1000;			//100の位
		num[2] = (tenths / 100) % 10;	//10の位
		num[3] = (tenths / 10) % 10;	//1の位
		num[4] = tenths % 10;			//0.1の位
	}
	else
	{
		num[0] = (uint8_t)(Cal_SetVal_1_tmp / 10000);
		num[1] = (uint8_t)((Cal_SetVal_1_tmp - (num[0] * 10000)) / 1000);
		num[2] = (uint8_t)((Cal_SetVal_1_tmp - (num[0] * 10000) - (num[1] * 1000)) / 100);
		num[3] = (uint8_t)((Cal_SetVal_1_tmp - (num[0] * 10000) - (num[1] * 1000) - (num[2] * 100)) / 10);
		num[4] = (uint8_t)(Cal_SetVal_1_tmp - (num[0] * 10000) - (num[1] * 1000) - (num[2] * 100) - (num[3] * 10));
	}


	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}
	LS027_disp_icon(0, 1, (uint8_t *)t_2p_cal);	//2点校正
	LS027_disp_battery_icon(batvol);  // 電池残量アイコン
	LS027_disp_datetime();				   //日付時刻表示

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

		LS027_disp_number(29, 18, 1, num[0], 0); //スパン校正設定値10000の位
		LS027_disp_number(32, 18, 1, num[1], 0); //スパン校正設定値1000の位
		LS027_disp_number(35, 18, 1, num[2], 0); //スパン校正設定値100の位
		LS027_disp_number(38, 18, 1, num[3], 0); //スパン校正設定値10の位
		LS027_disp_number(41, 18, 1, num[4], 0); //スパン校正設定値1の位
		if (flash_flg) {
			//編集中桁を b_m_num (反転) で再描画して点滅させる (num_sel = 0..4)
			static const uint8_t mlss_x[5] = {29, 32, 35, 38, 41};
			LS027_disp_icon(mlss_x[num_sel], 18, (uint8_t *)b_m_num[num[num_sel]]);
		}

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

		//SS スパン校正設定値 4桁 (1000/100/10/1 の位、右寄せで 1 の位を x=41 に揃える)
		LS027_disp_number(32, 18, 1, num[1], 0); //スパン校正設定値1000の位
		LS027_disp_number(35, 18, 1, num[2], 0); //スパン校正設定値100の位
		LS027_disp_number(38, 18, 1, num[3], 0); //スパン校正設定値10の位
		LS027_disp_number(41, 18, 1, num[4], 0); //スパン校正設定値1の位
		if (flash_flg) {
			//編集中桁を b_m_num (反転) で再描画 (num_sel = 1..4、index 0 は未使用)
			static const uint8_t ss_x[5] = {0, 32, 35, 38, 41};
			LS027_disp_icon(ss_x[num_sel], 18, (uint8_t *)b_m_num[num[num_sel]]);
		}

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

		//TR スパン校正設定値 4桁 + 小数点 (100/10/1 の位、"."、0.1 の位)
		//SCal_2/3 で LS027_disp_number(41, ..., 1) が描画する位置に合わせる:
		//  桁幅 m_num=3、ドット幅 m_dot=2 → 右から 41 / 39(".") / 36 / 33 / 30
		LS027_disp_number(30, 18, 1, num[1], 0); //100の位
		LS027_disp_number(33, 18, 1, num[2], 0); //10の位
		LS027_disp_number(36, 18, 1, num[3], 0); //1の位
		LS027_disp_icon(39, 18, (uint8_t *)m_dot); //小数点 (幅2単位)
		LS027_disp_number(41, 18, 1, num[4], 0); //0.1の位
		if (flash_flg) {
			//編集中桁を b_m_num (反転) で再描画 (num_sel = 1..4、"." は飛ばす)
			static const uint8_t tr_x[5] = {0, 30, 33, 36, 41};
			LS027_disp_icon(tr_x[num_sel], 18, (uint8_t *)b_m_num[num[num_sel]]);
		}

		LS027_disp_number(36, 25, 2, TR_MODE + 1, 0); //TR_MODE
		break;

	case MEAS_MODE_DEPTH:
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

		if (((MLSS == FLT_MAX)||(MLSS >= 20000)||(MLSS <= -99))&& (flash_flg == 1))
		{
			lcd_fill_box(30, 18, 14, 6, 1); // 白塗り
		}
		else
		{
			if (MLSS > 20000) {
				MLSS = 20000;
			}
			else if (MLSS < -99) {
				MLSS = -99;
			}
			LS027_disp_number(41, 18, 1, MLSS, 0); //MLSS
		}
		LS027_disp_icon(44, 21, (uint8_t *)s_mgl); //"mg/L"
		LS027_disp_number(36, 25, 2, MLSS_MODE + 1, 0); //MLSS_MODE
		break;
	}
	LS027_disp_icon(25, 25, (uint8_t *)icon_mode); // "モード"
	LS027_disp_icon(38, 25, (uint8_t *)s_slash); // "/"
	LS027_disp_number(42, 25, 2, 30, 0); //"30"

	LS027_disp_icon(0, 13, (uint8_t *)icon_DISP);
	LS027_disp_icon(0, 18, (uint8_t *)icon_DISP);
	LS027_disp_icon(0, 23, (uint8_t *)icon_MEM);

	LS027_disp_icon(5, 13, (uint8_t *)icon_l_return);	//長押しで元の画面
	LS027_disp_icon(5, 18, (uint8_t *)icon_next);	//次へ
	LS027_disp_icon(5, 23, (uint8_t *)icon_change);	//切替

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}