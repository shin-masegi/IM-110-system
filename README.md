# IM-110-system

IM-110 製品の本体基板・プローブ基板を横断管理するための調整用リポジトリ。

実際の本体ファームウェアは [IM-110](https://github.com/shin-masegi/IM-110) リポジトリ、
プローブ側は [IM-110_Probe](https://github.com/shin_masegi/IM-110_Probe) リポジトリで管理しています。

## このリポジトリの役割

- **プロトコル仕様の一元管理** (`docs/protocol-rs232c.md`)
- **デバッグログのインボックス** (`docs/debug-log/` — Claude.ai での議論をここに集約)
- **Claude Code 用の横断ルール** (`CLAUDE.md`)
- **カスタムスラッシュコマンド** (`.claude/commands/`)

## セットアップ

```bash
# 親ディレクトリに 3 リポジトリを兄弟配置する
cd ~/dev
git clone git@github.com:shin-masegi/IM-110.git
git clone git@github.com:shin-masegi/IM-110_Probe.git
git clone git@github.com:shin-masegi/IM-110-system.git

# Claude Code を IM-110-system から起動
cd IM-110-system
claude
# セッション内で両方の実装リポジトリを追加
# /add-dir ../IM-110
# /add-dir ../IM-110_Probe
```

## 日常運用

- 通信に関わる変更: 必ず `docs/protocol-rs232c.md` を先に更新 → 両側コード修正
- Claude.ai でデバッグ: まとめを `docs/debug-log/YYYY-MM-DD-title.md` として保存
- 溜まったインボックスの取り込み: Claude Code で `/ingest-debug-log` を実行
- プロトコル整合性チェック: `/sync-protocol` で両側の実装が仕様と合っているか確認
