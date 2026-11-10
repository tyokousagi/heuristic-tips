// AHC062 King's Tour の検証。
// N=200 の盤面（A は 1..N^2 の順列）を、縦横斜めに隣接するマスへ移動しながら全マス1回ずつ訪れる。
// 得点は Σ k·A(k日目のマス) / N^2。後で訪れるマスほど重みが大きい。
//   snake : A を見ずに行ごとに蛇行する（相関 0、約 400M 相当）
//   pair  : 2行の帯を往復する。行きは各列の2マスのうち小さいほう、帰りは大きいほうを訪れる
//           （解説の「ペア解法」。相関 約50%、約 466.7M 相当）
//   bound : A を小さい順に並べた理想値（経路の制約を無視した上限、約 533M 相当）
// 使い方: g++ -std=c++20 -O2 ahc062.cpp -o ahc062 && ./ahc062 [ケース数=20]
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <numeric>
#include <random>
#include <vector>

constexpr int N = 200;

std::vector<int> generate(uint64_t seed) {
    std::vector<int> a(N * N);
    std::iota(a.begin(), a.end(), 1);
    std::mt19937_64 rng(seed);
    std::shuffle(a.begin(), a.end(), rng);
    return a;
}

double score(const std::vector<int>& a, const std::vector<int>& tour) {
    // 正しいツアーかを確かめてから得点を返す（不正なら -1）
    if ((int)tour.size() != N * N) return -1;
    std::vector<bool> seen(N * N, false);
    for (int k = 0; k < N * N; k++) {
        if (seen[tour[k]]) return -1;
        seen[tour[k]] = true;
        if (k > 0) {
            int di = std::abs(tour[k] / N - tour[k - 1] / N), dj = std::abs(tour[k] % N - tour[k - 1] % N);
            if (std::max(di, dj) != 1) return -1;
        }
    }
    double v = 0;
    for (int k = 0; k < N * N; k++) v += (double)k * a[tour[k]];
    return v / (N * N);
}

std::vector<int> snake() {
    std::vector<int> t;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) t.push_back(i * N + (i % 2 == 0 ? j : N - 1 - j));
    return t;
}

// ペア解法。帯 s は行 2s, 2s+1、列 1..198。偶数の帯は右向き、奇数の帯は左向き。
// 帯の折り返しは、右端なら列199、左端なら列0の4マスを使う。
// 折り返しでは、行きが「上の帯の外側の行 → 列の4マス → 下の帯の外側の行」、帰りが内側の2マスを直接つなぐ。
// 行きと帰りのどちらに4マスを入れるかは、値の和が小さいほうを行き（早い時刻）にする。
std::vector<int> pair_tour(const std::vector<int>& a) {
    auto id = [](int i, int j) { return i * N + j; };
    // 帯に沿った「スロット」の列を作る。各スロットは行き用と帰り用の2マス
    std::vector<int> fwd, bwd;  // 帰りは逆順で訪れる
    // 開始: 列0の行0,1を最初に訪れる（時刻 0, 1）
    std::vector<int> head = {id(0, 0), id(1, 0)};
    for (int s = 0; s < N / 2; s++) {
        int r = 2 * s;
        bool right = s % 2 == 0;
        int last = right ? N - 2 : 1;  // 帯の最後の列
        // 2本目以降の帯の最初の列は、前の折り返しで使ったので飛ばす
        for (int step = s > 0 ? 1 : 0; step < N - 2; step++) {
            int j = right ? 1 + step : N - 2 - step;
            int u = id(r, j), d = id(r + 1, j);
            if (s + 1 < N / 2 && j == last) {
                // 折り返し直前の列は後で決める
                break;
            }
            if (a[u] < a[d]) { fwd.push_back(u); bwd.push_back(d); }
            else { fwd.push_back(d); bwd.push_back(u); }
        }
        if (s + 1 == N / 2) {
            // 最後の帯の端（列1）は、行きの最後と帰りの最初を列0の2マスでつなぐ
            continue;
        }
        // 折り返し: 列 c の4マス（行 r..r+3）と、帯 s の最後の列 last、帯 s+1 の最初の列 last
        int c = right ? N - 1 : 0;
        std::vector<int> col = {id(r, c), id(r + 1, c), id(r + 2, c), id(r + 3, c)};
        int outer_top = id(r, last), inner_top = id(r + 1, last);
        int inner_bot = id(r + 2, last), outer_bot = id(r + 3, last);
        long long sum_col = 0;
        for (int v : col) sum_col += a[v];
        long long sum_inner = a[inner_top] + a[inner_bot];
        // 行き用に外側+4マス、帰り用に内側2マス、または逆
        // 「列の4マス+外側2マス」の6マスと「内側2マス」の2マスのどちらを早く訪れるか。
        // 早い時刻に小さい値を置きたいので、平均値が小さいほうを行きにする。
        bool col_fwd = (double)(sum_col + a[outer_top] + a[outer_bot]) / 6 < (double)sum_inner / 2;
        if (col_fwd) {
            fwd.push_back(outer_top);
            for (int v : col) fwd.push_back(v);
            fwd.push_back(outer_bot);
            bwd.push_back(inner_top);
            bwd.push_back(inner_bot);
        } else {
            fwd.push_back(inner_top);
            fwd.push_back(inner_bot);
            bwd.push_back(outer_top);
            for (int v : col) bwd.push_back(v);
            bwd.push_back(outer_bot);
        }
    }
    // 帰りのスロットは、折り返しの区間だけ順番が行きと対応していない。
    // 帰りは bwd を逆順に訪れる。折り返し区間で bwd に入れた順（上→下）を逆にたどると下→上になり、
    // 行きの逆向きの移動と一致する。
    std::vector<int> tour = head;
    for (int v : fwd) tour.push_back(v);
    // 最後の帯（行 198, 199、左向きで列1が最後）の端: 列0の行198,199を挟む
    tour.push_back(id(N - 2, 0));
    tour.push_back(id(N - 1, 0));
    for (int k = (int)bwd.size() - 1; k >= 0; k--) tour.push_back(bwd[k]);
    return tour;
}

int main(int argc, char** argv) {
    int cases = argc > 1 ? std::atoi(argv[1]) : 20;
    double ss = 0, sp = 0, sb = 0;
    int bad = 0;
    for (int s = 0; s < cases; s++) {
        auto a = generate(4000 + s);
        double vs = score(a, snake());
        double vp = score(a, pair_tour(a));
        if (vp < 0) bad++;
        std::vector<int> sorted(a);
        std::sort(sorted.begin(), sorted.end());
        double vb = 0;
        for (int k = 0; k < N * N; k++) vb += (double)k * sorted[k];
        vb /= N * N;
        ss += vs; sp += vp; sb += vb;
        std::fprintf(stderr, "case %d: snake=%.0f pair=%.0f bound=%.0f\n", s, vs, vp, vb);
    }
    std::printf("cases=%d 不正なツアー=%d\nsnake 平均 %.4e\npair  平均 %.4e\nbound 平均 %.4e\n", cases, bad, ss / cases, sp / cases, sb / cases);
}
