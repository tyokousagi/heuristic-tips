// 巡回路の近傍の差分計算と、焼きなましの骨格のテスト
#include <cassert>
#include <cmath>
#include <cstdio>
#include <numeric>

#include "../sa.hpp"
#include "../timer.hpp"
#include "../tsp.hpp"
#include "../xorshift.hpp"

int main() {
    const int n = 200;
    Xorshift rng(42);
    std::vector<double> xs(n), ys(n);
    for (int i = 0; i < n; ++i) {
        xs[i] = rng.next_double() * 1000;
        ys[i] = rng.next_double() * 1000;
    }
    const auto dist = [&](int a, int b) { return std::hypot(xs[a] - xs[b], ys[a] - ys[b]); };

    std::vector<int> tour(n);
    std::iota(tour.begin(), tour.end(), 0);

    // 差分で求めた値が、全体を計算し直した値と一致することを確かめる
    for (int iter = 0; iter < 2000; ++iter) {
        const double before = tour_length(tour, dist);
        double delta;
        if (iter % 2 == 0) {
            int i = rng.next_int(0, n), j = rng.next_int(0, n);
            if (i == j) continue;
            if (i > j) std::swap(i, j);
            delta = two_opt_delta(tour, dist, i, j);
            two_opt_apply(tour, i, j);
        } else {
            const int len = rng.next_int(1, 4);
            const int i = rng.next_int(1, n - len + 1);
            const int j = rng.next_int(0, n);
            if (i - 1 <= j && j <= i + len - 1) continue;
            const bool rev = rng.next_int(2u);
            delta = or_opt_delta(tour, dist, i, len, j, rev);
            or_opt_apply(tour, i, len, j, rev);
        }
        const double after = tour_length(tour, dist);
        assert(std::abs((after - before) - delta) < 1e-6);
    }
    // 並びが順列のままであること
    std::vector<int> sorted = tour;
    std::sort(sorted.begin(), sorted.end());
    for (int i = 0; i < n; ++i) assert(sorted[i] == i);

    // 焼きなましで巡回路が短くなること。最大化の形にするので、スコアは -長さ。
    std::iota(tour.begin(), tour.end(), 0);
    const double init_len = tour_length(tour, dist);
    Timer timer;
    // #region usage
    // 巡回路の長さを最小化する。anneal は最大化なので、スコアを -長さ にする
    std::vector<int> best_tour = tour;
    int mi = 0, mj = 0; // propose が選んだ近傍を覚えておく
    const AnnealConfig cfg{timer.elapsed() + 0.3, 30.0, 0.5};
    const AnnealResult res = anneal(
        timer, rng, cfg, -init_len,
        // propose: 2-opt の区間を選び、スコアの差分を返す。状態は変えない
        [&](double) {
            mi = rng.next_int(0, n);
            mj = rng.next_int(0, n);
            if (mi == mj) return -1e18; // 必ず不採用になる値
            if (mi > mj) std::swap(mi, mj);
            return -two_opt_delta(tour, dist, mi, mj);
        },
        // commit: 選んだ近傍を反映する
        [&] { two_opt_apply(tour, mi, mj); },
        // on_best: 最良解を保存する
        [&] { best_tour = tour; });
    // #endregion usage

    const double best_len = tour_length(best_tour, dist);
    assert(std::abs(-res.best_score - best_len) < 1e-6); // 記録したスコアと実際の長さが一致する
    assert(best_len < init_len * 0.5);                   // ランダムな並びの半分より短くなる
    assert(res.accepted > 0 && res.accepted <= res.iterations);
    std::printf("test_sa: OK (length %.0f -> %.0f, %lld iterations)\n", init_len, best_len, res.iterations);
    return 0;
}
