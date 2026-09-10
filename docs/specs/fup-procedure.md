# FUP: 本体経由 プローブ ファーム更新手順（UART / SWD不要）

プローブ (STM32G070) の FW を、**SWD を使わず本体経由の UART パススルー**で書き換える手順。
プローブがケースに固定され SWD (J4) が使えない現場更新を想定（方式A、`protocol-rs232c.md §3.2.5`）。

**2026-07-19 に実機で端から端まで動作確認済み**（本体ブリッジ→プローブROMブートローダー AN3155 応答まで）。

---

## 前提・接続

- **PC ↔ 本体 USART1 (CN2)** を USB-Serial で接続。CN2 = USART1 TXD/RXD（本体 `CLAUDE.md §4` 外部コネクタ）。
- 本体は通常FW（追加機能としてブリッジを内蔵。別ビルド不要）。プローブは Ver.0.13 以降（FUP 対応、Ver.0.17 で ISR ハング修正済み）。
- プローブは本体 CN4 から給電される（単独給電不可）。本体を電源ONにしておく。
- **全区間 9600**。ブリッジ時のみパリティが even(8E1) に変わる（ボーレートは 9600 のまま）。

---

## 手順

1. **PC を 9600 8N1** に設定して本体 USART1(CN2) に接続。
2. 本体へ **`FUP,45063`**（末尾 CRLF）を送信。
   - 本体応答 **`P:FUP`** = ブリッジ移行（この直後に本体は 9600 **8E1** へ切替わる）。
   - 本体は同時にプローブへ `FUP,45063`(9600 8N1) を発行 → プローブが ROM ブートローダーへ移行。
3. **PC を 9600 8E1**（8データ + **even パリティ** + 1stop、ボーレートは 9600 のまま）に切替。
4. `stm32flash` または STM32CubeProgrammer(UART) で**プローブを書換**。本体は PC↔プローブを透過中継する。
   - 疎通確認例（AN3155 オートボーレート）: `0x7F` 送信 → `0x79`(ACK)。`Get(0x00,0xFF)` → `79 0B 31 …`（`0x31`=bootloader Ver.3.1）。
   - `stm32flash` 例:
     ```
     stm32flash -b 9600 -w IM-110_Probe.bin -v -S 0x08000000 /dev/cu.usbserial-XXXX
     ```
     （`stm32flash` は既定で even パリティ。CubeProgrammer は Parity=Even を指定）
5. 書込完了後、**本体・プローブとも電源入れ直し**で通常動作へ復帰。
   - プローブは POR でバックアップレジスタ(FUPマーカ)が 0 クリアされ、誤ってブートローダーへ移行しない。
   - 本体ブリッジは「復帰しない（終了=電源OFF）」設計なので、電源再投入が必須。

---

## 実装リファレンス

- **本体側ブリッジ**: `IM-110/Core/Src/main.c` `ProbeFupBridge_Run()`（通常コンパイル・非static）。
  トリガは `IM-110/Core/Src/LinkSerial.c` `CheckSerialCMD()` の `FUP,45063` 判定 → `P:FUP` 応答 → `ProbeFupBridge_Run()`。
  動作: プローブへ FUP 発行 → 実行中アプリの DMA 受信停止(`HAL_UART_AbortReceive`) → USART1・LPUART1 を **9600 8E1** へ再設定 → ポーリングで PC↔プローブ透過中継（ORE/PE クリア）。
- **プローブ側 FUP**: `IM-110_Probe/Core/Src/IM_110.c` `uart_Enter_Bootloader()`。
  ガード `45063` 一致で `OK` 送信 → `TAMP->BKP0R` にセンチネル格納 → `NVIC_SystemReset()`。
  リセット後 `main()` 冒頭で「バックアップレジスタ一致 かつ ソフトリセット(SFTRST)」なら system memory(`0x1FFF0000`) へジャンプ。

## 注意・ハマりどころ（実機で踏んだ）

- **プローブ `uart_Enter_Bootloader` で `HAL_Delay` を使わない**。本関数は受信ISR(USART2=優先度0)内で走り、SysTick が割り込めず `HAL_Delay` が無限ハング → リセットに到達せずブートローダーへ移行しない。OK 送信は `HAL_UART_Transmit` が TC 完了まで待つので追加待機は不要（Ver.0.17 で `HAL_Delay(5)` を除去）。
- **本体 USART1 のボーレートは実運用 9600**。`MX_USART1_UART_Init` の CubeMX 初期値だけ見ると 115200 に見えるが、`UART1_BPS`(=9600) で再設定される。ブリッジも 9600 に合わせること（プローブが 9600 のため。無駄な 115200 ジャンプは不要）。
- ブリッジは baud 据置・**parity のみ 8N1→8E1**。PC 側も parity を even にするだけ。
- 本体側 BLE(RN4871) は USART1 共用だが、ブリッジ移行後は電源OFFまで戻らない前提。
