# heuristic-tips

AtCoder Heuristic Contest（AHC）のhuman only部門に向けた、ヒューリスティックのtipsとC++の実装例の静的サイトです。VitePressで作っています。

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

ビルドの結果は `docs/.vitepress/dist` に出力されます。

## 検査

```bash
npm run check
```

次の2つを実行します。g++ が必要です。

- `scripts/check-snippets.mjs`。スニペットをコンパイルし、`snippets/tests` のテストを実行します
- `scripts/check-tips.mjs`。すべてのtipsに出典があることと、出典が登録されていることを確かめます

## 構成

| 場所 | 内容 |
|---|---|
| `docs/tips/` | tips集。1件は `### 見出し`、説明、`<Src id="..." />` の3つ |
| `docs/.vitepress/sources.json` | 出典の一覧。tipsからは `id` で参照します |
| `docs/guide/`、`docs/methods/`、`docs/techniques/`、`docs/workflow/` | 解説記事 |
| `snippets/` | C++のスニペットとテスト。記事からは `<<<` で取り込みます |

## tipsの追加

1. 出典を `docs/.vitepress/sources.json` に足します。
2. カテゴリのページの `<!-- +名前 -->` の直前に、tipsを足します。
3. `npm run check` を実行します。

## 公開

human only部門でスニペットを使うには、コンテストの開始前にサイトを公開し、提出コードに公開先のURLを書く必要があります。

1. GitHubにリポジトリを作り、Settings > Pages の Source を「GitHub Actions」にします。
2. 公開先のURLをスニペットに書き込みます。

   ```bash
   node scripts/set-site-url.mjs https://ユーザー名.github.io/リポジトリ名
   ```

3. `main` ブランチにpushすると、`.github/workflows/deploy.yml` がビルドして公開します。

## 文章

日本語の文章は [yomiyasu](https://github.com/nanaism/yomiyasu) の指針に沿って書き、付属の検査をかけています。スキルは外部のものなので、このリポジトリには含めていません。次のコマンドで `.claude/skills/yomiyasu` に入ります。

```bash
npx skills add nanaism/yomiyasu -a claude-code -y
```
