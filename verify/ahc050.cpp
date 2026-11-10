// AHC050 Gamble on Ice の検証。
// 問題文の入力生成方法で盤面を作り、岩を置く順番 P の得点（賞金の期待値の割合）を確率DPで計算する。
// 解説（physics0523）の段階的な解法を同じケースで比べる。
//   row      : 行優先で置く（解法1）
//   shuffle  : 行優先の列をシャッフルする（解法2）
//   greedy   : 毎ターン、ロボットがいる確率が最小のマスに置く（解法3）
//   greedyR  : greedy を乱数を変えて R 回やり、最良を取る（解法4）
//   tiebreak : 確率 − ε·min(p_i, q_j) で選ぶ（p, q はランダムな順列。解法5）
//   concentrate : 確率を少数のマスに集める（解法6・7。1回だけ実行）
// 使い方: g++ -std=c++20 -O2 ahc050.cpp -o ahc050 && ./ahc050 [ケース数=30] [R=8]
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <numeric>
#include <random>
#include <vector>

constexpr int N = 40;
using Grid = std::array<std::array<bool, N>, N>;  // true なら岩

struct Case {
    Grid rock{};
    int empty = 0;
};

Case generate(uint64_t seed) {
    std::mt19937_64 rng(seed);
    Case c;
    int lo = N * N / 10, hi = N * N / 4;
    int m = std::uniform_int_distribution<int>(lo, hi)(rng);
    std::vector<int> cells(N * N);
    std::iota(cells.begin(), cells.end(), 0);
    std::shuffle(cells.begin(), cells.end(), rng);
    for (int k = 0; k < m; k++) c.rock[cells[k] / N][cells[k] % N] = true;
    c.empty = N * N - m;
    return c;
}

// 1回の移動: 各マスの確率を4方向の滑走先へ 1/4 ずつ配る。O(N^2)。
void move(const Grid& rock, const std::vector<double>& p, std::vector<double>& q) {
    std::fill(q.begin(), q.end(), 0.0);
    for (int i = 0; i < N; i++) {
        // 左へ: 左から見て、直前の岩の右隣が行き先
        int dest = 0;
        for (int j = 0; j < N; j++) {
            if (rock[i][j]) { dest = j + 1; continue; }
            q[i * N + dest] += p[i * N + j] * 0.25;
        }
        dest = N - 1;
        for (int j = N - 1; j >= 0; j--) {
            if (rock[i][j]) { dest = j - 1; continue; }
            q[i * N + dest] += p[i * N + j] * 0.25;
        }
    }
    for (int j = 0; j < N; j++) {
        int dest = 0;
        for (int i = 0; i < N; i++) {
            if (rock[i][j]) { dest = i + 1; continue; }
            q[dest * N + j] += p[i * N + j] * 0.25;
        }
        dest = N - 1;
        for (int i = N - 1; i >= 0; i--) {
            if (rock[i][j]) { dest = i - 1; continue; }
            q[dest * N + j] += p[i * N + j] * 0.25;
        }
    }
}

// 列 P の得点（10^6 × E / (空きマス数 − 1)）
double score(const Case& c, const std::vector<int>& order) {
    Grid rock = c.rock;
    std::vector<double> p(N * N, 0.0), q(N * N);
    for (int v = 0; v < N * N; v++)
        if (!rock[v / N][v % N]) p[v] = 1.0 / c.empty;
    double e = 0;
    for (int v : order) {
        move(rock, p, q);
        std::swap(p, q);
        p[v] = 0;  // 置いたマスにいた分は潰れる
        rock[v / N][v % N] = true;
        double alive = 0;
        for (double x : p) alive += x;
        e += alive;
    }
    return 1e6 * e / (c.empty - 1);
}

std::vector<int> row_order(const Case& c) {
    std::vector<int> o;
    for (int v = 0; v < N * N; v++)
        if (!c.rock[v / N][v % N]) o.push_back(v);
    return o;
}

