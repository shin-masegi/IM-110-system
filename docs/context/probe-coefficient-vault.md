# プローブ係数 vault 化仕様（旧 auto-memory: project）

> 出所: Claude auto-memory `project_probe_coefficient_vault.md`（type: project / originSession 6598803f…）を 2026-07-22 にリポへ移設。
> メモリ側は本ファイルへのポインタのみ。編集はここを真実源とする。
> **重要な注記（2026-07-22）**: 本ファイルが記す vault（`probe_vault_t`, cf[7] 系, Page63/VAULT_AD）は**旧モデル**。
> その後の測定モデル再設計で係数レイアウトは `mlss-calc-reference.md §12`（512B 統合ストア `probe_store_t`）へ移行する方針に変わった。
> 旧 vault と §12 ストアは**別モデルで非互換**、かつ両者が同じ Page63 を使うため衝突する（`handoff.md` §4 参照）。
> 「係数を §12 統合ストアへ繋ぎ替える」= protocol-rs232c.md §3.2.6 改訂 + プローブ + 本体 の3点同時移行が未完。
> 本ファイルは旧 vault の設計経緯として保存する。現行の目標レイアウトは §12 を見ること。

修理で本体基板のみ交換・プローブ付替をしても測定値が正しく出るよう、**プローブ性能に関わる係数をプローブ flash に持たせ真実源とする**仕様変更(設計合意済み・実装未着手)。

**背景の構造事実**: プローブは raw 信号デバイスで、濃度換算(Calc_MLSS 等)は本体側でやる。プローブは係数を自分では使わず「金庫(vault)」として不揮発保存し、本体が接続時に読んで使う。

**分類(確定)**:
- プローブflash(真実源): ADC_Zero/Span/LED_Out/Span_S・Probe ID(現状維持) ＋ 校正係数 ZR/FABS_SPAN/SP_A/SP_B/SP_C/1A/1B(MLSS/SS 相関式No.21-30 各10式＋TR 1式) ＋ 校正点濃度 Cal_SetVal_1/2・3点中間(MLSS/SS/TR)
- 本体EEPROM: 相関式選択(MLSS_MODE/SS_MODE)・測定モード(Meas_Mode)・界面閾値(Interface_Threshold)・本体情報/エラー情報・測定履歴 ＋ 上記プローブ係数のキャッシュ
- FW固定: 相関式No.1-20(校正不可ハードコード、保存対象外)

**真実源=プローブ／本体=キャッシュ**。転送量は**A:全相関式ぶん完全転送**を採用(接続時1回 ~147float/~3KB。Bの選択中式のみだと他式切替で校正消失する穴があるため却下)。

**フロー**: 接続/起動時=本体がプローブから全係数読込→RAM反映→本体EEPROMにキャッシュ。校正確定時=本体が該当式をプローブへ書戻し＋キャッシュ更新(先日の後校正 write_param_calibration にプローブ書込を追加)。プローブ無応答/旧FW時=本体キャッシュにフォールバック。既存校正済み機は初回に本体EEPROM→プローブ一括コピーで移行。

**How to apply:** 実装は CLAUDE.md §3.1/§3.2 に従い、まず docs/protocol-rs232c.md に係数ブロック転送コマンド(読:本体←プローブ / 書:本体→プローブ、RPP拡張。個別SET147個は不可)を規定→プローブFW(vault領域をflashに新設)→本体FW(接続時読込・校正時書戻し)。本体EEPROMの現行レイアウトは page19-59が校正係数(Eeprom.h/Eeprom.c read/write_param_calibration)。ハード根拠は `hardware-truth-source.md`、通信経路は `probe-fup-bridge.md` と同じ USART2↔本体。

**進捗(2026-07-09):** protocol §3.2.6 に RCF/WCF/WCS/WCFC 規定済(未push)。**プローブ側 実装済(Ver.0.14, ビルドOK)**: flash Page63(VAULT_AD=0x0801F800) に probe_vault_t(624B: magic/ver/mlss[10][7]/ss[10][7]/tr[7]/setval[3][2]/checksum)、vault_load()起動時読込、RCF/WCF/WCS/WCFC ハンドラ、float は %.9g で往復。コマンド名: 0=MLSS/1=SS/2=TR、eq=21-30(TRは0)。

**vault ver2 化(2026-07-13)**: 調整機能①で probe_vault_t を ver2 構造体化(624B→1408B)。詳細は `adjust-mode.md`。ver1 flash は起動時 ver2 へ自動移行。

**本体側 実装済(2026-07-09, ビルドOK, 未push)**: RCF読込=起動時(main.c ループ先頭 vault_synced 一度きり)＋相関式切替時(Normal.c C_S_CORR_3)。Probe_ReadVault()=MS停止→RCF→行毎パース→EEPROM差分同期(read/write_calibration_page_raw でグローバル非汚染・memcmp差分のみ書込)→write_param_common→選択式 read_param_calibration→MS再開。校正確定(Normal.c ADCAL_3/MCAL_3)で write_param_calibration(0,MLSS_MODE)＋Probe_WriteBackCalibration(WCF/WCS/WCFC)。マイグレーション=自動(起動時 RCF が 1=無効→Probe_MigrateToVault で本体EEPROM全係数push)＋手動(本体コマンド MIGV, LinkSerial.c)。vault通信一式は IM_110.c(main)、wire eq=No.(内部corr_idx+1)、TRはeq0。**残: 実機確認のみ(往復整合・差分同期・マイグレーション・校正書戻し)。SS/透視度の後校正書戻しは後校正未実装のため対象外。**
