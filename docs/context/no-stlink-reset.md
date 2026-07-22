# ST-Link リセット厳禁（旧 auto-memory: feedback）

> 出所: Claude auto-memory `feedback_no_stlink_reset.md`（type: feedback / originSession 4b13ce96… / modified 2026-07-22）を 2026-07-22 にリポへ移設。
> メモリ側は本ファイルへのポインタのみ。編集はここを真実源とする。

IM-110 本体実機で、**ST-Link に「接続確認だけして走らせ直さない」系のコマンドを叩くと本体電源が落ちる**。2026-07-22、`st-flash write` を指示された直後に Claude が `st-info --probe` を実行 → 本体電源断。ユーザーから繰り返し強く指摘されている（今回で複数回目）。

**Why:** 本体は **MCU が自分の電源を保持する自己ラッチ電源**（`V_BAT`→Q4 P-MOSFET 主電源→MAX8881 U9→3V3。ソフト電源OFF `shutt_down()` mainSub.c:553 が存在＝MCU の GPIO で Q4 ゲートを ON 保持している証拠）。`st-info --probe` はコアを halt/reset のまま放置 or NRST を握って抜けるため、保持 GPIO が復帰せず Q4 が外れて電源が落ちる、という筋が最も辻褄が合う（**機構は未確定・断定しない**）。**「リセット＝必ず電源断」ではない**：通常の書込は最後に reset→run で終わり、起動した MCU が保持を張り直すので落ちない。

**How to apply（運用ルール — 機構論より優先）:**
- **書き込みはユーザーの既知安全経路のみ**: (1) `st-flash --format ihex write <hex>`（例: `st-flash --format ihex write ./x.hex`。build/IM-110.hex 等）、(2) デバッガで .elf 書込（gdb load、ユーザーの openocd cfg 経由で走行中 attach）。どちらも最後に走らせて終わるので電源保持が戻る。
- **禁止（事前確認なしに絶対実行しない）**: `st-info`（--probe 含む）、`openocd` の単発 probe / 接続確認、connect-under-reset、NRST アサート、`monitor reset` 系。要は「接続だけして走らせ直さない」操作全部。
- ST-Link に触れる操作の前は必ず、それが最後に target を run させて終わるか確認。少しでも不明なら実行前にユーザーへ確認する。
- 一度落とすと SW3 電源 ON/OFF での復帰が必要（プローブも本体給電依存で道連れ）。
- 関連: プローブは本体給電依存 → `probe-fup-bridge.md` / 実機検証は本体UART経由（AMV/AAD0）。