// 確率最小の貪欲。tie_eps > 0 なら −tie_eps·min(pi[i], qj[j]) を足して直線的に置く。
std::vector<int> greedy(const Case& c, std::mt19937_64& rng, double tie_eps) {
    Grid rock = c.rock;
    std::vector<double> p(N * N, 0.0), q(N * N);
    for (int v = 0; v < N * N; v++)
        if (!rock[v / N][v % N]) p[v] = 1.0 / c.empty;
    std::vector<int> pi(N), qj(N);
    std::iota(pi.begin(), pi.end(), 0);
    std::iota(qj.begin(), qj.end(), 0);
    std::shuffle(pi.begin(), pi.end(), rng);
    std::shuffle(qj.begin(), qj.end(), rng);
    std::uniform_real_distribution<double> noise(0.0, 1e-12);
    std::vector<int> order;
    for (int step = 0; step < c.empty; step++) {
        move(rock, p, q);
        std::swap(p, q);
        int best = -1;
        double bv = 1e18;
        for (int v = 0; v < N * N; v++) {
            if (rock[v / N][v % N]) continue;
            double val = p[v] + noise(rng);
            if (tie_eps > 0) val -= tie_eps * std::min(pi[v / N], qj[v % N]);
            if (val < bv) { bv = val; best = v; }
        }
        order.push_back(best);
        p[best] = 0;
        rock[best / N][best % N] = true;
    }
    return order;
}

// 確率を少数のマスに集める貪欲（解法6・7）。
// 置く候補は、確率が最小に近い K 個。候補ごとに「置いたあと、各マスから何回の滑走で
// 集めたいマス（いま確率が最大に近いマス）に着くか」を逆向きBFSで求め、
// 確率で重みづけした f(距離) の和が最小の候補に置く。f(x) = 4^x − 1（解法7）。
std::vector<int> concentrate(const Case& c, std::mt19937_64& rng, int K) {
    Grid rock = c.rock;
    std::vector<double> p(N * N, 0.0), q(N * N);
    for (int v = 0; v < N * N; v++)
        if (!rock[v / N][v % N]) p[v] = 1.0 / c.empty;
    std::uniform_real_distribution<double> noise(0.0, 1e-12);
    std::vector<double> fval(64);
    for (int d = 0; d < 64; d++) fval[d] = std::min(std::pow(4.0, d) - 1, 1e18);
    std::vector<int> order, dist(N * N), dest(4 * N * N), radj_start(N * N + 1), radj(4 * N * N), que(N * N);
    for (int step = 0; step < c.empty; step++) {
        move(rock, p, q);
        std::swap(p, q);
        // 候補: 確率 + 微小な乱数が小さい K 個（最小より 1e-6 以上大きいものは除く）
        std::vector<std::pair<double, int>> cand;
        double pmin = 1e18, pmax = 0;
        for (int v = 0; v < N * N; v++) {
            if (rock[v / N][v % N]) continue;
            cand.push_back({p[v] + noise(rng), v});
            pmin = std::min(pmin, p[v]);
            pmax = std::max(pmax, p[v]);
        }
        int k = std::min<int>(K, cand.size());
        std::partial_sort(cand.begin(), cand.begin() + k, cand.end());
        int best = cand[0].second;
        double bestE = 1e300;
        for (int t = 0; t < k; t++) {
            int v = cand[t].second;
            if (p[v] > pmin + 1e-6) break;
            rock[v / N][v % N] = true;
            // 滑走先を作る
            for (int i = 0; i < N; i++) {
                int d = 0;
                for (int j = 0; j < N; j++) { if (rock[i][j]) d = j + 1; else dest[(i * N + j) * 4 + 0] = i * N + d; }
                d = N - 1;
                for (int j = N - 1; j >= 0; j--) { if (rock[i][j]) d = j - 1; else dest[(i * N + j) * 4 + 1] = i * N + d; }
            }
            for (int j = 0; j < N; j++) {
                int d = 0;
                for (int i = 0; i < N; i++) { if (rock[i][j]) d = i + 1; else dest[(i * N + j) * 4 + 2] = d * N + j; }
                d = N - 1;
                for (int i = N - 1; i >= 0; i--) { if (rock[i][j]) d = i - 1; else dest[(i * N + j) * 4 + 3] = d * N + j; }
            }
            // 逆向きの辺
            std::fill(radj_start.begin(), radj_start.end(), 0);
            for (int x = 0; x < N * N; x++) {
                if (rock[x / N][x % N]) continue;
                for (int dir = 0; dir < 4; dir++) radj_start[dest[x * 4 + dir] + 1]++;
            }
            for (int x = 0; x < N * N; x++) radj_start[x + 1] += radj_start[x];
            std::vector<int> fill(radj_start.begin(), radj_start.end() - 1);
            for (int x = 0; x < N * N; x++) {
                if (rock[x / N][x % N]) continue;
                for (int dir = 0; dir < 4; dir++) radj[fill[dest[x * 4 + dir]]++] = x;
            }
            // 集めたいマス（確率が最大に近いマス）から逆向きBFS
            std::fill(dist.begin(), dist.end(), -1);
            int head = 0, tail = 0;
            for (int x = 0; x < N * N; x++)
                if (!rock[x / N][x % N] && x != v && p[x] >= pmax - 1e-9) { dist[x] = 0; que[tail++] = x; }
            while (head < tail) {
                int x = que[head++];
                for (int e = radj_start[x]; e < radj_start[x + 1]; e++) {
                    int y = radj[e];
                    if (dist[y] < 0) { dist[y] = dist[x] + 1; que[tail++] = y; }
                }
            }
            double E = 0;
            for (int x = 0; x < N * N; x++) {
                if (rock[x / N][x % N] || p[x] == 0) continue;
                int d = dist[x] < 0 ? 63 : std::min(dist[x], 63);
                E += p[x] * fval[d];
            }
            rock[v / N][v % N] = false;
            if (E < bestE) { bestE = E; best = v; }
        }
        order.push_back(best);
        p[best] = 0;
        rock[best / N][best % N] = true;
    }
    return order;
}

