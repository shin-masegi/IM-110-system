/**
 * @file Calc.c
 * @brief 測定値、水温計算
 * @author 飯島電子 柵木
 * @date 2018/07/26
 */
#define CALC_C
#include "stm32l4xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Normal.h"
#include "Calc.h"
#include "Setting.h"
#include "mainSub.h"
#include "Adc.h"
#include "Adjust.h"
#include "AllDef.h"
#include <stdint.h>
#include "Timer.h"
#include "LinkSerial.h"

/********************************************************/
/*	マクロ定義                                        */
/********************************************************/
/* 水温計算 */

//AD8237と2次試作基板回路に合わせて係数変更 2019/02/11 柵木

/*
#define	FT_A	(1.09169E-09)		// 水温計算 三次係数
#define	FT_B	(3.07907E-06)		// 水温計算 二次係数
#define	FT_C	(5.62158E-02)		// 水温計算 一次係数
#define	FT_D	(-8.20578E+01)		// 水温計算 定数項
*/
#define FT_A (0)			// 水温計算 三次係数
#define FT_B (0)			// 水温計算 二次係数
#define FT_C (2.8612E-02) 	// 水温計算 一次係数 2.28424E-02 -> 2.8612E-02 2022/01/19 柵木
#define FT_D (-1.8340E+01)	// 水温計算 定数項 -1.22228E+01 -> -1.8340E+01 2022/01/19 柵木

//*310		JIS K0102-2016対応の飽和表ﾃｰﾌﾞﾙ
//*310		※各飽和値を100倍した値をセット
//*310		(..[n][]: n=水温+10 (上桁のみ), ..[][c]: c=0/1 : 淡水/海水)
//*310		備考：水温測定範囲(仕様)±5℃、JIS K0102-2016 表32.1範囲外は近似式で求めた値を使用
const int SatrTable[][2] = {
	{1989, 1525}, //  -10℃
	{1925, 1478}, //   -9℃
	{1863, 1434}, //   -8℃
	{1804, 1392}, //   -7℃
	{1748, 1351}, //   -6℃
	{1695, 1313}, //   -5℃
	{1644, 1276}, //   -4℃
	{1595, 1241}, //   -3℃
	{1549, 1208}, //   -2℃
	{1504, 1176}, //   -1℃
	{1462, 1145}, //    0℃
	{1422, 1116}, //    1℃
	{1383, 1088}, //    2℃
	{1346, 1061}, //    3℃
	{1311, 1035}, //    4℃
	{1277, 1011}, //    5℃
	{1245, 988},  //    6℃
	{1214, 965},  //    7℃
	{1184, 943},  //    8℃
	{1156, 922},  //    9℃
	{1129, 903},  //   10℃
	{1103, 884},  //   11℃
	{1078, 865},  //   12℃
	{1054, 847},  //   13℃
	{1031, 830},  //   14℃
	{1008, 814},  //   15℃
	{987, 798},   //   16℃
	{967, 783},   //   17℃
	{947, 768},   //   18℃
	{928, 754},   //   19℃
	{909, 739},   //   20℃
	{892, 726},   //   21℃
	{874, 713},   //   22℃
	{858, 701},   //   23℃
	{842, 689},   //   24℃
	{826, 677},   //   25℃
	{811, 666},   //   26℃
	{797, 655},   //   27℃
	{783, 644},   //   28℃
	{769, 634},   //   29℃
	{756, 624},   //   30℃
	{743, 613},   //   31℃
	{731, 604},   //   32℃
	{718, 595},   //   33℃
	{707, 586},   //   34℃
	{695, 577},   //   35℃
	{684, 568},   //   36℃
	{673, 560},   //   37℃
	{662, 551},   //   38℃
	{652, 543},   //   39℃
	{641, 535},   //   40℃
	{631, 528},   //   41℃
	{621, 520},   //   42℃
	{612, 512},   //   43℃
	{602, 505},   //   44℃
	{593, 498},   //   45℃
	{583, 490},   //   46℃
	{574, 482},   //   47℃
	{565, 475},   //   48℃
	{556, 468},   //   49℃
	{547, 460},   //   50℃
	{537, 453},   //   51℃
	{528, 446},   //   52℃
	{519, 438},   //   53℃
	{509, 430},   //   54℃
	{500, 422}	//   55℃
};

