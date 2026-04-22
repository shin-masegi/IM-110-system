# デバッグログ インボックス

ここは **Claude.ai でのデバッグ議論の溜め場** です。

## 流れ

1. 回路図やプローブ基板の挙動を Claude.ai Projects で相談する
   （本リポとは別に、Claude.ai 側のプロジェクトに回路図PDFとnetlistを置いておく）
2. セッション終わりに Claude.ai に以下を依頼:

   > このセッションの議論を、事実 / 仮説 / 検証した結果 / 次に検証すべきこと / 関連ファイル
   > の5セクションに分けて、`docs/debug-log/TEMPLATE.md` のフォーマットで md としてまとめて。
   > ファイル名は `YYYY-MM-DD-短いタイトル.md` の形式で提案して。

3. 返ってきた内容をこの `docs/debug-log/` 配下に新規ファイルとして保存（コピペで10秒）
4. コミット & プッシュ
5. 後日 Claude Code で `/ingest-debug-log` を実行
   → 未処理エントリを読み、関連する `CLAUDE.md` / `protocol-rs232c.md` / ソースコメント
     への反映案を生成 → 承認してPR化

## ファイル命名規則

```
YYYY-MM-DD-短いタイトル.md          ← 未処理 (インボックス)
_processed/YYYY-MM-DD-短いタイトル.md  ← 取り込み済みは _processed/ に移動
```

`/ingest-debug-log` コマンドは取り込み完了後に `_processed/` への移動を提案します。

## テンプレート

`TEMPLATE.md` をコピーして使ってください（Claude.ai にもこのテンプレートを渡せば同形式で返ってきます）。

## やらないこと

- ここは **議論の一次記録** であって、仕様書ではありません。
- ここにプロトコル定義や API 仕様を書かないこと。
  確定した仕様は `docs/protocol-rs232c.md` や各リポジトリの `CLAUDE.md` に反映する。
