# Claude Code 環境引き継ぎ手順（アカウント/マシン切替用）

新しい Claude Code アカウント（会社アカウント等）や別マシンで IM-110 開発を再開するための手順。
**設計方針: 開発文脈の正本はすべてリポジトリ内**（`CLAUDE.md` / `docs/handoff.md` / `docs/context/`）にあり、
Claude Code のローカルメモリはポインタしか持たない。したがって以下の手順だけで文脈が完全に復元される。

作成: 2026-08-04（個人→会社アカウント切替のため整備）

---

## 1. リポジトリの clone（3つ、同一親ディレクトリに並べる）

```bash
git clone git@github.com:shin-masegi/IM-110-system.git
git clone https://github.com/shin-masegi/IM-110.git
git clone https://github.com/shin-masegi/IM-110_Probe.git
```

ディレクトリ構成は `IM-110-system/CLAUDE.md §2` のとおり（3リポが兄弟階層に並ぶこと）。
※ アカウント切替時は GitHub 側のアクセス権（会社アカウントへの collaborator 追加 or リポ移管）を先に済ませること。

## 2. Claude Code の起動

1. `IM-110-system/` から起動する
2. `/add-dir ../IM-110` と `/add-dir ../IM-110_Probe` で両コードツリーを追加
3. 初回セッションは `CLAUDE.md §5` のチェックリスト（`docs/handoff.md` → `docs/context/README.md` →
   `docs/protocol-rs232c.md`）を読めば現状に追いつける。**ローカルメモリの移行作業は不要**
   （メモリはリポ md へのポインタのみで、内容はすべてリポ側にある）

## 3. 手動コピーが必要なファイル（リポ外・個人設定）

| ファイル | 内容 | 対応 |
|---|---|---|
| `~/.claude/CLAUDE.md` | ユーザーグローバル指示（作業範囲・報告規律・応答スタイル等） | 旧マシンから新マシンの同パスへコピー |

これ以外に引き継ぎ必須のローカルファイルは無い（プロジェクトメモリは新環境で自然に再構築される）。

## 4. ビルドツールチェーン

| ツール | 用途 | 入手 |
|---|---|---|
| `arm-none-eabi-gcc` | 両FWのビルド (`make -j`) | Arm GNU Toolchain（実績: 15.2.rel1、`/Applications/ArmGNUToolchain/…` を PATH に） |
| `st-flash` (stlink) | 本体 FW 書き込み | `brew install stlink`（実績: v1.8.0） |
| `stm32flash` | プローブ FW 書き込み（FUP ブリッジ経由） | `brew install stm32flash` |
| `python3` + `pyserial` | CN2 シリアルデバッグ（AD/AWDS 等） | `python3 -m pip install --user --break-system-packages pyserial` |

## 5. 実機接続

- **CN2（本体デバッグ UART）**: USB-Serial アダプタで接続。**9600bps 8N1**（本体⇔プローブ間を 38400 化した後も CN2 は 9600 のまま。protocol 0.6.5 §1.1）
  - デバイス名例: `/dev/cu.usbserial-0001`
  - 主要コマンド: `AD`（ADC_mV[0..5] ダンプ、6番目=気圧hPa）/ `AWDS`（水深内部変数）/ `AMIR`（統合ストア状態）
- **ST-Link（SWD、本体 CN1）**: 書き込み専用。**厳守ルールあり → `docs/context/no-stlink-reset.md` を必ず読む**
  - 許可: `st-flash --format ihex write build/IM-110.hex`（最後に run で終わる）
  - 禁止: `st-info` 等の「接続だけして走らせ直さない」操作全部（本体電源が落ちる）
  - 書込系操作（st-flash / FUP）は毎回、実行前に宣言してユーザーの合図を待つ
- **プローブ書き込み**: SWD 不要、本体経由 FUP ブリッジ → `docs/fup-procedure.md`
  （`FUP,45063` 送信 → `stm32flash -b 9600 -w IM-110_Probe.bin -v -S 0x08000000 <port>`）

## 6. 絶対ルールの所在（新環境で最初に読むもの）

1. `IM-110-system/CLAUDE.md` — §3 絶対ルール（protocol 真実源・両側同時修正・ハード真実源・commit/push 運用）
2. `docs/handoff.md` — 現状スナップショット（FWバージョン・直近の変更・残件）
3. `docs/context/README.md` — 横断コンテキスト索引（ST-Link 禁止事項・FUP 方針・ストア仕様等）
4. `docs/protocol-rs232c.md` — 通信仕様の唯一の真実源

## 7. 検証（環境が正しく引き継げたかの確認）

```bash
cd IM-110 && make -j          # → build/IM-110.hex が生成されること
cd ../IM-110_Probe && make -j # → build/IM-110_Probe.hex が生成されること
# 実機接続時: CN2 に AD を送って 6 値が返ること（例は docs/handoff.md 参照）
```