/**
 * @brief 			低温側補正前水温値FT(n)を計算する
 * @detail			FTLにFT(Y0)が格納される
 * 					Y0は水温アンプmV値
 * @attention
 */
void calc1(void)
{
	FTL = (FT_A * Y0 * Y0 * Y0) + (FT_B * Y0 * Y0) + (FT_C * Y0) + FT_D;
}

/**
 * @brief 			低温側水温値WTempを計算する
 * @detail			TLzr,TLspで補正する
 * 					FTLは補正前水温値FT(n)
 * @attention
 */
void calc2(void)
{
	WTemp = TLsp * FTL + TLzr + TOffset;
}

/**
 * @brief 			高温側補正前水温値FT(n)を計算する
 * @detail			FTHにFT(Y0)が格納される
 * 					Y0は水温アンプmV値
 * @attention
 */
void calc3(void)
{
	FTH = (FT_A * Y0 * Y0 * Y0) + (FT_B * Y0 * Y0) + (FT_C * Y0) + FT_D;
}

/**
 * @brief 			高温側水温値WTempを計算する
 * @detail			THzr,THspで補正する
 * 					FTHは補正前水温値FT(n)
 * @attention
 */
void calc4(void)
{
	WTemp = THsp * FTH + THzr + TOffset;
}

/**
 * @brief 			35℃以上高温側補正前水温値FT(n)を計算する
 * @detail			FTHにFT(Y0)が格納される
 * 					Y0は水温アンプmV値
 * @attention		現状はcalc3と全く同じ
 */
void calc3_over(void)
{
	FTH = (FT_A * Y0 * Y0 * Y0) + (FT_B * Y0 * Y0) + (FT_C * Y0) + FT_D;
}

/**
 * @brief 			35℃以上高温側水温値WTempを計算する
 * @detail			TOzr,TOspで補正する
 * 					FTHは補正前水温値FT(n)
 * @attention
 */
void calc4_over(void)
{
	WTemp = TOsp * FTH + TOzr + TOffset;
}

/**
 * @brief 			未使用計算式
 * @detail
 * @attention		何もしない
 */
void calc5(void)
{
	//何もしない
}

/**
 * @brief 			レンジ補正後DOmV値Rを計算する
 * @param[in]		range	レンジ番号（0:低 1:中 2:高）
 * @detail			ADsp[r]は各レンジの補正係数
 * 					中レンジを1倍として低、高レンジを倍率補正
 * @attention
 */
void calc6(uint8_t range)
{
	R = Y1_ / ADsp[(uint8_t)range];
}

/**
 * @brief 			センサー温度特性補正前DOmV値ADを計算する
 * @param[in]		range	レンジ番号（0:低 1:中 2:高）
 * @detail			ADzr[r],ADsp[r]は各レンジの補正係数
 * @attention		中、高レンジで計算上ゼロにならないため、
 * 					ADzr[1],ADzr[2]以下にしない判定追加
 */
void calc7(uint8_t range)
{
//	AD = (R - ADzr[(uint8_t)range]) * ADH[(uint8_t)range];

//	アンプ調整値のズレ対策で、中、高レンジ時の式修正	(2020/02/25 設計部　三浦)
	if( range == LOW_RANGE ){
		AD = (R - ADzr[(uint8_t)range]) * ADH[(uint8_t)range];
	}
	else{
		AD = ADH[(uint8_t)range] * R + ADzr[(uint8_t)range];
	}

	//低レンジ切替直後のゼロ点処理
	if ((range == LOW_RANGE) && (AD < (ADzr[LOW_RANGE] / ADsp[LOW_RANGE])))
	{
		if (first_LOW_timer)
		{
			AD = ADzr[LOW_RANGE] / ADsp[LOW_RANGE];
			MAve_Reset(((AD / ADH[LOW_RANGE]) + ADzr[LOW_RANGE]) * ADsp[LOW_RANGE]);
		}
	}
	//中、高レンジの場合のゼロ点処理
	//if ((range == MID_RANGE) && (AD < (ADzr[MID_RANGE] / ADsp[MID_RANGE])))
	if ((range != LOW_RANGE) && (AD < (ADzr[LOW_RANGE] / ADsp[LOW_RANGE])))
	{
		AD = ADzr[LOW_RANGE] / ADsp[LOW_RANGE];
		MAve_Reset(((AD / ADH[LOW_RANGE]) + ADzr[LOW_RANGE]) * ADsp[LOW_RANGE]);
	}
}


