# IM-110 既知の不具合バックログ (Phase 2 で対応)

通信信頼性の切り分けとは別系統で見つかった不具合を、後で潰すために記録する。

---

## KI-1: 電源OFF時の「保管方法」ガイドが毎回表示される (本来は1日1回)

- **発見**: 2026-07-19。ユーザー報告「電源OFFで保管方法表示が毎回でる。1日1回だけのはず」。
- **系統**: 本体 UI/EEPROM 永続化。**vault/通信の件とも、私(Claude)の切り分け変更とも無関係**
  (577a769 起動分離リファクタも該当領域=main.c init 部を未変更、と git 確認済み)。

### 確定した直接原因 (openocd 実機読取, 電源断なし)
- `guide_disp_flag[0..4]` が起動時 **全部 0**(=表示する)。
- `LastDate` = 19/12/11 = RTC 現在日と一致(検証機 RTC は 2019/12/11)。
- フラグは起動時 EEPROM 復元のみ(他に毎起動 0 化する箇所なし)。
  → **「表示済(=2)」が EEPROM に永続化されていない**ため毎起動 0 → 毎回表示。

### 構造的弱点 (初期コミット由来・今回の regression ではない)
- `main.c:368-370` で `LastDate=今日` を、`main.c:400` の `day_disp_reset()`
  (`今日 != LastDate` で `guide_disp_flag` 2→0)**より前**に上書き。
  よって起動時の日次リセットは常に不発。1日1回はフラグ永続化のみに依存していた。

### 永続化が崩れた場所の候補 (未確定・Phase 2 で切り分け)
1. 電源OFF `shutt_down()`(mainSub.c:553)で flag=2 セット + `Req_SetupData_Write=1` するが、
   保存 `eep_write_setting()`(消費は mainSub.c:925)が電源断前に走っていない可能性。
   - guide 表示経路は2つ: `mainSub.c:567`(shutt_down) と `Normal.c:3589`。
     Normal.c:3780-3781 は flag=1 + Req_Write をセット。どちらが実際に保存まで到達するか要確認。
2. vault 対応(e933955 / 57171eb)で EEPROM SetupData の offset がずれ、
   `guide_disp_flag`/`LastDate` の読み書き位置が不一致になった可能性(Eeprom.c:726-729 / 1386-1389)。

### デバッグ手段
- 走行中 RAM 読取(電源断なし)= openocd。手順は memory `im-110-debugger-live-read`。
  アドレス: `guide_disp_flag=0x20003c74`(5B), `LastDate=0x20003c64`(3B)。
- UART デバッグダンプ: LinkSerial.c:1074(guide_disp_flag) / 1097(LastDate)。
- EEPROM 実内容の該当 offset を読めば候補1 vs 2 を確定できる。
