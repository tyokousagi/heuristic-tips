// https://tyokousagi.github.io/heuristic-tips/snippets/#tsp
#pragma once
#include <algorithm>
#include <vector>

// 巡回路の近傍。tour は都市の並びで、最後の都市から最初の都市へ戻る。
// d(a, b) は都市 a, b の距離を返す関数。距離は対称（d(a, b) == d(b, a)）とする。

template <class Dist>
double tour_length(const std::vector<int>& tour, const Dist& d) {
    const int n = (int)tour.size();
    double len = 0;
    for (int i = 0; i < n; ++i) len += d(tour[i], tour[(i + 1) % n]);
    return len;
}

// 2-opt: tour[i+1 .. j] を反転したときの長さの増分。0 <= i < j < n。
template <class Dist>
double two_opt_delta(const std::vector<int>& tour, const Dist& d, int i, int j) {
    const int n = (int)tour.size();
    const int a = tour[i], b = tour[i + 1], c = tour[j], e = tour[(j + 1) % n];
    return d(a, c) + d(b, e) - d(a, b) - d(c, e);
}

inline void two_opt_apply(std::vector<int>& tour, int i, int j) {
    std::reverse(tour.begin() + i + 1, tour.begin() + j + 1);
}

// Or-opt: 区間 tour[i .. i+len-1] を抜き、tour[j] と tour[j+1] のあいだに入れたときの長さの増分。
// rev が true なら区間を逆向きにして入れる。
// 条件: 1 <= i, i + len <= n, j は [0, i-2] か [i+len, n-1] の範囲。
template <class Dist>
double or_opt_delta(const std::vector<int>& tour, const Dist& d, int i, int len, int j, bool rev) {
    const int n = (int)tour.size();
    const int p = tour[i - 1], s = tour[i], t = tour[i + len - 1], q = tour[(i + len) % n];
    const int u = tour[j], v = tour[(j + 1) % n];
    const double removed = d(p, s) + d(t, q) + d(u, v);
    const double added = d(p, q) + (rev ? d(u, t) + d(s, v) : d(u, s) + d(t, v));
    return added - removed;
}

inline void or_opt_apply(std::vector<int>& tour, int i, int len, int j, bool rev) {
    if (j >= i + len) {
        // 区間を後ろへ動かす。動かしたあとの区間は [j-len+1, j]
        std::rotate(tour.begin() + i, tour.begin() + i + len, tour.begin() + j + 1);
        if (rev) std::reverse(tour.begin() + j - len + 1, tour.begin() + j + 1);
    } else {
        // 区間を前へ動かす。動かしたあとの区間は [j+1, j+len]
        std::rotate(tour.begin() + j + 1, tour.begin() + i, tour.begin() + i + len);
        if (rev) std::reverse(tour.begin() + j + 1, tour.begin() + j + 1 + len);
    }
}
