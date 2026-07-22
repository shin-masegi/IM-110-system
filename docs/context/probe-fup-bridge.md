# プローブ UART FW 更新（FUP / 8E1 パススルー）方針（旧 auto-memory: project）

> 出所: Claude auto-memory `project_probe_fup_bridge.md`（type: project / originSession 6598803f…）を 2026-07-22 にリポへ移設。
> メモリ側は本ファイルへのポインタのみ。編集はここを真実源とする。
> 手順の真実源は `docs/fup-procedure.md`、protocol は `docs/protocol-rs232c.md` §3.2.5。

IM-110_Probe (STM32G070) はケース内に接着固定され SWD(J4) が触れなくなるため、UART経由の FW 書き換え経路 (方式A) を用意する。設計合意:

- 経路: PC(USB-TTL) → 本体CN2(USART1) → **本体=8E1透過ブリッジ** → LPUART1 → ケーブル → プローブUSART2 → ROMブートローダー。本体は焼き直し可能(CN1のSWD生存)、プローブのみ埋め込み。
- **FUP コマンド**(プローブFWに自作追加): 受信で RAMマジック+NVIC_SystemReset→起動直後にシステムメモリ(0x1FFF0000)へジャンプさせ ROMブートローダー(AN3155)を待受状態にする。BOOT0=PA14はSWCLK共用+R26プルアップで外部制御不可のため、このソフトジャンプが唯一の入口。
- PC側ツール: STM32CubeProgrammer / stm32flash が .hex を AN3155 で書込。パリティは通常8N1→ブートローダー中は **8E1**。
- **アンブリック**: 更新はベクタ先頭ページ消去から始める→失敗しても empty check で次回電源ONにプローブが自動でブートローダーに入る。ブリッジFWは FUP 無応答(=空Flash)ならそのまま8E1パススルーへ進む堅牢化を入れる。
- 抜け方: **電源OFF (案D)**。社内の製造・修理専用機能でランタイム解除ロジックは持たない。

**Why:** 本番FWに書込パススルー経路を残すと誤爆・品質/認証リスク。かつ開発中は本体FW単体で回したい。

**How to apply:** 機能は最初から `#ifdef ENABLE_PROBE_FUP_BRIDGE` で囲う。現状=本体FWでフラグON運用。製品リリースビルドはフラグOFF(自動除外・消し忘れ防止)、治具FWはフラグONでビルド。実装順は CLAUDE.md §3.1/§3.2 に従い protocol-rs232c.md に FUP 規定→プローブFW→本体ブリッジ。ハード根拠は `hardware-truth-source.md`。

**進捗(2026-07-09):** ①②とも実装済み(未push)。protocol-rs232c.md §3.2.5 に FUP 規定。
- プローブ FW Ver.0.13: FUP,45063 ハンドラ(uart_Enter_Bootloader / .noinit フラグ g_fup_boot_flag / main冒頭 JumpToSystemBootloader / リンカ .noinit セクション)。常時搭載。
  - ※注: その後 Ver.0.17 で移行マーカを `.noinit` SRAM から **TAMP バックアップレジスタ** へ変更（コールド起動誤爆対策、protocol-rs232c.md:290）。
- 本体 FW: ProbeFupBridge_Run() を main.c に追加、`#ifdef ENABLE_PROBE_FUP_BRIDGE` で囲む。Makefile に `make FUP_BRIDGE=1`(-DENABLE_PROBE_FUP_BRIDGE)追加。既定ビルドは text 変化なし(=製品FW不変)、治具ビルドは gc-sections でアプリ除去されブリッジ専用に。動作: FUP発行(9600 8N1)→300ms待ち→LPUART1/USART1を115200 8E1(WORDLENGTH_9B+EVEN)に再Init→USART1↔LPUART1 レジスタ直叩き透過中継(終了=電源OFF)。本体IWDGは無効なのでrefresh不要。

**残: 実機検証 GO/NO-GO** — ①IWDG オプションバイトが software mode か(hardware だと bootloader が落ちる) ②empty-check が既定オプションバイトで効くか ③FUP,45063→OK→CubeProgrammer/stm32flash(br=115200,P=EVEN) で実書込。**本体側ブリッジは製品リリース前に治具専用FWへ分離(現状は同一ソースをフラグ制御)。**
