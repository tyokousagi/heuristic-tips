// AHC032 Mod Stamp の検証。
// 盤面 9×9、スタンプ20種、81回まで。各マスの値 mod 998244353 の和を最大化する。
//   none   : 何も押さない
//   sa     : 81回ぶんの（スタンプ, 位置）を焼きなます。近傍は1回ぶんの変更
//   greedy : 左上から1マスずつ確定させる貪欲（解説スライドの貪欲法。右端・下端は3マス、右下は9マスまとめて）
//   beam   : 同じ確定順でビームサーチ（評価 = 確定した値の和 + 0.06·MOD·(1−進行度)·残り回数）
// 使い方: g++ -std=c++20 -O2 ahc032.cpp -o ahc032 && ./ahc032 [ケース数=20] [ビーム幅=300]
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>

constexpr int64_t MOD = 998244353;
constexpr int N = 9, M = 20, K = 81;

struct Case {
    std::array<int64_t, N * N> a;
    std::array<std::array<int64_t, 9>, M> s;
};

Case generate(uint64_t seed) {
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<int64_t> d(0, MOD - 1);
    Case c;
    for (auto& v : c.a) v = d(rng);
    for (auto& st : c.s)
        for (auto& v : st) v = d(rng);
    return c;
}

// 重複組合せ（スタンプの多重集合）を k 個まで列挙し、3×3 の和を前計算しておく
struct Combo {
    std::array<int64_t, 9> v;  // mod 済み
    int k;
};
std::vector<Combo> combos(const Case& c, int maxk) {
    std::vector<Combo> out;
    std::vector<int> cur;
    auto rec = [&](auto&& self, int from) -> void {
        Combo cb{};
        for (int m : cur)
            for (int t = 0; t < 9; t++) cb.v[t] = (cb.v[t] + c.s[m][t]) % MOD;
        cb.k = cur.size();
        out.push_back(cb);
        if ((int)cur.size() == maxk) return;
        for (int m = from; m < M; m++) {
            cur.push_back(m);
            self(self, m);
            cur.pop_back();
        }
    };
    rec(rec, 0);
    return out;
}

// 位置 (p,q) で確定するマスの一覧（盤面の添字）
std::vector<int> fixed_cells(int p, int q) {
    std::vector<int> cells;
    int h = p == 6 ? 3 : 1, w = q == 6 ? 3 : 1;
    for (int i = 0; i < h; i++)
        for (int j = 0; j < w; j++) cells.push_back((p + i) * N + (q + j));
    return cells;
}

struct State {
    std::array<int64_t, N * N> b;
    int used = 0;
    int64_t fixed_sum = 0;
    double eval = 0;
};

// ビーム幅 w のビームサーチ（w=1 なら貪欲）。maxk[位置の種類] は1か所に押す最大回数。
int64_t beam(const Case& c, int width, int k_single, int k_edge, int k_corner) {
    auto c1 = combos(c, k_single), c3 = combos(c, k_edge), c9 = combos(c, k_corner);
    std::vector<State> cur(1);
    cur[0].b = c.a;
    int step = 0;
    for (int p = 0; p <= 6; p++) {
        for (int q = 0; q <= 6; q++, step++) {
            const auto& cs = (p == 6 && q == 6) ? c9 : (p == 6 || q == 6) ? c3 : c1;
            auto cells = fixed_cells(p, q);
            double progress = (step + 1) / 49.0;
            std::vector<State> next;
            // 各状態から、確定するマスの値の和が大きい候補を上位 width 個まで作る
            for (const auto& st : cur) {
                std::vector<std::pair<int64_t, int>> cand;
                for (int idx = 0; idx < (int)cs.size(); idx++) {
                    const auto& cb = cs[idx];
                    if (st.used + cb.k > K) continue;
                    int64_t sum = 0;
                    for (int cell : cells) {
                        int i = cell / N - p, j = cell % N - q;
                        sum += (st.b[cell] + cb.v[i * 3 + j]) % MOD;
                    }
                    cand.push_back({sum, idx});
                }
                int take = std::min<int>(width, cand.size());
                std::partial_sort(cand.begin(), cand.begin() + take, cand.end(), std::greater<>());
                for (int t = 0; t < take; t++) {
                    State ns = st;
                    const auto& cb = cs[cand[t].second];
                    for (int i = 0; i < 3; i++)
                        for (int j = 0; j < 3; j++) ns.b[(p + i) * N + q + j] = (ns.b[(p + i) * N + q + j] + cb.v[i * 3 + j]) % MOD;
                    ns.used += cb.k;
                    ns.fixed_sum += cand[t].first;
                    ns.eval = ns.fixed_sum + 0.06 * MOD * (1 - progress) * (K - ns.used);
                    next.push_back(ns);
                }
            }
            int keep = std::min<int>(width, next.size());
            std::partial_sort(next.begin(), next.begin() + keep, next.end(),
                              [](const State& x, const State& y) { return x.eval > y.eval; });
            next.resize(keep);
            cur = std::move(next);
        }
    }
    int64_t best = 0;
    for (const auto& st : cur) {
        int64_t s = 0;
        for (auto v : st.b) s += v;
        best = std::max(best, s);
    }
    return best;
}

