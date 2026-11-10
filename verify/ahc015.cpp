// AHC015 Halloween Candy の検証。
// 10×10 の箱に100個のキャンディーが空きマスへ一様ランダムに入る。入るたびに前後左右のどれかへ傾ける。
// 得点は 10^6 × Σ(連結成分の大きさ^2) / Σ(味ごとの個数^2)。
//   random   : ランダムに傾ける
//   greedy   : 傾けた直後の Σ(大きさ^2) が最大の向き（1手先だけ見る）
//   mc_rand  : 4方向それぞれ、残りをランダムに傾けるプレイアウトの平均で選ぶ
//   mc_greedy: 残りを greedy で傾けるプレイアウトの平均で選ぶ（強い方策をプレイアウトに使う）
// 各向きのプレイアウトは同じ乱数列（キャンディーの入る位置）を使う。
// 使い方: g++ -std=c++20 -O2 ahc015.cpp -o ahc015 && ./ahc015 [ケース数=20] [プレイアウト数=8]
#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>

constexpr int W = 10;
using Board = std::array<int8_t, W * W>;  // 0 は空き、1..3 は味

void tilt(Board& b, int dir) {  // 0:F(上) 1:B(下) 2:L(左) 3:R(右)
    for (int line = 0; line < W; line++) {
        int8_t buf[W];
        int n = 0;
        for (int k = 0; k < W; k++) {
            int idx = (dir == 0) ? k * W + line : (dir == 1) ? (W - 1 - k) * W + line : (dir == 2) ? line * W + k : line * W + (W - 1 - k);
            if (b[idx]) buf[n++] = b[idx];
        }
        for (int k = 0; k < W; k++) {
            int idx = (dir == 0) ? k * W + line : (dir == 1) ? (W - 1 - k) * W + line : (dir == 2) ? line * W + k : line * W + (W - 1 - k);
            b[idx] = k < n ? buf[k] : 0;
        }
    }
}

int sq_sum(const Board& b) {  // Σ 連結成分の大きさ^2
    std::array<bool, W * W> seen{};
    int total = 0, stack[W * W];
    for (int s = 0; s < W * W; s++) {
        if (!b[s] || seen[s]) continue;
        int top = 0, cnt = 0;
        stack[top++] = s;
        seen[s] = true;
        while (top) {
            int v = stack[--top];
            cnt++;
            int i = v / W, j = v % W;
            const int ni[4] = {i - 1, i + 1, i, i}, nj[4] = {j, j, j - 1, j + 1};
            for (int d = 0; d < 4; d++) {
                if (ni[d] < 0 || ni[d] >= W || nj[d] < 0 || nj[d] >= W) continue;
                int u = ni[d] * W + nj[d];
                if (!seen[u] && b[u] == b[v]) { seen[u] = true; stack[top++] = u; }
            }
        }
        total += cnt * cnt;
    }
    return total;
}

void put(Board& b, int p, int flavor) {  // p 番目（0始まり）の空きマスに置く
    for (int v = 0; v < W * W; v++)
        if (!b[v] && p-- == 0) { b[v] = flavor; return; }
}

int greedy_dir(const Board& b) {
    int best = 0, bv = -1;
    for (int d = 0; d < 4; d++) {
        Board c = b;
        tilt(c, d);
        int v = sq_sum(c);
        if (v > bv) { bv = v; best = d; }
    }
    return best;
}

// t 個目を置いた直後の盤面 b から、残りを方策 policy で進めたときの最終 Σ(大きさ^2)
int playout(Board b, const std::vector<int>& f, int t, std::mt19937_64& rng, bool greedy_policy) {
    for (int s = t + 1; s < 100; s++) {
        put(b, rng() % (100 - s), f[s]);
        int d = greedy_policy ? greedy_dir(b) : (int)(rng() % 4);
        tilt(b, d);
    }
    return sq_sum(b);
}

int main(int argc, char** argv) {
    int cases = argc > 1 ? std::atoi(argv[1]) : 20;
    int R = argc > 2 ? std::atoi(argv[2]) : 8;
    const char* names[] = {"random", "greedy", "mc_rand", "mc_greedy"};
    double sum[4] = {};
    for (int cs = 0; cs < cases; cs++) {
        std::mt19937_64 gen(5000 + cs);
        std::vector<int> f(100), p(100);
        for (int t = 0; t < 100; t++) f[t] = gen() % 3 + 1;
        for (int t = 0; t < 100; t++) p[t] = gen() % (100 - t);
        int d2 = 0;
        int cnt[4] = {};
        for (int t = 0; t < 100; t++) cnt[f[t]]++;
        for (int k = 1; k <= 3; k++) d2 += cnt[k] * cnt[k];
        for (int strat = 0; strat < 4; strat++) {
            Board b{};
            std::mt19937_64 rng(cs * 7 + strat);
            for (int t = 0; t < 100; t++) {
                put(b, p[t], f[t]);
                int dir = 0;
                if (strat == 0) dir = rng() % 4;
                else if (strat == 1) dir = greedy_dir(b);
                else {
                    double best = -1;
                    uint64_t base = rng();
                    for (int d = 0; d < 4; d++) {
                        Board c = b;
                        tilt(c, d);
                        double tot = 0;
                        for (int r = 0; r < R; r++) {
                            std::mt19937_64 pr(base + r);  // 4方向で同じ乱数列を使う
                            tot += playout(c, f, t, pr, strat == 3);
                        }
                        if (tot > best) { best = tot; dir = d; }
                    }
                }
                tilt(b, dir);
            }
            double score = 1e6 * sq_sum(b) / d2;
            sum[strat] += score;
        }
        std::fprintf(stderr, "case %d done\n", cs);
    }
    std::printf("cases=%d playouts=%d\n", cases, R);
    for (int k = 0; k < 4; k++) std::printf("%-9s 平均 %.0f\n", names[k], sum[k] / cases);
}
