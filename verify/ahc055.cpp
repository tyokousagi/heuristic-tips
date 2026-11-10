// AHC055 Weakpoint の検証。
// 宝箱 N=200。素手は1、武器 w は箱 b に A[w][b]（耐久 C[w] 回）。開けた箱の武器が使えるようになる。
// 得点は ΣH − T + 1（T は攻撃回数）。tomerun の解説（AHCラジオ）の段階を比べる。
//   seq      : 箱を番号順に開け、直前に手に入れた武器を使い、足りないぶんは素手（最初の解法）
//   maxdmg   : 武器があれば（手持ちの武器, 未開封の箱）で与ダメージ min(A,H) 最大の攻撃をする。
//              武器がなければ H 最小の箱を素手で開ける。最初の箱は候補 S 通りから最良を取る
//   precious : 評価を min(A,H)^2 / Σ_w A[w][b]^2·残り耐久[w] に変える（貴重な攻撃を優先）。
//              さらに H が5以下の箱は先に素手で開ける
// 使い方: g++ -std=c++20 -O2 ahc055.cpp -o ahc055 && ./ahc055 [ケース数=10] [最初の箱の候補数=8]
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <numeric>
#include <random>
#include <vector>

constexpr int N = 200;
struct Case {
    std::vector<int> H, C;
    std::vector<std::vector<int>> A;
};

Case generate(uint64_t seed) {
    std::mt19937_64 rng(seed);
    Case c;
    c.H.resize(N); c.C.resize(N); c.A.assign(N, std::vector<int>(N));
    for (int i = 0; i < N; i++) c.H[i] = std::uniform_int_distribution<int>(100, 500)(rng);
    for (int i = 0; i < N; i++) c.C[i] = std::uniform_int_distribution<int>(1, 6)(rng);
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) c.A[i][j] = (int)std::lround(500.0 / std::uniform_real_distribution<double>(1.0, 500.0)(rng));
    return c;
}

long long seq(const Case& c) {
    long long T = c.H[0];
    for (int b = 1; b < N; b++) {
        int h = c.H[b], w = b - 1, dur = c.C[w];
        while (h > 0 && dur > 0) { h -= c.A[w][b]; dur--; T++; }
        if (h > 0) T += h;
    }
    return T;
}

// mode 0: maxdmg, 1: precious
long long greedy(const Case& c, int first, int mode) {
    std::vector<int> h(c.H), dur(c.C);
    std::vector<bool> open(N, false), have(N, false);
    // precious 用: 箱 b の分母 Σ_w A[w][b]^2·残り耐久[w]
    std::vector<double> den(N, 0);
    for (int w = 0; w < N; w++)
        for (int b = 0; b < N; b++) den[b] += (double)c.A[w][b] * c.A[w][b] * dur[w];
    long long T = 0;
    int opened = 0;
    auto open_box = [&](int b) { open[b] = true; have[b] = true; opened++; };
    T += h[first];
    open_box(first);
    while (opened < N) {
        if (mode == 1) {
            // H が5以下の箱は素手で開ける
            bool done = false;
            for (int b = 0; b < N; b++)
                if (!open[b] && h[b] <= 5) { T += h[b]; open_box(b); done = true; }
            if (done) continue;
        }
        int bw = -1, bb = -1;
        double bv = -1;
        for (int w = 0; w < N; w++) {
            if (!have[w] || dur[w] == 0) continue;
            for (int b = 0; b < N; b++) {
                if (open[b]) continue;
                double dmg = std::min(c.A[w][b], h[b]);
                double v = mode == 0 ? dmg : dmg * dmg / den[b];
                if (v > bv) { bv = v; bw = w; bb = b; }
            }
        }
        if (bw < 0) {
            int b = -1;
            for (int k = 0; k < N; k++)
                if (!open[k] && (b < 0 || h[k] < h[b])) b = k;
            T += h[b];
            open_box(b);
            continue;
        }
        T++;
        h[bb] -= c.A[bw][bb];
        dur[bw]--;
        for (int b = 0; b < N; b++) den[b] -= (double)c.A[bw][b] * c.A[bw][b];
        if (h[bb] <= 0) open_box(bb);
    }
    return T;
}

int main(int argc, char** argv) {
    int cases = argc > 1 ? std::atoi(argv[1]) : 10;
    int starts = argc > 2 ? std::atoi(argv[2]) : 8;
    double s[3] = {};
    for (int k = 0; k < cases; k++) {
        Case c = generate(9000 + k);
        long long sumH = std::accumulate(c.H.begin(), c.H.end(), 0LL);
        // 最初の箱の候補: H が小さい順に starts 個
        std::vector<int> ord(N);
        std::iota(ord.begin(), ord.end(), 0);
        std::sort(ord.begin(), ord.end(), [&](int x, int y) { return c.H[x] < c.H[y]; });
        long long best0 = 1LL << 60, best1 = 1LL << 60;
        for (int t = 0; t < starts; t++) {
            best0 = std::min(best0, greedy(c, ord[t], 0));
            best1 = std::min(best1, greedy(c, ord[t], 1));
        }
        s[0] += sumH - seq(c) + 1;
        s[1] += sumH - best0 + 1;
        s[2] += sumH - best1 + 1;
        std::fprintf(stderr, "case %d: ΣH=%lld seq=%lld maxdmg=%lld precious=%lld\n", k, sumH, sumH - seq(c) + 1, sumH - best0 + 1, sumH - best1 + 1);
    }
    const char* names[] = {"seq", "maxdmg", "precious"};
    std::printf("cases=%d starts=%d\n", cases, starts);
    for (int m = 0; m < 3; m++) std::printf("%-9s 平均 %.0f\n", names[m], s[m] / cases);
}
