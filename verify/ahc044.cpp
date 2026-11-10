// AHC044 Cleaning Up の検証。
// 社員 N=100 の当番の遷移 a_i, b_i を焼きなます。評価は L=500000 週のシミュレーション。
// 同じ実行時間で次の3つを比べる。
//   full     : 毎回 L 週を愚直にシミュレーションする。近傍は (a_i か b_i) をランダムな社員に変える
//   multires : 週数を L/128 から倍々に伸ばし、前の段の解を次の段の初期解にする（terry_u16 の方式）
//   biased   : multires に加えて、近傍を誤差に寄せる。行き先は当番が足りない社員ほど選ばれやすく、
//              変える辺は当番が多すぎる社員へ向かう辺ほど選ばれやすくする
// 得点は 10^6 − Σ|t_i − T_i|（t_i は L 週での実際の回数）。
// 使い方: g++ -std=c++20 -O2 ahc044.cpp -o ahc044 && ./ahc044 [ケース数=8] [1手法あたりの秒数=1.5]
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>

constexpr int N = 100, L = 500000;

std::vector<int> generate(uint64_t seed) {
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<int> u(0, 10000);
    while (true) {
        std::vector<int> t(N);
        long long s = 0;
        for (int i = 0; i < N - 1; i++) { t[i] = u(rng); s += t[i]; }
        if (L - s >= 0 && L - s <= 10000) { t[N - 1] = L - s; return t; }
    }
}

// len 週のシミュレーションで、各社員の回数を L 週に換算した値を返す
void simulate(const std::vector<int>& a, const std::vector<int>& b, int len, std::vector<double>& cnt) {
    static std::vector<int> c(N);
    std::fill(c.begin(), c.end(), 0);
    int x = 0;
    for (int w = 0; w < len; w++) {
        c[x]++;
        x = (c[x] & 1) ? a[x] : b[x];
    }
    cnt.assign(N, 0);
    for (int i = 0; i < N; i++) cnt[i] = (double)c[i] * L / len;
}

double error_of(const std::vector<double>& cnt, const std::vector<int>& T) {
    double e = 0;
    for (int i = 0; i < N; i++) e += std::fabs(cnt[i] - T[i]);
    return e;
}

double solve(const std::vector<int>& T, int mode, double seconds, uint64_t seed) {
    std::mt19937_64 rng(seed);
    std::vector<int> a(N), b(N);
    for (int i = 0; i < N; i++) { a[i] = rng() % N; b[i] = rng() % N; }
    auto start = std::chrono::steady_clock::now();
    auto elapsed = [&]() { return std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count(); };
    int stages = mode == 0 ? 1 : 8;
    std::vector<double> cnt;
    for (int st = 0; st < stages; st++) {
        int len = mode == 0 ? L : L >> (stages - 1 - st);
        double t_end = seconds * (st + 1) / stages;
        simulate(a, b, len, cnt);
        double cur = error_of(cnt, T);
        double t0 = 2000.0 * std::pow(0.6, st), t1 = 5.0;
        double t_begin = elapsed();
        while (true) {
            double now = elapsed();
            if (now >= t_end) break;
            double prog = (now - t_begin) / (t_end - t_begin);
            double temp = t0 * std::pow(t1 / t0, prog);
            // 近傍を作る
            int i, which, j;
            if (mode == 2) {
                // 行き先 j: 不足 (T_j − cnt_j) の2乗に比例。変える辺: 行き先が多すぎる辺を、超過の2乗に比例して選ぶ
                std::vector<double> w(N);
                double sw = 0;
                for (int k = 0; k < N; k++) { double d = std::max(0.0, T[k] - cnt[k]); w[k] = d * d + 1e-9; sw += w[k]; }
                double r = std::uniform_real_distribution<double>(0, sw)(rng);
                j = 0;
                while (j < N - 1 && (r -= w[j]) > 0) j++;
                std::vector<double> we(2 * N);
                double se = 0;
                for (int k = 0; k < 2 * N; k++) {
                    int to = k < N ? a[k] : b[k - N];
                    double d = std::max(0.0, cnt[to] - T[to]);
                    we[k] = d * d + 1e-9;
                    se += we[k];
                }
                r = std::uniform_real_distribution<double>(0, se)(rng);
                int e = 0;
                while (e < 2 * N - 1 && (r -= we[e]) > 0) e++;
                i = e % N;
                which = e / N;
            } else {
                i = rng() % N;
                which = rng() % 2;
                j = rng() % N;
            }
            int& slot = which == 0 ? a[i] : b[i];
            int old = slot;
            if (old == j) continue;
            slot = j;
            std::vector<double> nc;
            simulate(a, b, len, nc);
            double ne = error_of(nc, T);
            if (ne <= cur || std::exp((cur - ne) / temp) > std::uniform_real_distribution<double>(0, 1)(rng)) {
                cur = ne;
                cnt = nc;
            } else {
                slot = old;
            }
        }
    }
    simulate(a, b, L, cnt);
    return 1e6 - error_of(cnt, T);
}

int main(int argc, char** argv) {
    int cases = argc > 1 ? std::atoi(argv[1]) : 8;
    double sec = argc > 2 ? std::atof(argv[2]) : 1.5;
    double s[3] = {};
    for (int c = 0; c < cases; c++) {
        auto T = generate(8000 + c);
        for (int m = 0; m < 3; m++) s[m] += solve(T, m, sec, c);
    }
    const char* names[] = {"full", "multires", "biased"};
    std::printf("cases=%d seconds=%.2f\n", cases, sec);
    for (int m = 0; m < 3; m++) std::printf("%-9s 平均 %.0f（誤差 %.0f）\n", names[m], s[m] / cases, 1e6 - s[m] / cases);
}
