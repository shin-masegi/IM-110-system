# docs/context — 横断コンテキスト（旧 auto-memory 移設先）

Claude の auto-memory（`~/.claude/projects/.../memory/`）に溜めていた横断コンテキストを、
セッション跨ぎで確実に持ち越すため **リポ内 md へ全文移設**した（2026-07-22）。
以降、メモリ側は本ディレクトリへのポインタのみを持つ。**内容の真実源はここ**。

| ファイル | 種別 | 概要 |
|---|---|---|
| [hardware-truth-source.md](hardware-truth-source.md) | feedback | 基板事実は回路図+NET が真実源、FW 設定は二次資料。CLAUDE.md §3.5 に明文化 |
| [no-stlink-reset.md](no-stlink-reset.md) | feedback | ST-Link で「接続だけして走らせ直さない」操作は本体電源が落ちる。書込は既知安全経路のみ |
| [probe-fup-bridge.md](probe-fup-bridge.md) | project | プローブ UART FW 更新（FUP+本体8E1パススルー）。リリース前に治具専用FW化 |
| [probe-coefficient-store.md](probe-coefficient-store.md) | project | 係数の統合ストア化仕様（§12, 512B/32Bページ, Page63）。本体EEPROMバイト同一ミラー。**移行未完** |
| [adjust-mode.md](adjust-mode.md) | project | 調整機能(ADBOAD)設計・進捗。設計真実源は docs/adjust-mode-design.md |
| [local-llm-server.md](local-llm-server.md) | project | 社内 LLM 推論サーバー導入検討（社長打診予定） |

## 注意 — 係数モデルの移行（未完）

現行の係数保存レイアウトの真実源は `mlss-calc-reference.md §12`（512B 統合ストア `probe_store_t`, 16ページ×32B, Page63/`STORE_AD`）、
通信の真実源は **`protocol-rs232c.md §3.2.6`（32B ページ転送 `RPG`/`WPG`/`WSC`, 2026-07-23 改訂済み）**。
旧の per-eq/per-mode 意味論コマンド（旧測定モデル・`kiza` 系）は退役し、統合ストアの係数ページ（Page3-15）へ吸収する方針。
ただし**プローブ FW / 本体 FW の実装が旧係数コードのまま残っており未追従**（旧係数構造が統合ストアと同じ Page63 を使い衝突するバグを含む）。
詳細と残作業は [`../handoff.md`](../handoff.md) §4 / §6(T1)。
