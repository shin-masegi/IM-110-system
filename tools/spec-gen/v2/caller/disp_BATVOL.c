// ===== Adjust.c : default (L813) =====
	default:
		if (disp_BATVOL(Y2) == DISP_OK)
		{
			operation_mode = BAT_2;
			timer_set(&disp_timer, DISP_CYCLE);
		}
		break;


// ===== Adjust.c : case BAT_2 (L821) =====
	case BAT_2:		//画面表示中
		if (!disp_timer)
		{
			operation_mode = BAT_1;
		}
		break;
	}

	//SW動作
	if (DISP_sw_check() >= 3)
	{
		DISP_sw_step = 0;
		operation_mode = LOWRANGE0;		//DO低レンジ調整に移る
	}
}


/**
 * @brief 			プローブ調整 (仮レベル): DO系スロットを流用し MEM で Adj_* を実行、DISP で次スロット
 * @detail			タイトル表示は既存 disp_BATVOL を流用 (仮)。mlss-calc-reference §8。
 *                  スロット割当: LOWRANGE0=LED PWM / LOWRANGE=MLSS AD0 / MIDRANGE0=清水ゼロ /
 *                  MIDRANGE=水深6m / HIRANGE0=相関式初期化 / HIRANGE=温度フィット / WAT05/20/35C=温度点
 *                  順序: LED PWM(動作点確定=空中1700) → MLSS AD0(span化) の順 (mlss-calc §8, 2026-07-21修正)
 */
void adj_probe(void)
{
	uint16_t slot  = operation_mode & 0xFF00;
	uint16_t phase = operation_mode & 0x00F0;

	if (phase == 0x00)					//画面描画 (タイトル流用=仮)
	{
		if (disp_BATVOL(Y2) == DISP_OK)
		{
			operation_mode = slot | 0x10;
			timer_set(&disp_timer, DISP_CYCLE);
		}
		return;
	}
	if (!disp_timer)					//表示更新
	{
		operation_mode = slot | 0x00;
	}

	//MEM 押下 = 実行
	if (MEM_sw_check() >= 3)
	{
		MEM_sw_step = 0;
		switch (slot)
		{