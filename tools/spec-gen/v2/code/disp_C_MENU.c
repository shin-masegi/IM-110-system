uint8_t disp_C_MENU(uint8_t calmode, uint8_t select, uint8_t batvol, uint8_t fl)
{
	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}
	LS027_disp_icon(0, 1, (uint8_t *)t_cal_menu); // タイトル
	LS027_disp_battery_icon(batvol);		// 電池残量アイコン

	LS027_disp_datetime(); //日付時刻表示

	LS027_disp_icon(0, 13, (uint8_t *)icon_DISP);
	LS027_disp_icon(0, 18, (uint8_t *)icon_DISP);
	LS027_disp_icon(0, 23, (uint8_t *)icon_MEM);

	LS027_disp_icon(5, 13, (uint8_t *)icon_l_return); //長押しで前の画面へ
	LS027_disp_icon(5, 18, (uint8_t *)icon_select);   //選択
	LS027_disp_icon(5, 23, (uint8_t *)icon_decision); //決定

	if (cal_from_powerOn)
	{
		//電源 ON 経由の校正メニュー: 全校正モード共通の 4 ボタン配置 ("測定" 無し)
		//  select=0: ゼロ校正、=1: 2点校正、=2: 3点校正、=3: 校正リセット
		LS027_disp_icon(17,  7, ((select == 0) && (fl == 1)) ? (uint8_t *)b_0cal       : (uint8_t *)w_0cal);       // "ゼロ校正"
		LS027_disp_icon(34,  7, ((select == 1) && (fl == 1)) ? (uint8_t *)b_2pcal      : (uint8_t *)w_2pcal);      // "2点校正"
		LS027_disp_icon(17, 17, ((select == 2) && (fl == 1)) ? (uint8_t *)b_3pcal      : (uint8_t *)w_3pcal);      // "3点校正"
		LS027_disp_icon(34, 17, ((select == 3) && (fl == 1)) ? (uint8_t *)b_calreset_s : (uint8_t *)w_calreset_s); // "校正リセット"
	}
	else
	{
		//測定メニュー経由の従来配置: 現校正モードを除いた 3 種 + 「測定」(復帰)
		switch(calmode) {
		case CAL_MODE_Z:
			{
				//IM-110: 校正不可な相関式 (内部値 < 20) では 2点/3点アイコンを描画しない。
				//界面 (DEPTH) は相関式に依らず 2点/3点 非表示 (corr_idx=0 扱い)。
				uint8_t corr_idx;
				switch(Meas_Mode) {
				case MEAS_MODE_MLSS:
				default:             corr_idx = MLSS_MODE; break;
				case MEAS_MODE_SS:   corr_idx = SS_MODE;   break;
				case MEAS_MODE_TR:   corr_idx = TR_MODE;   break;
				case MEAS_MODE_DEPTH:corr_idx = 0;          break;
				}
				if (corr_idx >= EEP_CF_BASE_IDX)
				{
					LS027_disp_icon(17, 7, ((select == 0) && (fl == 1)) ? (uint8_t *)b_2pcal : (uint8_t *)w_2pcal);  // "2点校正"
					LS027_disp_icon(34, 7, ((select == 1) && (fl == 1)) ? (uint8_t *)b_3pcal : (uint8_t *)w_3pcal);  // "3点校正"
				}
			}
			LS027_disp_icon(17, 17, ((select == 2) && (fl == 1)) ? (uint8_t *)b_calreset_s : (uint8_t *)w_calreset_s); // "校正リセット"
			LS027_disp_icon(34, 17, ((select == 3) && (fl == 1)) ? (uint8_t *)b_measmode_s : (uint8_t *)w_measmode_s); // "測定"
			break;
		case CAL_MODE_2P:
			LS027_disp_icon(17, 7, ((select == 0) && (fl == 1)) ? (uint8_t *)b_0cal : (uint8_t *)w_0cal);  // "ゼロ校正"
			LS027_disp_icon(34, 7, ((select == 1) && (fl == 1)) ? (uint8_t *)b_3pcal : (uint8_t *)w_3pcal);  // "3点校正"
			LS027_disp_icon(17, 17, ((select == 2) && (fl == 1)) ? (uint8_t *)b_calreset_s : (uint8_t *)w_calreset_s); // "校正リセット"
			LS027_disp_icon(34, 17, ((select == 3) && (fl == 1)) ? (uint8_t *)b_measmode_s : (uint8_t *)w_measmode_s); // "測定"
		break;
		case CAL_MODE_3P:
			LS027_disp_icon(17, 7, ((select == 0) && (fl == 1)) ? (uint8_t *)b_0cal : (uint8_t *)w_0cal);  // "ゼロ校正"
			LS027_disp_icon(34, 7, ((select == 1) && (fl == 1)) ? (uint8_t *)b_2pcal : (uint8_t *)w_2pcal);  // "2点校正"
			LS027_disp_icon(17, 17, ((select == 2) && (fl == 1)) ? (uint8_t *)b_calreset_s : (uint8_t *)w_calreset_s); // "校正リセット"
			LS027_disp_icon(34, 17, ((select == 3) && (fl == 1)) ? (uint8_t *)b_measmode_s : (uint8_t *)w_measmode_s); // "測定"
		break;
		default:
		break;
		}
	}

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}