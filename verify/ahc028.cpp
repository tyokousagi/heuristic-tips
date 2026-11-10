// AHC028 Lucky Words の検証。
// 15×15 のキーボードで、長さ5の単語200個をすべて含む文字列を打つ。1打のコストは指の移動距離+1。
// 単語の重なり（接頭辞と接尾辞の共有）は使わず、単語を並べる順番だけをビームサーチで決める。
// 単語 w を、指がマス c にある状態から打ち、マス e で打ち終える最小コストを前計算しておく。
//   raw  : 評価 = これまでのコストの和
//   base : 評価 = これまでのコストの和 − Σ base(使った単語)。base(w) は w を最も安く打つコスト
//          （どのマスから始めてもよい）。避けられないコストを引いて比べる
// 得点は 10000 − T（T は総コスト）。
// 使い方: g++ -std=c++20 -O2 ahc028.cpp -o ahc028 && ./ahc028 [ケース数=10] [ビーム幅=30]
#include <algorithm>
#include <array>
#include <bitset>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <set>
#include <string>
#include <vector>

constexpr int N = 15, M = 200, C = N * N;

struct Case {
    std::array<char, C> key;
    int start;
    std::vector<std::string> words;
};

Case generate(uint64_t seed) {
    std::mt19937_64 rng(seed);
    Case c;
    c.start = (rng() % N) * N + rng() % N;
    while (true) {
        std::set<char> s;
        for (auto& k : c.key) { k = 'A' + rng() % 26; s.insert(k); }
        if (s.size() == 26) break;
    }
    std::set<std::string> seen;
    while ((int)c.words.size() < M) {
        std::string w(5, 'A');
        for (auto& ch : w) ch = 'A' + rng() % 26;
        if (seen.insert(w).second) c.words.push_back(w);
    }
    std::sort(c.words.begin(), c.words.end());
    return c;
}

int dist(int a, int b) { return std::abs(a / N - b / N) + std::abs(a % N - b % N); }

struct Node {
    std::bitset<M> used;
    int cell;
    int cost;
    int basesum;
};

int main(int argc, char** argv) {
    int cases = argc > 1 ? std::atoi(argv[1]) : 10;
    int width = argc > 2 ? std::atoi(argv[2]) : 30;
    double s_raw = 0, s_base = 0;
    for (int cs = 0; cs < cases; cs++) {
        Case c = generate(10000 + cs);
        std::array<std::vector<int>, 26> cells;
        for (int v = 0; v < C; v++) cells[c.key[v] - 'A'].push_back(v);
        // cost[w][start][k]: 単語 w を start から打って、最後の文字の k 番目のマスで終える最小コスト
        std::vector<std::vector<std::vector<int>>> cost(M, std::vector<std::vector<int>>(C));
        std::vector<int> base(M, 1 << 30);
        for (int w = 0; w < M; w++) {
            const auto& word = c.words[w];
            for (int st = 0; st < C; st++) {
                std::vector<int> prev_cells{st}, prev_cost{0};
                for (char ch : word) {
                    const auto& cs2 = cells[ch - 'A'];
                    std::vector<int> nc(cs2.size(), 1 << 30);
                    for (size_t a = 0; a < prev_cells.size(); a++)
                        for (size_t b = 0; b < cs2.size(); b++)
                            nc[b] = std::min(nc[b], prev_cost[a] + dist(prev_cells[a], cs2[b]) + 1);
                    prev_cells = cs2;
                    prev_cost = nc;
                }
                cost[w][st] = prev_cost;
                for (int v : prev_cost) base[w] = std::min(base[w], v);
            }
        }
        auto run = [&](bool use_base) {
            std::vector<Node> beam{{std::bitset<M>(), c.start, 0, 0}};
            for (int step = 0; step < M; step++) {
                std::vector<std::pair<int, std::array<int, 4>>> cand;  // 評価値, (親, 単語, 終わりのマス, コスト)
                for (int p = 0; p < (int)beam.size(); p++) {
                    const auto& nd = beam[p];
                    for (int w = 0; w < M; w++) {
                        if (nd.used[w]) continue;
                        const auto& last = cells[c.words[w][4] - 'A'];
                        for (size_t k = 0; k < last.size(); k++) {
                            int nc = nd.cost + cost[w][nd.cell][k];
                            int ev = use_base ? nc - (nd.basesum + base[w]) : nc;
                            cand.push_back({ev, {p, w, last[k], nc}});
                        }
                    }
                }
                int keep = std::min<int>(width, cand.size());
                std::partial_sort(cand.begin(), cand.begin() + keep, cand.end(),
                                  [](const auto& x, const auto& y) { return x.first < y.first; });
                std::vector<Node> next;
                for (int t = 0; t < keep; t++) {
                    auto [p, w, cell, nc] = cand[t].second;
                    Node nd = beam[p];
                    nd.used[w] = true;
                    nd.cell = cell;
                    nd.cost = nc;
                    nd.basesum += base[w];
                    next.push_back(nd);
                }
                beam = std::move(next);
            }
            int best = 1 << 30;
            for (auto& nd : beam) best = std::min(best, nd.cost);
            return best;
        };
        int t_raw = run(false), t_base = run(true);
        s_raw += std::max(10000 - t_raw, 1001);
        s_base += std::max(10000 - t_base, 1001);
        std::fprintf(stderr, "case %d: raw T=%d base T=%d\n", cs, t_raw, t_base);
    }
    std::printf("cases=%d width=%d\nraw  平均 %.0f\nbase 平均 %.0f\n", cases, width, s_raw / cases, s_base / cases);
}
