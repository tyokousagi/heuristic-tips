# スニペット一覧

コンテストでそのまま貼れるC++のコードを集めた。どのファイルもほかのファイルに依存していないので、必要なものだけを貼れる。コードはすべてテストでコンパイルと動作を確かめている。

各コードの1行目のコメントはこのページの中の公開先URLだ。human only部門で使うときは、このコメントを消さずに提出コードへ貼る。理由は[サイトの使い方とルール](/guide/about)に書いた。

## ひな形 {#template}

入力、出力、時間の管理、乱数、スコアの表示を入れた、1ファイルのひな形だ。

<<< @/../snippets/template.cpp

## Timer {#timer}

経過時間を秒で返す。使い方は[時間計測](/techniques/timer)にある。

<<< @/../snippets/timer.hpp{cpp}

## Xorshift {#xorshift}

軽い乱数生成器だ。整数と実数の乱数を返す。

<<< @/../snippets/xorshift.hpp{cpp}

## 焼きなましの骨格 {#sa}

時間の管理、温度の計算、受理の判定、最良スコアの記録をまとめた関数だ。近傍の選び方と反映の処理を、ラムダ式で渡す。

<<< @/../snippets/sa.hpp{cpp}

## 巡回路の近傍 {#tsp}

2-optとOr-optの差分計算と適用だ。

<<< @/../snippets/tsp.hpp{cpp}

## logの表 {#log-table}

受理のしきい値に使う log(乱数) を、表から引く。

<<< @/../snippets/log_table.hpp{cpp}

## ビームサーチの骨格 {#beam-search}

状態をコピーする素直なビームサーチだ。ハッシュ値での重複除去と、行動の列の復元を含む。

<<< @/../snippets/beam_search.hpp{cpp}

## chokudaiサーチの骨格 {#chokudai-search}

<<< @/../snippets/chokudai_search.hpp{cpp}

## IndexSet {#index-set}

追加、削除、ランダムな要素の取得がO(1)の整数の集合だ。

<<< @/../snippets/index_set.hpp{cpp}

## Zobrist hash {#zobrist}

<<< @/../snippets/zobrist.hpp{cpp}

## 一括クリアが速い配列 {#stamp-set}

<<< @/../snippets/stamp_set.hpp{cpp}

## 開番地法のハッシュ集合 {#hash-set}

<<< @/../snippets/hash_set.hpp{cpp}

## 番兵つきグリッド {#grid}

方向の配列、1次元の添字、BFSを含む。

<<< @/../snippets/grid.hpp{cpp}

## Union-Find {#dsu}

<<< @/../snippets/dsu.hpp{cpp}

## ダイクストラ法 {#dijkstra}

<<< @/../snippets/dijkstra.hpp{cpp}

## 正規分布の確率 {#normal-prob}

推定を行う問題で使う、丸めた観測値の確率と、対数尤度から確率への変換だ。

<<< @/../snippets/normal_prob.hpp{cpp}

## テストの並列実行 {#run-tests}

複数のケースを並列に実行して、平均スコア、対数の平均、相対スコア、失敗したケースを表示するPythonのスクリプトだ。使い方は[ローカルテスト](/workflow/tester)にある。

<<< @/../snippets/run_tests.py
