# docs/context — 横断コンテキスト（旧 auto-memory 移設先）

Claude の auto-memory（`~/.claude/projects/.../memory/`）に溜めていた横断コンテキストを、
セッション跨ぎで確実に持ち越すため **リポ内 md へ全文移設**した（2026-07-22）。
以降、メモリ側は本ディレクトリへのポインタのみを持つ。**内容の真実源はここ**。

| ファイル | 種別 | 概要 |
|---|---|---|
| [hardware-truth-source.md](hardware-truth-source.md) | feedback | 基板事実は回路図+NET が真実源、FW 設定は二次資料。CLAUDE.md §3.5 に明文化 |
| [no-stlink-reset.md](no-stlink-reset.md) | feedback | ST-Link で「接続だけして走らせ直さない」操作は本体電源が落ちる。書込は既知安全経路のみ |
| [probe-fup-bridge.md](probe-fup-bridge.md) | project | プローブ UART FW 更新（FUP+本体8E1パススルー）。リリース前に治具専用FW化 |
| [probe-coefficient-vault.md](probe-coefficient-vault.md) | project | 旧 vault 化仕様（cf[7] 系, Page63）。**現行目標は §12 統合ストア、移行未完** |
| [adjust-mode.md](adjust-mode.md) | project | 調整機能(ADBOAD)設計・進捗。設計真実源は docs/adjust-mode-design.md |
| [local-llm-server.md](local-llm-server.md) | project | 社内 LLM 推論サーバー導入検討（社長打診予定） |

## 注意 — 係数モデルの二重化（未解決）

`probe-coefficient-vault.md` / `adjust-mode.md` が記す係数 vault（`probe_vault_t`, cf[7]/modecf[7]/kiza,
Page63/VAULT_AD）は**旧モデル**。測定モデル再設計で係数レイアウトは
`mlss-calc-reference.md §12`（512B 統合ストア `probe_store_t`）へ移す方針に変わったが、
**protocol-rs232c.md §3.2.6 と本体 FW は旧モデルのまま**で、プローブの新ストアと非互換・同一 Page63 で衝突する。
詳細と残作業は [`../handoff.md`](../handoff.md) §4 / §6(T1)。