int main(int argc, char** argv) {
    int cases = argc > 1 ? std::atoi(argv[1]) : 30;
    int reps = argc > 2 ? std::atoi(argv[2]) : 8;
    const char* names[] = {"row", "shuffle", "greedy", "greedyR", "tiebreak", "concentrate"};
    std::vector<double> sum(6, 0.0);
    for (int s = 0; s < cases; s++) {
        Case c = generate(1000 + s);
        std::mt19937_64 rng(s);
        std::vector<double> sc(6);
        auto o = row_order(c);
        sc[0] = score(c, o);
        std::shuffle(o.begin(), o.end(), rng);
        sc[1] = score(c, o);
        sc[2] = score(c, greedy(c, rng, 0));
        sc[3] = sc[2];
        for (int r = 1; r < reps; r++) sc[3] = std::max(sc[3], score(c, greedy(c, rng, 0)));
        sc[4] = 0;
        for (int r = 0; r < reps; r++) sc[4] = std::max(sc[4], score(c, greedy(c, rng, 1e-6)));
        sc[5] = score(c, concentrate(c, rng, 10));
        for (int k = 0; k < 6; k++) sum[k] += sc[k];
        std::fprintf(stderr, "case %d: M=%d row=%.0f shuffle=%.0f greedy=%.0f greedyR=%.0f tiebreak=%.0f concentrate=%.0f\n",
                     s, N * N - c.empty, sc[0], sc[1], sc[2], sc[3], sc[4], sc[5]);
    }
    std::printf("cases=%d reps=%d\n", cases, reps);
    for (int k = 0; k < 6; k++) std::printf("%-9s 平均 %8.0f（満点比 %.2f%%）\n", names[k], sum[k] / cases, sum[k] / cases / 1e4);
}
