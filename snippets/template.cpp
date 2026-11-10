// __SITE_URL__/snippets/#template
// AHC 用のひな形。入力、解の出力、時間の管理、乱数、スコアの表示をひととおり入れてある。
#pragma GCC optimize("O3,unroll-loops")
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <vector>
using namespace std;

struct Timer {
    chrono::steady_clock::time_point start = chrono::steady_clock::now();
    double elapsed() const { return chrono::duration<double>(chrono::steady_clock::now() - start).count(); }
};

struct Xorshift {
    uint64_t x = 88172645463325252ULL;
    uint64_t next() {
        x ^= x << 13;
        x ^= x >> 7;
        x ^= x << 17;
        return x;
    }
    uint32_t next_int(uint32_t n) { return (uint32_t)(((next() >> 32) * (uint64_t)n) >> 32); }
    int next_int(int l, int r) { return l + (int)next_int((uint32_t)(r - l)); }
    double next_double() { return (double)(next() >> 11) * (1.0 / 9007199254740992.0); }
};

// 制限時間より少し手前で探索を終える。出力にかかる時間の分を残す。
constexpr double TIME_LIMIT = 1.9;

Timer timer;
Xorshift rng;

struct Input {
    int n = 0;
    // 問題に合わせて入力の項目を足す

    void read() { cin >> n; }
};

struct Solution {
    vector<int> order;
    long long score = 0;

    void print() const {
        for (int v : order) cout << v << '\n';
    }
};

// 解のスコアを最初から計算する。差分計算の検算にも使う。
long long calc_score(const Input& in, const Solution& sol) {
    (void)in;
    (void)sol;
    return 0;
}

Solution solve(const Input& in) {
    // まずは形式が正しいだけの自明な解を作る
    Solution sol;
    sol.order.resize(in.n);
    iota(sol.order.begin(), sol.order.end(), 0);
    sol.score = calc_score(in, sol);

    // ここに貪欲法、山登り法、焼きなましなどを書く
    long long iterations = 0;
    while (timer.elapsed() < TIME_LIMIT) {
        ++iterations;
        break;
    }
    cerr << "iterations = " << iterations << '\n';
    return sol;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Input in;
    in.read();
    const Solution sol = solve(in);
    sol.print();

    // 判定に関係しない情報は標準エラー出力に書く。ローカルのテストではこの行からスコアを読む。
    cerr << "Score = " << sol.score << '\n';
    cerr << "Time = " << timer.elapsed() << '\n';
    return 0;
}
