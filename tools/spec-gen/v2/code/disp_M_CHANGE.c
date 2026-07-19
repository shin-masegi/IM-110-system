uint8_t disp_M_CHANGE(uint8_t select, uint8_t flg, uint8_t batvol, uint8_t fl)
{
	if (lcd_init(0) != LCD_OK)
	{					// LCD表示クリア(＆LCD表示バッファ更新OKか確認)
		return DISP_NG; // 更新NGのため終了
	}
	LS027_disp_icon(0, 1, (uint8_t *)t_measure); // タイトル
	LS027_disp_battery_icon(batvol);		// 電池残量アイコン

	LS027_disp_datetime(); //日付時刻表示

	LS027_disp_icon(0, 18, (uint8_t *)icon_DISP);
	LS027_disp_icon(0, 23, (uint8_t *)icon_MEM);

	LS027_disp_icon(5, 18, (uint8_t *)icon_select);   //選択
	LS027_disp_icon(5, 23, (uint8_t *)icon_decision); //決定

	switch(flg)
	{
	case 0:
	default:
		LS027_disp_icon(18, 7, ((select == 0) && (fl == 1)) ? (uint8_t *)b_MLSS : (uint8_t *)w_MLSS);  // "MLSS"
		LS027_disp_icon(34, 7, ((select == 1) && (fl == 1)) ? (uint8_t *)b_SS : (uint8_t *)w_SS);  // "SS"
		LS027_disp_icon(18, 18, ((select == 2) && (fl == 1)) ? (uint8_t *)b_Transparency : (uint8_t *)w_Transparency); // "透視度"
		//「界面」アイコンは廃止 (設定メニュー側で界面しきい値だけ設定する仕様に移行)
		break;
	case 1:
		LS027_disp_icon(15, 6, (uint8_t *)msg_MLSS);
		break;
	case 2:
		LS027_disp_icon(15, 6, (uint8_t *)msg_SS);
		break;
	case 3:
		LS027_disp_icon(15, 6, (uint8_t *)msg_Transparency);
		break;
	case 4:
		LS027_disp_icon(15, 6, (uint8_t *)msg_Interface);
		break;
	}

	SPI_transmitLCD(); // LCD表示

	return DISP_OK;
}