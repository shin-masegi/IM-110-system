# 調整機能（ADBOAD）設計・進捗ログ（旧 auto-memory: project）

> 出所: Claude auto-memory `project_adjust_mode.md`（type: project / originSession 6598803f…）を 2026-07-22 にリポへ移設。
> メモリ側は本ファイルへのポインタのみ。編集はここを真実源とする。
> **設計の一次真実源は `docs/adjust-mode-design.md`**（本ファイルは進捗スナップショット）。
> **注記(2026-07-22)**: 本ログ中の 係数ストア(cf[7]/modecf[7]/kiza 系, Page63) は旧モデル。係数保存の現行目標は
> `mlss-calc-reference.md §12` の 512B 統合ストア。移行未完の状況は `handoff.md` §4 参照。

プローブの補正式作成・校正（現状 Excel `docs/specs/IM補正式作成手順.xlsx` の手作業）を、本体の**基板調整モード ADBOAD** 上で実行できるようにする。作った係数は係数ストア（Page63。保存レイアウトは `mlss-calc-reference.md §12` 統合ストア）へ書く。

**★次セッション起点(2026-07-14 合意)**: 補正式①②③・係数ストア ver2・算出コア・MLSS/SS/透視度 校正フローは実装+push 済。次は **A(シリアル調整インターフェース=LinkSerial.c にコマンド追加)を実装→A で実機検証→B(ADBOAD LCD 6画面)を実装** の順。A を先にする理由=ADBOAD 入口が無効化中(main.c~264 PC4_nTEST 誤起動対策でコメントアウト)＋既存ADBOADは ID-200T DOレガシー、一方 LinkSerial は PC 直結コマンド基盤あり算出コア+係数ストア書込を直接叩ける。**A のコマンド設計スケッチと全体の完了/残は `docs/adjust-mode-design.md` 冒頭「★次セッション引き継ぎ」に記載**。検証順=FUP→係数ストア→調整/校正。