int64_t anneal(const Case& c, uint64_t seed, int iters) {
    std::mt19937_64 rng(seed);
    // 操作 t は (スタンプ m, 位置 pos) か、使わない（m = -1）
    std::vector<int> om(K, -1), op(K, 0);
    std::array<int64_t, N * N> b = c.a;
    auto apply = [&](int m, int pos, int sign) {
        int p = pos / 7, q = pos % 7;
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++) {
                int64_t& x = b[(p + i) * N + q + j];
                x = (x + (sign > 0 ? c.s[m][i * 3 + j] : MOD - c.s[m][i * 3 + j])) % MOD;
            }
    };
    auto total = [&]() { int64_t s = 0; for (auto v : b) s += v; return s; };
    int64_t score = total(), best = score;
    double t0 = 2e8, t1 = 1e6;
    for (int it = 0; it < iters; it++) {
        double temp = t0 * std::pow(t1 / t0, (double)it / iters);
        int t = rng() % K;
        int nm = (rng() % 4 == 0) ? -1 : (int)(rng() % M), np = rng() % 49;
        int pm = om[t], pp = op[t];
        int64_t before = 0, after = 0;
        // 影響するマスだけ差分で計算する
        std::array<int, 18> touched;
        int nt = 0;
        auto add_cells = [&](int pos) { for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) touched[nt++] = (pos / 7 + i) * N + pos % 7 + j; };
        if (pm >= 0) add_cells(pp);
        if (nm >= 0) add_cells(np);
        std::sort(touched.begin(), touched.begin() + nt);
        nt = std::unique(touched.begin(), touched.begin() + nt) - touched.begin();
        for (int x = 0; x < nt; x++) before += b[touched[x]];
        if (pm >= 0) apply(pm, pp, -1);
        if (nm >= 0) apply(nm, np, +1);
        for (int x = 0; x < nt; x++) after += b[touched[x]];
        int64_t delta = after - before;
        if (delta >= 0 || std::exp(delta / temp) > std::uniform_real_distribution<double>(0, 1)(rng)) {
            om[t] = nm;
            op[t] = np;
            score += delta;
            best = std::max(best, score);
        } else {
            if (nm >= 0) apply(nm, np, -1);
            if (pm >= 0) apply(pm, pp, +1);
        }
    }
    return best;
}

int main(int argc, char** argv) {
    int cases = argc > 1 ? std::atoi(argv[1]) : 20;
    int width = argc > 2 ? std::atoi(argv[2]) : 300;
    double sn = 0, ss = 0, sg = 0, sb = 0;
    for (int s = 0; s < cases; s++) {
        Case c = generate(3000 + s);
        int64_t none = 0;
        for (auto v : c.a) none += v;
        int64_t sa = anneal(c, s, 3'000'000);
        int64_t gr = beam(c, 1, 1, 3, 5);   // 内側は1回、端は3回、右下は5回まで（貪欲）
        int64_t bm = beam(c, width, 3, 3, 5);  // 内側も3回まで押せるようにしたビームサーチ
        sn += none; ss += sa; sg += gr; sb += bm;
        std::fprintf(stderr, "case %d: none=%lld sa=%lld greedy=%lld beam=%lld\n", s, (long long)none, (long long)sa, (long long)gr, (long long)bm);
    }
    auto pr = [&](const char* name, double v) { std::printf("%-7s 平均 %.4e（理論上限 81·MOD の %.2f%%）\n", name, v / cases, 100.0 * v / cases / (81.0 * MOD)); };
    std::printf("cases=%d width=%d\n", cases, width);
    pr("none", sn); pr("sa", ss); pr("greedy", sg); pr("beam", sb);
}
