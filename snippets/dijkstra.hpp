// https://tyokousagi.github.io/heuristic-tips/snippets/#dijkstra
#pragma once
#include <functional>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

// 隣接リスト g[v] = {(行き先, 重み), ...} のグラフで、start からの最短距離を求める。
// 届かない頂点は INF。prev を渡すと、最短路で1つ前の頂点が入る（始点と届かない頂点は -1）。
inline std::vector<long long> dijkstra(const std::vector<std::vector<std::pair<int, long long>>>& g, int start,
                                       std::vector<int>* prev = nullptr) {
    const long long INF = std::numeric_limits<long long>::max();
    std::vector<long long> dist(g.size(), INF);
    if (prev) prev->assign(g.size(), -1);
    using P = std::pair<long long, int>;
    std::priority_queue<P, std::vector<P>, std::greater<P>> pq;
    dist[start] = 0;
    pq.push({0, start});
    while (!pq.empty()) {
        const auto [dv, v] = pq.top();
        pq.pop();
        if (dv > dist[v]) continue;
        for (const auto& [u, w] : g[v]) {
            if (dist[u] > dv + w) {
                dist[u] = dv + w;
                if (prev) (*prev)[u] = v;
                pq.push({dist[u], u});
            }
        }
    }
    return dist;
}