**A シリアル調整インターフェース 実装済(2026-07-15, 未push, 本体ビルドOK)**: LinkSerial.c `Handle_Adjust_CMD()`(先頭'A'委譲・未該当は既存AD/AAS/ATPへフォールスルー)+IM_110.c `Adj_*` オーケストレーション層。コマンド: AMV(生値表示) / ALD,<duty>(LED揮発) / ALDA,<start>(1750±30自動+WPP保存,MD一発測定同期反復) / AZR(遮光受光ダーク→CZ) / ATC,<temp>+ATCF(温度3点→CT/CZ) / AMC,<mgL>+AMCF,<次数>(多項式CM,MLSS/SS) / AMCP(累乗CM,透視度) / AKZ,<mgL>(機差CK) / AWC(WCFC確定) / AMR(リセット) / AST(状態)。捕捉=ADC_mV_ave(MS稼働前提)、書込=staging更新→Apply_Coef_To_Live→WCM/WCK/WCZ/WCT(MS一時停止)。**プローブFW変更なし・protocol変更なし**(新cmdは本体↔PC/USART1、SP/MD/WPP/WCxは既存)。**設計精査修正: AZR は受光ダーク adzr[0]のみ、adzr[1]=refZR(20℃清水Ref)は ATCF が設定**(E'再定義に整合)。詳細=adjust-mode-design.md「A:シリアル調整インターフェース」。**次=A で実機検証(FUP→係数ストア→調整)→ B(ADBOAD LCD)**。

**回帰安定化+R² 実装済(2026-07-16, 未push, ホスト検証済)**: adj_fit_poly に x スケーリング(x'=x/max|x|→a_k=b_k/s^k で厳密逆変換)追加=4次悪条件緩和。adj_r2_poly/adj_r2_power(R²)追加、AMCF/AMCP が `OK,R2:0.999` 併記、AMD で捕捉点(ABS,基準器)ダンプ。ホスト検証: 4次厳密復元誤差1e-6/R²=1、大x相対7e-8、2点1次厳密、ノイズR²<1。design §5 F の x中心化・R²残は解消。**B(ADBOAD LCD 6画面+PC4_nTEST入口再有効化)は実機セッションで配線**(画面/ボタン/誤起動対策は実機依存、計算パイプラインは A で全駆動可ゆえデバッグ先行可能)。**ユーザ計画: 実機確認時に Claude がデバッガで計算式をデバッグ(初試み)**。

**実機デバッグ ハーネス 準備済(2026-07-16, IM-110-system/tools/adjust-debug/, 未push)**: ユーザ合意=Claude が OpenOCD/GDB とシリアル(A)両方を駆動、Mac から USB-シリアルで本体届く。`openocd_l452.cfg`(ST-Link+stm32l4x)/`adj.gdb`(マクロ adcs/mlssv/ssv/trv/caps + armfit=fit/adjustブレーク + stepmlss=ホットパス1サイクル)/`adjctl.py`(依存なしtermios、115200 8N1 LF、A cmd送受、-i対話)/`README.md`(手順1-9→Aコマンド列)。接続=ST-Link→CN1,USBシリアル→CN2(USART1 115200 8N1),プローブ→CN4。ツール類はarm-none-eabi-gdb/openocd/st-util/st-flash がMac導入済(pyserialは無=termiosで回避)。**シリアル完全駆動用に AMODE,<0/1/2>(モード切替)/AEQ,<21-30>(MLSS相関式選択) を追加**(ボタン不要化)。次セッションで実機接続→openocd起動→gdb attach→adjctl で手順実行しながらデバッグ。

**設計の真実源 = `docs/adjust-mode-design.md`**（口頭質問を1トピックずつ相談→決定を積む作業ログ）。元資料 xlsx 2つも docs/specs に commit 済(push 済 f715940)。

**決定済み(2026-07-13, A〜G):**
- A 入口: 現状のまま。
- B メニュー: 残す=Ver表示/EEPROMテスト/電池電圧/時刻設定。削除→新規置換= A/DCゼロ・スパン・レンジ・水温調整系(LOWRANGE0等/WAT05C等/CALTEMP)。既存コードは置換先確定後に撤去。
- C/D ゼロ/スパン: 空気中スパン正規化は**削除**(吸光度=log比でキャンセル、清水校正が吸収)。「清水≒1750mV」目標はLED duty側へ。ゼロ(暗時ADZR)は残すが**本体側で一本化(後で)**、相関式非依存の単一係数。MLSS/SS/透視度 共通。プローブADC_Zeroと本体MLSS_ADZRの二重を解消。
- E Ref温度補正: Refを温度プロキシに受光を**加算補正** `受光_corr=受光+a·(Ref_20−Ref)`。フィット=**2次式3点通過**(既存MLSS_Calib_Span流用、実機で誤差なら1次式2本へ)。RefADZR取得要。係数はper-mode単一(MLSS/SS 2セット)。透視度=SS式流用(同一光学ch+PGA x2、Ref共通。加算補正ゆえゲイン前信号に補正→x2)。Calc_MLSS/SSの吸光度前に補正段新設。3点厳密解=機上可。
- F 相関式回帰: **機上で最小二乗**。4次式=正規方程式5×5ガウス消去、累乗式=log線形化、2点=厳密解。単精度FPUの悪条件対策(x中心化スケーリング+double解+残差R²)。Mode_CF(`MLSS/SS/TR_Mode_CF[30][7]`)は現状FWハードコードで未保存→per-eqで係数ストアに追加保存要。
- G 機差補正: 工場機差補正(ADBOAD,1点比例=基準器mg/L÷1次演算後)と現場スパン校正(既存2点/3点 SP_A/B/C)は**別係数で分離**、記録も別。パイプライン=吸光度→Mode_CF→FABSS→機差補正(工場)→スパン校正(現場)→最終。

**実装進捗:**
- **① 係数ストア ver2 実装済(2026-07-13, 両側ビルドOK, 未push・実機未確認)**。境界=「保存層のみ、測定パイプライン非配線」。
  - プローブFW(Ver.0.15): 旧係数構造 を ver2 構造体化(eq_coef_t{cf[7],modecf[7],kiza}×MLSS/SS No.21-30・TR + mode_coef_t{setval[2],adzr[2],tempc[7]}×3、1408B/176dword/Page63)。RCF に CM/CK(per-eq)・CZ/CT(per-mode,MLSS/SSのみ)追加、CFV ver=2。WCM/WCK/WCZ/WCT ハンドラ+ディスパッチ。`coef_load` に ver1→2 移行(cf/setval引継・新0、次WCFCでflashもver2化)。
  - 本体FW(FW_VER1据置=受信のみ): `Probe_ReadCoef` に CM/CK/CZ/CT パース追加→staging グローバル `MLSS/SS_Coef_ModeCF[10][7]`,`*_Coef_Kiza[10]`/`TR_Coef_Kiza`,`*_Coef_ADZR[2]`,`*_Coef_TempC[7]`(IM_110.c/.h)。**安全策: 新品/移行直後は modecf=0 なのでハードコード相関表 MLSS_Mode_CF[] には配線しない**(0上書きで測定破壊回避)。本体→プローブ ver2書戻し(WCM等送信)は未実装。
  - protocol §3.2.6.1 の「未実装」→実装状況注記に更新。
- **② 実装済(2026-07-13, push済 da8084d)**: IM_110.c パイプラインに 係数ストア ver2 配線。`apply_temp_corr`(Ref加算温度補正、TRはSS参照gain2.0)を Calc_MLSS/SS/Transparency の吸光度前に挿入、`sel_kiza`(機差補正)を FABSS→SP 間に、`Apply_Coef_To_Live`(Mode_CF/ADZR を 係数ストア由来へ、modecf全0=未校正はハードコード保持ガード)を Probe_ReadCoef 末尾で呼ぶ。**全係数 未校正(0)なら恒等=現行測定 非破壊**。本体→プローブ書戻し送信 `Probe_WriteCoefModeCF/Kiza/ADZR/TempC`(WCM/WCK/WCZ/WCT)追加(未使用ゆえgc-sections除去、③で呼ぶと組込)。
- **③ 係数算出コア 実装済(同上, ホスト単体検証OK)**: IM_110.c に `adj_fit_poly`(最小二乗多項式,double部分ピボットGauss,最大6次)、`adj_fit_power`(累乗log線形化)、`adj_calc_tempc`(2次3点通過,gausejordan_3pt流用)、`adj_calc_kiza`(1点比例)。
- **② 残(検証フェーズ)**: ver2フィールドの EEPROMキャッシュ(切断時フォールバック)、後校正パスへの温度補正適用。
- **③ 残(検証フェーズで実機と)**: ADBOAD 調整画面/メニュー配線(Adjust.c 1921行に新画面群、キャプチャ→算出コア→係数ストア書込・生mV表示)。x中心化・R²チェック。

**SS/透視度 校正フロー配線 完了(2026-07-14, 未push, ビルドOK)**: 校正実行(Normal.c ZCAL_3/ADCAL_3/MCAL_3)を MLSS ハードコードから Meas_Mode ディスパッチ(`Calib_Zero/Span_Post/StoreHold*/UpdateMidDefault/SaveWriteback_Current`, IM_110.c)へ置換。記録は his_get_raw_mv_x10() が現モードch保存済でモード対応。SS パリティ=`SS_FABSS_from_raw_mv`+`SS_Calib_Span_Post`追加・Calc_SS ヘルパ化。**SS/TR は相関式No.概念廃止→内部 No.21(EEP_CF_BASE_IDX=20)固定**(INI_SS/TR_MODE=20、Change_Meas_Mode で強制+係数ロード+Apply_Coef_To_Live)。M_MENU の相関式選択(項目1)を SS/TR でスキップ(→S_CORR画面に入らない)。残cosmetic=メニュー一覧に項目1テキスト残存。→ MLSS/SS/透視度 とも ゼロ/2点/3点後スパン校正→EEPROM→係数ストア書戻し が動作。実機未確認。

**手順2 LED duty 調整 決定+算出コア済(2026-07-14, 未push)**: 空中で Ch0-3(MCP3424 [0]MLSS受光/[1]MLSS_Ref/[2]SS受光/[3]SS_Ref、透視度[4]除外)の**最大出力を1750mV(±30mV)ちょうど**に。比例1発+収束反復2〜3回、`adj_led_duty_step()`(=duty×1750/max)実装済。**プローブFW変更不要**: 本体ADBOADが既存 `SP,<duty>`(LED_Out[0]更新+PWM,揮発)+MS(ADC_mV[0..3]読)+`WPP`(LED_Out[]をPARAM_AD/Page62 flash保存)で駆動。duty分解能=CCR=1240×duty。UI本体は検証フェーズで配線。要確認: 初期duty コード0.458 vs 実機0.36。詳細 adjust-mode-design.md 手順2。
  - ※注(2026-07-21以降): 空中ターゲットは **1700mV / span傾き正規化**へ確定変更（handoff §2/§7.2）。上記 1750 記述は当時のもの。
- **検証順(ユーザ計画)**: FUP(先日実装のプローブUART書換)→係数ストア(①)→adjust の順に実機検証・修正。ADBOAD UI はこの adjust ステップ到達時に実機を見ながら配線する。

**補正式修正(2026-07-14〜, 真実源 IM補正式作成手順.xlsx 照合で順次):**
- **①MLSS/SS共通式 完了(未push)**: 現行 `log10(ZR/(Y0−ADZR))×FABS_SPAN` は真実源と構造相違。新式 `I(0)=ZR+(ref−refZR)·(B+(ref−refZR)·B2); ABS=log10(I(0)/(Y0−ADZR))`。Ref温度補正を分子I(0)に内蔵、**FABS_SPAN廃止**(器差はkizaへ)。②の受光加算 apply_temp_corr は MLSS/SS から撤去。係数ストア意味再定義(レイアウト不変): tempc[0]=B傾き/tempc[1]=B2/adzr[1]=refZR。MLSS_FABSS_from_raw_mv に ref 引数追加、adj_calc_tempc は B(最小二乗)+refZR。ホスト検証 清水3点 ABS≈0。詳細 adjust-mode-design.md §5 E'。移行: 既存器差校正機は kiza 再校正要。
- **②MLSS/SS 1次演算式=1次直線化 完了(未push)**: MLSS_Mode_CF/SS_Mode_CF を全て1次(C1のみ)。No.1-20 等間隔グラデ(No.1=21315=従来No.1、No.11=42494.4572=従来黒No.5一致、No.20=61555.97≈1.45×、step≈2117.95)。No.21-30(校正可)は初期=21315、ADBOAD校正で xlsx値(黒14556.63/白4次)へ上書き。旧No.4,5の4次撤去。TR表は累乗フォールバック据え置き。
- **③透視度 mV-cm=累乗式 完了(未push)**: Calc_Transparency を `cm=a·ABS^b`(a=TR_Coef_ModeCF[0],b=[1])へ。ABS=log10(I(0)/(Y0−ADZR))、I(0)=TR_ZR+(ref−refZR)·B(ref=ADC_mV_ave[3]、B/refZR 当面SS流用≈1.366)。ABS≤0は1e-4クランプ(NaN回避)、a==0は旧多項式フォールバック。apply_temp_corr 撤去。ホスト検証 xlsx cm を±8〜15%(累乗回帰残差、後段1点校正+kizaで補正)。**xlsx: 透視度ZR=1777.17, B=1.365945, 累乗 a=0.2409198 b=−0.8627988**。
- **③透視度 digit-mV 変換=不要で確定(2026-07-14)**: 専用 digit→mV 式は無し。A/DC の mV をそのまま mV→cm(累乗)へ。現行 Calc_Transparency が既にこれ。**論点3 完結・コード変更不要**。
- Log は全て log10 確定。補正式①②③(mV-cm)は本体側のみ・ビルドOK。

**相関式の個数(2026-07-14 確定)**: MLSS=No.1-20(FW固定1次)+No.21-30(校正可10個)。**SS/透視度=3点校正できる相関式が1つだけ**(=MLSS No.21相当、モード選択なし)。「3点校正」=スパン校正の段(SP_A/B/C 2点/3点、既存 *_Calib_Span)であって相関式自体を3点で作る意ではない。**SS=案A**(係数ストア ss[10]のまま No.21 slotのみ使用、余剰9は当面放置、両側変更なし)。**透視度 校正プリミティブを MLSS 同等に整備済(未push)**: `TR_FABSS_from_raw_mv`(累乗raw→FABSS再計算)+`TR_Calib_Span_Post`(後スパン校正)追加、Calc_Transparency をヘルパ化。**残=Normal.c の透視度/SS 校正フロー配線**(TR_1A/1B ライブ捕捉・後校正呼出・EEPROM保存)=校正画面作業(MLSSは Ver.0.23 で実装済、SS/透視度は未)。

**設計上の残り(全て決定済・下記は実装時の確認事項):**
- H 係数ストア保存レイアウト **決定済(adjust-mode-design.md §5 H)**。構造体化+ver2: eq_coef_t{cf[7],modecf[7],kiza} × mlss[10]/ss[10]/tr + mode_coef_t{setval[2],adzr[2],tempc[7]} × mode[3] + header/checksum ≈1408B(Page63内,8境界padding)。tempc[7]=1EEPROMページ最大枠。**TRのtempc/adzrはSS参照(mode[2]は未使用,setvalのみ使用)**。protocol §3.2.6 ver2拡張: 読 CM=Mode_CF/CK=機差/CZ=ADZR/CT=温度補正、書 WCM/WCK/WCZ/WCT (CM/CK は per-eq、CZ/CT は per-mode)。ver1→2移行は本体検出で既存cf/setval引継・新フィールド初期値。
  - ※注(2026-07-22): この H レイアウトは旧 係数ストア モデル。係数保存の現行目標は `mlss-calc-reference.md §12` の 512B 統合ストアへ寄せる方針に更新済み（移行未完、handoff §4）。
- I 調整中の生mV(受光/Ref)表示・記録方法。
- G派生: 機差補正 per-eq/per-mode 確定、式は1点比例で確定か。
