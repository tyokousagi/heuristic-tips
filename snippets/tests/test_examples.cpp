// 解説記事に載せる使用例。記事からは #region の範囲を取り込む。
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <numeric>

#include "../index_set.hpp"
#include "../timer.hpp"
#include "../tsp.hpp"
#include "../xorshift.hpp"
#include "../zobrist.hpp"

int main() {
    const int n = 150;
    Xorshift rng(7);
    Timer timer;
    std::vector<double> xs(n), ys(n);
    for (int i = 0; i < n; ++i) {
        xs[i] = rng.next_double() * 1000;
        ys[i] = rng.next_double() * 1000;
    }
    const auto dist = [&](int a, int b) { return std::hypot(xs[a] - xs[b], ys[a] - ys[b]); };

    // #region greedy
    // 貪欲法: まだ訪れていない都市のうち、今いる都市から最も近い都市へ進む
    std::vector<int> tour = {0};
    std::vector<char> used(n, 0);
    used[0] = 1;
    while ((int)tour.size() < n) {
        const int cur = tour.back();
        int best = -1;
        for (int v = 0; v < n; ++v) {
            if (used[v]) continue;
            if (best < 0 || dist(cur, v) < dist(cur, best)) best = v;
        }
        used[best] = 1;
        tour.push_back(best);
    }
    // #endregion greedy
    const double greedy_len = tour_length(tour, dist);

    // #region hill
    // 山登り法: 2-opt で短くなる変更だけを採用する
    const double time_limit = timer.elapsed() + 0.2;
    double length = tour_length(tour, dist);
    long long iterations = 0;
    while (true) {
        if ((iterations & 255) == 0 && timer.elapsed() > time_limit) break;
        ++iterations;
        int i = rng.next_int(0, n), j = rng.next_int(0, n);
        if (i == j) continue;
        if (i > j) std::swap(i, j);
        const double delta = two_opt_delta(tour, dist, i, j);
        if (delta < 0) { // 最小化なので、増分が負なら改善
            two_opt_apply(tour, i, j);
            length += delta;
        }
    }
    // #endregion hill
    assert(std::abs(length - tour_length(tour, dist)) < 1e-6);
    assert(length < greedy_len);

    // #region zobrist
    // 5×5 の盤面に 3 種類の駒を置く。置くときも取り除くときも XOR で更新する
    const Zobrist zobrist(3, 25);
    uint64_t hash = 0;
    hash ^= zobrist(0, 7);  // 種類 0 の駒をマス 7 に置く
    hash ^= zobrist(2, 12); // 種類 2 の駒をマス 12 に置く
    hash ^= zobrist(0, 7);  // マス 7 の駒を取り除く
    hash ^= zobrist(0, 8);  // マス 8 に置き直す
    // #endregion zobrist
    assert(hash == (zobrist(2, 12) ^ zobrist(0, 8)));

    // #region index-set
    // まだ使っていない要素の集合から、ランダムに1つ取り出す
    IndexSet unused(n);
    for (int v = 0; v < n; ++v) unused.add(v);
    std::vector<int> order;
    while (!unused.empty()) {
        const int v = unused.random(rng);
        unused.remove(v);
        order.push_back(v);
    }
    // #endregion index-set
    std::sort(order.begin(), order.end());
    for (int v = 0; v < n; ++v) assert(order[v] == v);

    // #region param
    // パラメータを環境変数から読む。環境変数がなければ既定値を使うので、そのまま提出できる
    const auto param = [](const char* name, double default_value) {
        const char* s = std::getenv(name);
        return s ? std::atof(s) : default_value;
    };
    const double start_temp = param("START_TEMP", 30.0);
    const double end_temp = param("END_TEMP", 0.5);
    // #endregion param
    assert(start_temp > 0 && end_temp > 0);

    std::printf("test_examples: OK (greedy %.0f -> hill climbing %.0f)\n", greedy_len, length);
    return 0;
}