/**
 * @brief 			センサー温度特性補正後DOmV値FHを計算する
 * @detail			FtempH[x]はワグニット毎の温度特性式係数
 * @attention		現状はWA-TXのものを使用
 */
void calc8(void)
{
	FH = (WAG_FtempH[6] * WTemp * WTemp * WTemp * WTemp * WTemp * WTemp)
			+ (WAG_FtempH[5] * WTemp * WTemp * WTemp * WTemp * WTemp)
			+ (WAG_FtempH[4] * WTemp * WTemp * WTemp * WTemp)
			+ (WAG_FtempH[3] * WTemp * WTemp * WTemp)
			+ (WAG_FtempH[2] * WTemp * WTemp)
			+ (WAG_FtempH[1] * WTemp)
			+ WAG_FtempH[0];
}

/**
 * @brief 			センサー20℃補正後DOmV値AD20を計算する
 * @detail			ADはcalc7,FHはcalc8で計算
 * @attention
 */
void calc9(void)
{
	AD20 = AD / FH;
}

/**
 * @brief 			飽和溶存酸素量FDOHを計算する
 * @detail
 * @attention		JIS2016,JIS2013で計算式が異なる
 */
void calc10(void)
{
	float T;
	float tbl_d1, tbl_d2;
	int tbl_p, tbl_t;
	char tbl_cl;

	T = WTemp;

	if (select_JIS == 0)
	{
		// 旧JIS(JIS K0102-2013)設定時は今まで通りの計算を行う
		FDOH = (-6.6018e-08 * T * T * T * T) + (-6.06148e-05 * T * T * T) + (7.63103e-03 * T * T) + (-0.393737 * T) + 14.1608;
	}
	else
	{
		//新JIS(JIS K0102-2016)の場合の計算を追加
		if (tansui_sw_flag)
		{ //海水設定時
			tbl_cl = 1;
		}
		else
		{ //淡水設定時
			tbl_cl = 0;
		}

		if (T <= -10.0)
		{ //テーブル範囲外は境界値で計算させる
			FDOH = (float)(SatrTable[0][(uint8_t)tbl_cl]) / 100.0;
		}
		else if (T >= 55.0)
		{
			FDOH = (float)(SatrTable[55][(uint8_t)tbl_cl]) / 100.0;
		}
		else
		{
			tbl_t = (int)(T * 10.0); //水温下1桁までを整数値化
			tbl_p = tbl_t / 10;		 //水温の上桁のみ抽出
			if (tbl_t < 0)
			{ //水温マイナス時の計算
				tbl_d1 = (float)(SatrTable[(uint8_t)(tbl_p + 9)][(uint8_t)tbl_cl]) / 100.0;
				tbl_d2 = (float)(SatrTable[(uint8_t)(tbl_p + 10)][(uint8_t)tbl_cl]) / 100.0;
				FDOH = tbl_d1 - (tbl_d1 - tbl_d2) * ((((float)(tbl_t) / 10.0) - (float)(tbl_p - 1)) / 1.0);
			}
			else
			{ //水温時の計算
				tbl_d1 = (float)(SatrTable[(uint8_t)(tbl_p + 10)][(uint8_t)tbl_cl]) / 100.0;
				tbl_d2 = (float)(SatrTable[(uint8_t)(tbl_p + 11)][(uint8_t)tbl_cl]) / 100.0;
				FDOH = tbl_d1 - (tbl_d1 - tbl_d2) * ((((float)(tbl_t) / 10.0) - (float)(tbl_p)) / 1.0);
			}
		}
	}
}

