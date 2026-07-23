// ===== Adjust.c : case EEP_3 (L237) =====
	case EEP_3:		//0xAA書込
		if (!disp_timer)
		{
			per = ((float)EEP_Tadrs / ((float)EEP_PAGE_MAX * 6)) * 100.0;
			//EEPテスト画面表示
			if (disp_EEP2(disp_lr_yajirushi, per) == DISP_OK)
			{
				timer_set(&disp_timer, DISP_CYCLE);
				//矢印アイコンのトグル
				if (disp_lr_yajirushi == 0)
				{
					disp_lr_yajirushi = 1;
				}
				else
				{
					disp_lr_yajirushi = 0;
				}
			}
		}

		if (EEP_Tadrs < EEP_PAGE_MAX)
		{
			ret = eep_write_page(EEP_Tadrs);
			switch (ret)
			{

// ===== Adjust.c : case EEP_4 (L281) =====
	case EEP_4:		//0xAAベリファイ
		if (!disp_timer)
		{
			per = ((((float)EEP_PAGE_MAX * 1) + (float)EEP_Tadrs) / ((float)EEP_PAGE_MAX * 6)) * 100;
			//EEPテスト画面表示
			if (disp_EEP2(disp_lr_yajirushi, per) == DISP_OK)
			{
				timer_set(&disp_timer, DISP_CYCLE);
				//矢印アイコンのトグル
				if (disp_lr_yajirushi == 0)
				{
					disp_lr_yajirushi = 1;
				}
				else
				{
					disp_lr_yajirushi = 0;
				}
			}
		}
		ret = eep_write_page(EEP_Tadrs);

		if (EEP_Tadrs < EEP_PAGE_MAX)
		{
			ret = eep_read_page(EEP_Tadrs);
			switch (ret)
			{

// ===== Adjust.c : case EEP_5 (L338) =====
	case EEP_5:		//0x55書込
		if (!disp_timer)
		{
			per = ((((float)EEP_PAGE_MAX * 2) + (float)EEP_Tadrs) / ((float)EEP_PAGE_MAX * 6)) * 100;
			//EEPテスト画面表示
			if (disp_EEP2(disp_lr_yajirushi, per) == DISP_OK)
			{
				timer_set(&disp_timer, DISP_CYCLE);
				//矢印アイコンのトグル
				if (disp_lr_yajirushi == 0)
				{
					disp_lr_yajirushi = 1;
				}
				else
				{
					disp_lr_yajirushi = 0;
				}
			}
		}

		if (EEP_Tadrs < EEP_PAGE_MAX)
		{
			ret = eep_write_page(EEP_Tadrs);
			switch (ret)
			{

// ===== Adjust.c : case EEP_6 (L382) =====
	case EEP_6:		//0x55ベリファイ
		if (!disp_timer)
		{
			per = ((((float)EEP_PAGE_MAX * 3) + (float)EEP_Tadrs) / ((float)EEP_PAGE_MAX * 6)) * 100;
			//EEPテスト画面表示
			if (disp_EEP2(disp_lr_yajirushi, per) == DISP_OK)
			{
				timer_set(&disp_timer, DISP_CYCLE);
				//矢印アイコンのトグル
				if (disp_lr_yajirushi == 0)
				{
					disp_lr_yajirushi = 1;
				}
				else
				{
					disp_lr_yajirushi = 0;
				}
			}
		}

		if (EEP_Tadrs < EEP_PAGE_MAX)
		{
			ret = eep_read_page(EEP_Tadrs);
			switch (ret)
			{

// ===== Adjust.c : case EEP_7 (L438) =====
	case EEP_7:		//0x00書込
		if (!disp_timer)
		{
			per = ((((float)EEP_PAGE_MAX * 4) + (float)EEP_Tadrs) / ((float)EEP_PAGE_MAX * 6)) * 100;
			//EEPテスト画面表示
			if (disp_EEP2(disp_lr_yajirushi, per) == DISP_OK)
			{
				timer_set(&disp_timer, DISP_CYCLE);
				//矢印アイコンのトグル
				if (disp_lr_yajirushi == 0)
				{
					disp_lr_yajirushi = 1;
				}
				else
				{
					disp_lr_yajirushi = 0;
				}
			}
		}

		if (EEP_Tadrs < EEP_PAGE_MAX)
		{
			ret = eep_write_page(EEP_Tadrs);
			switch (ret)
			{

// ===== Adjust.c : case EEP_8 (L482) =====
	case EEP_8:		//0x00ベリファイ
		if (!disp_timer)
		{
			per = ((((float)EEP_PAGE_MAX * 5) + (float)EEP_Tadrs) / ((float)EEP_PAGE_MAX * 6)) * 100;
			//EEPテスト画面表示
			if (disp_EEP2(disp_lr_yajirushi, per) == DISP_OK)
			{
				timer_set(&disp_timer, DISP_CYCLE);
				//矢印アイコンのトグル
				if (disp_lr_yajirushi == 0)
				{
					disp_lr_yajirushi = 1;
				}
				else
				{
					disp_lr_yajirushi = 0;
				}
			}
		}

		if (EEP_Tadrs < EEP_PAGE_MAX)
		{
			ret = eep_read_page(EEP_Tadrs);
			switch (ret)
			{