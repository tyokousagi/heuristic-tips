// __SITE_URL__/snippets/#grid
#pragma once
#include <array>
#include <vector>

// 方向は 上、右、下、左 の順。(k + 2) % 4 が逆向きになる。
constexpr int DR[4] = {-1, 0, 1, 0};
constexpr int DC[4] = {0, 1, 0, -1};
constexpr char DIR_CHAR[4] = {'U', 'R', 'D', 'L'};

// 番兵つきのグリッド。h×w の盤面の外周に壁を1マス足し、1次元の配列で持つ。
// 隣のマスは v + d[k] で求まり、範囲外の判定が要らない。
struct Grid {
    int h, w, W;            // W は番兵を含めた幅
    std::vector<char> wall; // 1 なら通れない
    std::array<int, 4> d;   // 4 方向の添字の差

    Grid(int h_, int w_) : h(h_), w(w_), W(w_ + 2), wall((size_t)(h_ + 2) * (w_ + 2), 1), d{-(w_ + 2), 1, w_ + 2, -1} {
        for (int r = 0; r < h; ++r)
            for (int c = 0; c < w; ++c) wall[id(r, c)] = 0;
    }

    int id(int r, int c) const { return (r + 1) * W + (c + 1); }
    int row(int v) const { return v / W - 1; }
    int col(int v) const { return v % W - 1; }
    int size() const { return (int)wall.size(); }

    // start からの最短距離。届かないマスは -1。
    std::vector<int> bfs(int start) const {
        std::vector<int> dist(wall.size(), -1);
        std::vector<int> que;
        que.reserve(wall.size());
        dist[start] = 0;
        que.push_back(start);
        for (size_t qi = 0; qi < que.size(); ++qi) {
            const int v = que[qi];
            for (int k = 0; k < 4; ++k) {
                const int u = v + d[k];
                if (wall[u] || dist[u] >= 0) continue;
                dist[u] = dist[v] + 1;
                que.push_back(u);
            }
        }
        return dist;
    }
};