/**
 * @brief 			塩化物イオン補正値FCLを計算する
 * @detail			塩化物イオン濃度 海水はCL = 19000
 * @attention		JIS2013以前用（JIS2013はFDOH計算時に行っている）
 */
void calc11(void)
{
	FCL = ((4.2561e-09 * WTemp * WTemp * WTemp) + (6.6295e-06 * WTemp * WTemp) + (-4.62284e-04 * WTemp) + 1.52867e-02) * CL / 100;
}

/**
 * @brief 			塩化物イオン補正後飽和溶存酸素量DOHを計算する
 * @detail
 * @attention		JIS2013以前用（JIS2013はFDOH計算時に行っている）
 */
void calc12(void) //
{
	DOH = FDOH - FCL;
}

/**
 * @brief 			溶存酸素量DO(mg/L)を計算する
 * @detail			WA_Fはセンサー別流速補正係数WAG_Fflow[x]
 * 					WA_Tはセンサー別空気校正係数WAG_FairH[x]
 * @attention		現状WA_F,WA_TはID-150のものを使用
 */
void calc13(void)
{

	//流速補正値計算
	WA_F = (WAG_Fflow[3] * WTemp * WTemp * WTemp)
			+ (WAG_Fflow[2] * WTemp * WTemp)
			+ (WAG_Fflow[1] * WTemp)
			+ WAG_Fflow[0];

	//検査課恒温水槽で空気校正後に飽和水測定値が理論値と一致しない
	//問題の処置として、ズレ0.2mg/L分を内部計算で補正する
	//2020/01/17 柵木
	WA_F = WA_F * 1.022;	//9.1 - 8.9 = 0.2mg/L 0.2 / 9.1 = 0.02197 = +2.2%


	//空気補正値計算
	WA_T = (WAG_FairH[3] * WTemp * WTemp * WTemp)
			+ (WAG_FairH[2] * WTemp * WTemp)
			+ (WAG_FairH[1] * WTemp)
			+ WAG_FairH[0];
	//DO値計算
	DO = ((AD20 - DOzr) / (DOsp - DOzr)) * WA_F * WA_T * DOH;
}

/**
 * @brief 			DO値表示分解能Rtを計算する
 * @detail			calc14～17でRtを計算
 * 					表示分解能Rtを計算し、それをレンジ切替判断に使用
 * @attention		現状ID-150と係数が合わないため未使用
 */
void calc14(void)
{
	CNT = DO * 10;
}
void calc15(void)
{
	CNT = DO * 100;
}
void calc16(void)
{
	DIG = Y1_ / 2.44;
}
void calc17(void)
{
	Rt = DIG / CNT;
}

/**
 * @brief 			空気飽和率AIR_PERを計算する
 * @detail
 * @attention
 */
void calc18(void)
{
	if (DO > 20.0)
	{
		AIR_PER = (20.0 / FDOH) * 100.0;

	}
	else
	{
		AIR_PER = (DO / FDOH) * 100.0;
	}
}

/**
 * @brief 			未使用計算式
 * @detail
 * @attention		何もしない
 */
void calc19(void)
{
	//何もしない
}

/**
 * @brief 			レンジ毎のADzr[r]を計算する
 * @param[in]		range	レンジ番号（0:低 1:中 2:高）
 * @detail
 * @attention		レンジ毎のゼロ時Rを代入
 */
void calc20(uint8_t range)
{
	switch(range){
	case 0:
		ADzr[0] = R;
		break;
	case 1:
//		ADzr[1] = R - AD_LOW;

//	アンプ調整値のズレ対策で、式修正	(2020/02/25 設計部　三浦)
		ADH[1] = (AD_MID - AD_LOW) / (AD_MID - R) ;
		ADzr[1] = AD_LOW - (R * ADH[1]) ;
		break;
	case 2:
//		ADzr[2] = R - AD_MID;

//	アンプ調整値のズレ対策で、式修正	(2020/02/25 設計部　三浦)
		ADH[2] = (AD_HI - AD_MID) / (AD_HI - R ) ;
		ADzr[2] = AD_MID - (R * ADH[2]) ;
		break;
	}
}

