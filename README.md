# heuristic-tips

AtCoder Heuristic Contest（AHC）のhuman only部門に向けて、ヒューリスティックのtips、過去AHCの解法、C++の実装例をまとめた静的サイトだ。VitePressで作っている。公開先は https://tyokousagi.github.io/heuristic-tips/ だ。

## 使い方

```bash
npm install
```

```bash
npm run docs:dev
```

```bash
npm run docs:build
```

ビルドの結果は `docs/.vitepress/dist` に出力される。

## 検査

```bash
npm run check
```

次の3つを実行する。g++ が必要だ。

- `scripts/check-snippets.mjs`。スニペットをコンパイルし、`snippets/tests` のテストを実行する。`verify/` の検証コードは、コンパイルが通ることだけを確かめる
- `scripts/check-tips.mjs`。すべてのtipsに出典があることと、出典が登録されていることを確かめる
- `scripts/check-ahc.mjs`。過去AHCメモの各回に、一覧用のコメント、要点の段落、「なぜ強いか」、出典のリンクがあることを確かめる。ラベルとコロンで始まる行が残っていないことと、横断メモからのリンク先があることも確かめる

## 構成

| 場所 | 内容 |
|---|---|
| `docs/tips/` | tips集。1件は `### 見出し`、説明、`<Src id="..." />` の3つでできている |
| `docs/.vitepress/sources.json` | 出典の一覧。tipsからは `id` で参照する |
| `docs/ahc/` | 過去AHCメモ。`001-010.md` などに1回1節で書き、`patterns.md` に横断メモを置いた。一覧表は、各節のコメントと最初の段落から自動で作る |
| `verify/` | 過去AHCの解法を手元で確かめたC++のコード。結果は `verify/README.md` に書いた |
| `docs/guide/`、`docs/methods/`、`docs/techniques/`、`docs/workflow/` | 解説記事 |
| `snippets/` | C++のスニペットとテスト。記事からは `<<<` で取り込む |

## tipsの追加

1. 出典を `docs/.vitepress/sources.json` に足す。
2. カテゴリのページの `<!-- +名前 -->` の直前に、tipsを足す。
3. `npm run check` を実行する。

## 過去AHCメモの追加

1. 該当するページ（例: `docs/ahc/061-074.md`）に `## AHC075 名前 {#ahc075}` の節を足す。範囲を超えるときは新しいページを作り、`docs/.vitepress/config.ts` のサイドバーに足す。
2. 見出しの直後に `<!-- ahc: 長期 | 年-月 | 題材 | 解法 -->` を書き、続けて要点を1〜2文の段落で書く。
3. `### 問題`、`### 上位の解法`、`### なぜ強いか`、`### うまくいかなかったこと`、`### ほかの問題に使える考え方`、`### 出典` の順に書く。手元で確かめた回は、`### ほかの問題に使える考え方` の前に `### 手元で確かめた結果` を足す。
4. `npm run check` を実行する。

## 公開

human only部門でスニペットを使うには、コンテストの開始前にサイトを公開し、提出コードに公開先のURLを書く必要がある。

1. GitHubにリポジトリを作り、Settings > Pages の Source を「GitHub Actions」にする。
2. 公開先のURLをスニペットに書き込む。

   ```bash
   node scripts/set-site-url.mjs https://ユーザー名.github.io/リポジトリ名
   ```

3. `main` ブランチにpushすると、`.github/workflows/deploy.yml` がビルドして公開する。

## 文章

日本語の文章は、[yomiyasu](https://github.com/nanaism/yomiyasu) の指針に沿って常体で書き、付属の検査をかけている。スキルは外部のものなので、このリポジトリには含めていない。次のコマンドで `.claude/skills/yomiyasu` に入る。

```bash
npx skills add nanaism/yomiyasu -a claude-code -y
```
