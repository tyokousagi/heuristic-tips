// AHC021 Pyramid Sorting の検証。
// 30段のピラミッドに 0..464 のボール。隣接交換で「どのボールも直下の2個より小さい」状態にする。
// 得点は E=0 なら 100000 − 5K（K は交換回数）。
//   sift   : 下の段から順に、直下より大きいボールを小さいほうの子と交換して沈める（ヒープ構築）
//   up_large : 小さい番号から順に、上の2個のうち自分より大きいものがある間、大きいほうと交換して上げる（解説の貪欲）
//   up_small : 同じだが、自分より大きい上の2個のうち小さいほうと交換する
//   up_rand  : 同じだが、自分より大きい上の2個からランダムに選ぶ
// 使い方: g++ -std=c++20 -O2 ahc021.cpp -o ahc021 && ./ahc021 [ケース数=50]
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <numeric>
#include <random>
#include <vector>

constexpr int N = 30, B = N * (N + 1) / 2;

struct Pyr {
    std::vector<int> v;   // 位置 -> 数字
    std::vector<int> pos; // 数字 -> 位置
    static int id(int x, int y) { return x * (x + 1) / 2 + y; }
    void swap_at(int a, int b) {
        std::swap(v[a], v[b]);
        pos[v[a]] = a;
        pos[v[b]] = b;
    }
};

Pyr generate(uint64_t seed) {
    std::mt19937_64 rng(seed);
    Pyr p;
    p.v.resize(B);
    std::iota(p.v.begin(), p.v.end(), 0);
    std::shuffle(p.v.begin(), p.v.end(), rng);
    p.pos.resize(B);
    for (int i = 0; i < B; i++) p.pos[p.v[i]] = i;
    return p;
}

int errors(const Pyr& p) {
    int e = 0;
    for (int x = 0; x + 1 < N; x++)
        for (int y = 0; y <= x; y++) {
            int a = p.v[Pyr::id(x, y)];
            if (a > p.v[Pyr::id(x + 1, y)]) e++;
            if (a > p.v[Pyr::id(x + 1, y + 1)]) e++;
        }
    return e;
}

int sift(Pyr p) {
    int k = 0;
    for (int x = N - 2; x >= 0; x--)
        for (int y = 0; y <= x; y++) {
            int cx = x, cy = y;
            while (cx + 1 < N) {
                int a = Pyr::id(cx, cy), l = Pyr::id(cx + 1, cy), r = Pyr::id(cx + 1, cy + 1);
                int c = p.v[l] < p.v[r] ? l : r;
                if (p.v[a] < p.v[c]) break;
                p.swap_at(a, c);
                k++;
                cx++;
                if (c == r) cy++;
            }
        }
    return errors(p) == 0 ? 100000 - 5 * k : 50000 - 50 * errors(p);
}

// mode 0: 大きいほう, 1: 小さいほう, 2: ランダム
int up(Pyr p, int mode, std::mt19937_64& rng) {
    int k = 0;
    // 位置から (x, y) を引く表
    std::vector<int> px(B), py(B);
    for (int x = 0; x < N; x++)
        for (int y = 0; y <= x; y++) { px[Pyr::id(x, y)] = x; py[Pyr::id(x, y)] = y; }
    for (int num = 0; num < B; num++) {
        while (true) {
            int a = p.pos[num], x = px[a], y = py[a];
            if (x == 0) break;
            int cand[2], n = 0;
            if (y - 1 >= 0 && p.v[Pyr::id(x - 1, y - 1)] > num) cand[n++] = Pyr::id(x - 1, y - 1);
            if (y <= x - 1 && p.v[Pyr::id(x - 1, y)] > num) cand[n++] = Pyr::id(x - 1, y);
            if (n == 0) break;
            int c = cand[0];
            if (n == 2) {
                if (mode == 0) c = p.v[cand[0]] > p.v[cand[1]] ? cand[0] : cand[1];
                else if (mode == 1) c = p.v[cand[0]] < p.v[cand[1]] ? cand[0] : cand[1];
                else c = cand[rng() % 2];
            }
            p.swap_at(a, c);
            k++;
        }
    }
    int e = errors(p);
    return e == 0 ? 100000 - 5 * k : 50000 - 50 * e;
}

int main(int argc, char** argv) {
    int cases = argc > 1 ? std::atoi(argv[1]) : 50;
    double s[4] = {};
    std::mt19937_64 rng(1);
    for (int c = 0; c < cases; c++) {
        Pyr p = generate(6000 + c);
        s[0] += sift(p);
        s[1] += up(p, 0, rng);
        s[2] += up(p, 1, rng);
        s[3] += up(p, 2, rng);
    }
    const char* names[] = {"sift", "up_large", "up_small", "up_rand"};
    std::printf("cases=%d\n", cases);
    for (int k = 0; k < 4; k++) std::printf("%-8s 平均 %.0f（交換回数に直すと %.0f 回）\n", names[k], s[k] / cases, (100000 - s[k] / cases) / 5);
}
