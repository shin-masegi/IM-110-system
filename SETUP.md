# IM-110-system セットアップ手順

このドキュメントは、今回受け取った雛形を実際に稼働させるまでの手順書です。
**順番に上から実行**してください。

---

## 事前準備

- 既存の `IM-110`（本体）と `IM-110_Probe`（プローブ）リポジトリがローカルに clone 済みであること
- Claude Code が `claude` コマンドで起動できること
- Git が使えること
- (任意) pandoc がインストール済み（Word 生成したい場合）

---

## ステップ 1: ディレクトリ配置を揃える

既存の 2 リポジトリと新しい IM-110-system が **同じ親ディレクトリ配下に兄弟として並ぶ**
ようにします。これが `/add-dir ../IM-110` のような相対パス参照の前提です。

```bash
# 例: ~/dev/ 直下に並べる場合
cd ~/dev
ls
#  IM-110/           ← 既存（本体）
#  IM-110_Probe/     ← 既存（プローブ）

# もし別々の親ディレクトリにあるなら、どちらかに寄せてください
```

もし既に ~/dev/IM-110 と ~/work/IM-110_Probe のように散らばっている場合、
シンボリックリンクでも OK です:

```bash
cd ~/dev
ln -s ~/work/IM-110_Probe IM-110_Probe
```

---

## ステップ 2: IM-110-system をこの場所に配置

提供された zip を解凍し、`IM-110-system/` を上記の親ディレクトリに置きます。

```bash
cd ~/dev
unzip /path/to/IM-110-system.zip
ls
#  IM-110/
#  IM-110_Probe/
#  IM-110-system/    ← 追加された
```

---

## ステップ 3: Git 初期化 & GitHub リポジトリ作成

```bash
cd ~/dev/IM-110-system
git init
git add .
git commit -m "Initial skeleton: cross-cutting coordinator repo for IM-110"

# GitHub 側で空リポジトリ IM-110-system を作成しておき:
git remote add origin git@github.com:YOUR_ORG/IM-110-system.git
git branch -M main
git push -u origin main
```

`README.md` 内の `YOUR_ORG` を実際の組織/ユーザー名に書き換えることを忘れずに。

---

## ステップ 4: CLAUDE.md の空欄を埋める

現時点の `CLAUDE.md` には以下の TODO があります。実際の値を確認して置き換えてください。

- §6 ビルド・テストコマンド: 各サブリポの実際のビルドコマンド
- (できれば) 各サブリポにも `CLAUDE.md` を設置して、そのリポ固有のルールを書く

---

## ステップ 5: protocol-rs232c.md を実装から埋める

最初の大仕事です。Claude Code で以下のように依頼してください:

```
IM-110-system から Claude Code を起動し:

  /add-dir ../IM-110
  /add-dir ../IM-110_Probe

その後プロンプト:

  docs/protocol-rs232c.md を実装から埋めてください。
  本体・プローブ両方のコードを読み、以下を抽出して TODO 欄を埋めてほしい:
  - 実際のボーレート等のシリアルパラメータ
  - 実際のフレームフォーマット（STX/CRC/ETXの値など）
  - 実装されている全コマンドとその要求/応答構造
  - タイムアウト値
  - エラーコード
  
  抽出元のファイル名と行番号を各項目にコメントで残してください。
  判断に迷った箇所は TODO のまま残し、最後にまとめて報告してください。
```

出てきた差分をレビューし、問題なければコミット:

```bash
git add docs/protocol-rs232c.md
git commit -m "docs: populate protocol spec from implementation (v0.1)"
```

---

## ステップ 6: 運用を回し始める

### 回路デバッグ時（Claude.ai 側）

1. Claude.ai の **Projects** 機能で IM-110 用のプロジェクトを作成
2. そのプロジェクトに以下を添付（ファイルまたはリンク）:
   - `IM-110/Materials/回路図_IM110T_本体基板_20260211.pdf`
   - `IM-110/Materials/IM-110T_本体基板.NET`
   - `IM-110_Probe/Materials/230925_id160t_rev8_1.pdf`
   - `IM-110_Probe/Materials/230925_ID-160T_REV8_1.NET`
   - `IM-110-system/docs/protocol-rs232c.md`（現時点版のコピー）
3. 回路で迷ったらこのプロジェクトでチャット
4. セッション終わりに `docs/debug-log/TEMPLATE.md` の形式でまとめてもらう
5. 返ってきた md を `IM-110-system/docs/debug-log/` に保存 → commit & push

### コード修正時（Claude Code 側）

1. `cd ~/dev/IM-110-system && claude`
2. `/add-dir ../IM-110` `/add-dir ../IM-110_Probe`
3. 普通に開発
4. 未処理デバッグログが溜まったら `/ingest-debug-log`
5. たまに `/sync-protocol` で整合性確認

---

## ステップ 7: Word 仕様書ワークフロー（任意）

### 定常ルート（pandoc）

```bash
# 初回: テンプレート用 docx を作る
# Word で体裁整えた空 docx を docs/templates/reference.docx として保存

# 生成
pandoc docs/protocol-rs232c.md \
  -o build/protocol-rs232c.docx \
  --reference-doc=docs/templates/reference.docx
```

これを `scripts/md2docx.sh` としてスクリプト化しておけば毎回一発です。

### 凝ったレイアウトにしたい時

Claude.ai のチャットに protocol-rs232c.md の内容を貼り付け、
「このmdを、体裁を整えたWord文書として出力して」と依頼。
docx スキルで装飾済みの docx が返ってきます。

---

## トラブルシューティング

### `/add-dir` が効かない / サブリポが見えない

Claude Code は起動時のルートディレクトリより**上**には行けません。
必ず親ディレクトリを共通にしてから起動してください。
あるいは `~/dev/` から起動する運用にすれば、IM-110-system の CLAUDE.md は
自動ロードされませんが、その代わり3つ全部が最初から見えます。
（自動ロードさせたい場合はトレードオフ: `~/dev/` に CLAUDE.md を置くか、
`IM-110-system` から起動するか）

### デバッグログインボックスが溜まりすぎた

`_processed/` へ手動で古いものを移動しても OK です。
`/ingest-debug-log` は `_processed/` 配下を無視します。

### プロトコル仕様を破壊的変更したくなった

1. `docs/protocol-rs232c.md` の MAJOR を上げる
2. `docs/debug-log/` か `docs/release-notes.md` に「本体 vX と プローブ vY から互換」と書く
3. 両リポの該当箇所を同一のタイミングでリリースする計画を立てる
4. 片側だけ先にマージしないこと
