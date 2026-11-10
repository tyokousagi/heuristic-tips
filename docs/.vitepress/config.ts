import { defineConfig } from 'vitepress'

// GitHub Pages のプロジェクトサイトで公開するときは SITE_BASE=/リポジトリ名/ を渡す
const base = process.env.SITE_BASE ?? '/'

export default defineConfig({
  lang: 'ja',
  title: 'ヒューリスティックtips',
  description: 'AHC向けのヒューリスティックのtipsとC++実装例',
  base,
  cleanUrls: true,
  markdown: {
    lineNumbers: false,
  },
  themeConfig: {
    nav: [
      { text: 'tips集', link: '/tips/' },
      { text: '過去AHC', link: '/ahc/' },
      { text: '手法', link: '/methods/greedy' },
      { text: 'テクニック', link: '/techniques/random' },
      { text: 'スニペット', link: '/snippets/' },
      { text: '運用', link: '/workflow/tester' },
      { text: '出典', link: '/sources' },
    ],
    sidebar: [
      {
        text: 'はじめに',
        collapsed: false,
        items: [
          { text: 'サイトの使い方とルール', link: '/guide/about' },
          { text: 'コンテストの進め方', link: '/guide/flow' },
          { text: '問題文を読んだあとの作業', link: '/guide/first-steps' },
          { text: '手法の選び方', link: '/guide/choosing' },
        ],
      },
      {
        text: 'tips集',
        collapsed: false,
        items: [
          { text: 'tips集の入口', link: '/tips/' },
          { text: '考察と方針決め', link: '/tips/thinking' },
          { text: '貪欲と初期解', link: '/tips/greedy' },
          { text: '山登りと焼きなましの基本', link: '/tips/sa-basic' },
          { text: '焼きなましの近傍', link: '/tips/sa-neighbor' },
          { text: '焼きなましの温度と受理', link: '/tips/sa-temp' },
          { text: '焼きなましの差分計算と高速化', link: '/tips/sa-speed' },
          { text: '焼きなましの制約と運用', link: '/tips/sa-misc' },
          { text: 'ビームサーチ', link: '/tips/beam' },
          { text: 'その他の探索', link: '/tips/search-other' },
          { text: 'インタラクティブと推定', link: '/tips/interactive' },
          { text: '評価関数とスコア', link: '/tips/evaluation' },
          { text: '高速化と実装', link: '/tips/speed' },
          { text: 'データ構造', link: '/tips/data-structures' },
          { text: '題材別', link: '/tips/topics' },
          { text: 'テストとパラメータ調整', link: '/tips/testing' },
          { text: '時間の使い方と学び方', link: '/tips/time' },
          { text: 'デバッグとよくあるミス', link: '/tips/debug' },
        ],
      },
      {
        text: '過去AHCメモ',
        collapsed: false,
        items: [
          { text: '一覧', link: '/ahc/' },
          { text: '横断メモ', link: '/ahc/patterns' },
          { text: 'AHC001〜010', link: '/ahc/001-010' },
          { text: 'AHC011〜020', link: '/ahc/011-020' },
          { text: 'AHC021〜030', link: '/ahc/021-030' },
          { text: 'AHC031〜040', link: '/ahc/031-040' },
          { text: 'AHC041〜050', link: '/ahc/041-050' },
          { text: 'AHC051〜060', link: '/ahc/051-060' },
          { text: 'AHC061〜074', link: '/ahc/061-074' },
        ],
      },
      {
        text: '手法',
        collapsed: true,
        items: [
          { text: '貪欲法', link: '/methods/greedy' },
          { text: '山登り法', link: '/methods/hill-climbing' },
          { text: '焼きなましの基本', link: '/methods/sa-basic' },
          { text: '焼きなましの近傍設計', link: '/methods/sa-neighbor' },
          { text: '焼きなましの温度と受理判定', link: '/methods/sa-temperature' },
          { text: '焼きなましの差分計算', link: '/methods/sa-speed' },
          { text: '焼きなましでの制約の扱い', link: '/methods/sa-constraints' },
          { text: '多点スタートと大近傍', link: '/methods/sa-variants' },
          { text: 'ビームサーチの基本', link: '/methods/beam-basic' },
          { text: 'ビームサーチの評価と多様性', link: '/methods/beam-eval' },
          { text: '差分更新型ビームサーチ', link: '/methods/beam-diff' },
          { text: 'chokudaiサーチ', link: '/methods/chokudai' },
          { text: 'モンテカルロ法', link: '/methods/montecarlo' },
          { text: 'MCTS', link: '/methods/mcts' },
          { text: 'インタラクティブ問題の推定', link: '/methods/interactive' },
        ],
      },
      {
        text: 'テクニック',
        collapsed: true,
        items: [
          { text: '乱数', link: '/techniques/random' },
          { text: '時間計測', link: '/techniques/timer' },
          { text: '高速化全般', link: '/techniques/speed' },
          { text: '評価関数の設計', link: '/techniques/evaluation' },
          { text: 'Zobrist hash', link: '/techniques/zobrist' },
          { text: 'ランダム選択できる集合', link: '/techniques/index-set' },
          { text: 'グリッドの扱い', link: '/techniques/grid' },
          { text: '最短路と前計算', link: '/techniques/shortest-path' },
          { text: '巡回路の近傍', link: '/techniques/tour' },
          { text: '近傍カタログ', link: '/techniques/neighbors' },
        ],
      },
      {
        text: '開発環境と運用',
        collapsed: true,
        items: [
          { text: 'ローカルテスト', link: '/workflow/tester' },
          { text: 'スコアの比較', link: '/workflow/compare' },
          { text: 'パラメータ調整', link: '/workflow/optuna' },
          { text: 'ビジュアライザとデバッグ', link: '/workflow/visualizer' },
          { text: '提出前チェックリスト', link: '/workflow/checklist' },
        ],
      },
      {
        text: 'スニペットと出典',
        collapsed: true,
        items: [
          { text: 'スニペット一覧', link: '/snippets/' },
          { text: '出典一覧', link: '/sources' },
        ],
      },
    ],
    search: {
      provider: 'local',
      options: {
        // 日本語は空白で区切られないので、Intl.Segmenter で単語に分けて索引を作る
        miniSearch: {
          options: {
            tokenize: (text: string) => {
              const segmenter = new Intl.Segmenter('ja', { granularity: 'word' })
              const tokens: string[] = []
              for (const s of segmenter.segment(text.toLowerCase())) {
                if (s.isWordLike) tokens.push(s.segment)
              }
              return tokens
            },
          },
          searchOptions: {
            combineWith: 'AND',
            prefix: true,
          },
        },
        translations: {
          button: { buttonText: '検索', buttonAriaLabel: '検索' },
          modal: {
            noResultsText: '見つからなかった',
            resetButtonTitle: '消去',
            footer: { selectText: '選択', navigateText: '移動', closeText: '閉じる' },
          },
        },
      },
    },
    outline: { level: [2, 3], label: 'このページの内容' },
    docFooter: { prev: '前のページ', next: '次のページ' },
    darkModeSwitchLabel: '表示',
    sidebarMenuLabel: 'メニュー',
    returnToTopLabel: '先頭に戻る',
  },
})