/**
 * @brief 			レンジ毎のADzr[r]を計算する
 * @param[in]		range	レンジ番号（0:低 1:中 2:高）
 * @detail
 * @attention		レンジ毎のゼロ時Rを代入
 * 					現状未使用
 */
void calc21(uint8_t range)
{
	ADzr[(uint8_t)range] = R;
}

/**
 * @brief 			レンジ毎のADzr[r]を計算する
 * @param[in]		range	レンジ番号（0:低 1:中 2:高）
 * @detail
 * @attention		レンジ毎のゼロ時Rを代入
 * 					現状未使用
 */
void calc22(uint8_t range)
{
	ADzr[(uint8_t)range] = R;
}

/**
 * @brief 			低レンジ0時のADH[0]を計算する
 * @detail
 * @attention		基準入力AD_LOW = 411.8644mV
 * 					疑似入力0.135μA
 *					※自動基板調整で疑似入力0.15uA(457.6240mV)→0.135uA(411.8644mV)に変更、コメント修正
 *						(2020/02/25 設計部　三浦)
 */
void calc23(void)
{
	ADH[0] = AD_LOW / (R - ADzr[0]);
	ADzr[0] = ADzr[0] * ADH[0] ;		//	アンプ調整値のズレ対策で、式追加	(2020/02/25 設計部　三浦)
}

/**
 * @brief 			中レンジ1時のADH[1]を計算する
 * @detail
 * @attention		基準入力AD_MID = 3050.8475mV
 * 					疑似入力1.0μA
 */
void calc24(void)
{
//	ADH[1] = AD_MID / (R - ADzr[1]);

//	アンプ調整値のズレ対策で、式修正	(2020/02/25 設計部　三浦)
	float	RL ;

	RL = (AD_LOW - ADzr[1]) / ADH[1] ;
	ADH[1] = (AD_MID - AD_LOW) / (R - RL) ;
	ADzr[1] = AD_LOW - (RL * ADH[1]) ;
}

/**
 * @brief 			高レンジ2時のADH[2]を計算する
 * @detail
 * @attention		基準入力AD_HI = 19832.9295mV
 * 					疑似入力6.5μA
 */
void calc25(void)
{
//	ADH[2] = AD_HI / (R - ADzr[2]);

//	アンプ調整値のズレ対策で、式修正	(2020/02/25 設計部　三浦)
	float	RL ;

	RL = (AD_MID - ADzr[2]) / ADH[2] ;
	ADH[2] = (AD_HI - AD_MID) / (R - RL) ;
	ADzr[2] = AD_MID - (RL * ADH[2]) ;
}


/**
 * @brief 			低温側調整係数TLspを計算する
 * @detail
 * @attention
 */
void calc27(void)
{
	TLsp = (SETmid - SETzr) / (TPmid - TPzr);
}

/**
 * @brief 			低温側調整係数TLzrを計算する
 * @detail
 * @attention
 */
void calc28(void)
{
	TLzr = SETzr - (TLsp * TPzr);
}

/**
 * @brief 			高温側調整係数THspを計算する
 * @detail
 * @attention
 */
void calc29(void)
{
	THsp = (SETsp - SETmid) / (TPsp - TPmid);
}

/**
 * @brief 			高温側調整係数THzrを計算する
 * @detail
 * @attention
 */
void calc30(void)
{
	THzr = SETmid - (THsp * TPmid);
}

/**
 * @brief 			35℃以上高温側調整係数T0spを計算する
 * @detail
 * @attention
 */
void calc31(void)
{
	float SimHenka35, SimHenka50;
	float Sim50;

	SimHenka35 = TPsp / 39.24344;
	SimHenka50 = ((SimHenka35 - 1.0) * 0.831441531) + 1.0;
	Sim50 = 55.05571 * SimHenka50;

	TOsp = (50.0 - SETsp) / (Sim50 - TPsp);
}

/**
 * @brief 			35℃以上高温側調整係数T0zrを計算する
 * @detail
 * @attention
 */
void calc32(void)
{
	TOzr = SETsp - (TOsp * TPsp);
}
