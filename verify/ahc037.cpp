// AHC037 Soda の検証。
// 2点 (x1,y1), (x2,y2) を (min x, min y) にまとめる操作を繰り返して、原点を根とする木を作る。
// まとめる組の選び方を2通り比べる（解説スライド AHC037 の貪欲法1・2）。
//   nearest : |x1−x2| + |y1−y2| が最小の組からまとめる
//   minsum  : min(x1,x2) + min(y1,y2) が最大の組からまとめる
// 得点は round(10^6 × N × L / (1 + C))。C は木の辺の長さ（マンハッタン距離）の合計。
// 使い方: g++ -std=c++20 -O2 ahc037.cpp -o ahc037 && ./ahc037 [ケース数=20]
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <queue>
#include <random>
#include <set>
#include <tuple>
#include <vector>

constexpr int N = 1000;

struct P {
    long long x, y;
};

std::vector<P> generate(uint64_t seed) {
    std::mt19937_64 rng(seed);
    auto gen = [&]() {
        std::set<long long> used{0};
        std::vector<long long> a{0};
        std::uniform_int_distribution<long long> d(1, 999'999'999);
        while ((int)a.size() < N) {
            long long v = d(rng);
            if (used.insert(v).second) a.push_back(v);
        }
        std::shuffle(a.begin(), a.end(), rng);
        return a;
    };
    auto a = gen(), b = gen();
    std::vector<P> p(N);
    for (int i = 0; i < N; i++) p[i] = {a[i], b[i]};
    return p;
}

// key(i, j) が最小の組から順にまとめ、木の辺の長さの合計を返す
template <class Key>
long long merge_all(std::vector<P> pts, Key key) {
    std::vector<bool> alive(pts.size(), true);
    using T = std::tuple<long long, int, int>;
    std::priority_queue<T, std::vector<T>, std::greater<T>> pq;
    for (int i = 0; i < (int)pts.size(); i++)
        for (int j = i + 1; j < (int)pts.size(); j++) pq.push({key(pts[i], pts[j]), i, j});
    long long cost = 0;
    int remain = pts.size();
    while (remain > 1) {
        auto [k, i, j] = pq.top();
        pq.pop();
        if (!alive[i] || !alive[j]) continue;
        P m{std::min(pts[i].x, pts[j].x), std::min(pts[i].y, pts[j].y)};
        cost += (pts[i].x - m.x) + (pts[i].y - m.y) + (pts[j].x - m.x) + (pts[j].y - m.y);
        alive[i] = alive[j] = false;
        int id = pts.size();
        pts.push_back(m);
        alive.push_back(true);
        for (int t = 0; t < id; t++)
            if (alive[t]) pq.push({key(pts[t], m), t, id});
        remain--;
    }
    for (int t = 0; t < (int)pts.size(); t++)
        if (alive[t]) cost += pts[t].x + pts[t].y;  // 最後に原点からつなぐ（実際は (0,0) になる）
    return cost;
}

int main(int argc, char** argv) {
    int cases = argc > 1 ? std::atoi(argv[1]) : 20;
    double s1 = 0, s2 = 0;
    for (int s = 0; s < cases; s++) {
        auto p = generate(2000 + s);
        long long L = 0;
        for (auto& q : p) L = std::max({L, q.x, q.y});
        long long c1 = merge_all(p, [](const P& a, const P& b) { return std::llabs(a.x - b.x) + std::llabs(a.y - b.y); });
        long long c2 = merge_all(p, [](const P& a, const P& b) { return -(std::min(a.x, b.x) + std::min(a.y, b.y)); });
        double sc1 = 1e6 * N * (double)L / (1 + c1), sc2 = 1e6 * N * (double)L / (1 + c2);
        s1 += sc1;
        s2 += sc2;
        std::fprintf(stderr, "case %d: nearest=%.0f minsum=%.0f\n", s, sc1, sc2);
    }
    std::printf("cases=%d\nnearest 平均 %.0f\nminsum  平均 %.0f\n", cases, s1 / cases, s2 / cases);
}
