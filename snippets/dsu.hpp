// https://tyokousagi.github.io/heuristic-tips/snippets/#dsu
#pragma once
#include <utility>
#include <vector>

// Union-Find。連結成分の管理に使う。
struct Dsu {
    std::vector<int> p; // 根なら -(成分の大きさ)、そうでなければ親

    explicit Dsu(int n) : p(n, -1) {}

    int root(int v) { return p[v] < 0 ? v : p[v] = root(p[v]); }
    bool same(int a, int b) { return root(a) == root(b); }
    int size(int v) { return -p[root(v)]; }

    // 別の成分だったら true を返す。
    bool unite(int a, int b) {
        a = root(a), b = root(b);
        if (a == b) return false;
        if (p[a] > p[b]) std::swap(a, b);
        p[a] += p[b];
        p[b] = a;
        return true;
    }
};
