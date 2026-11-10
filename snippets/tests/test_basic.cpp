// データ構造とユーティリティのテスト
#include <cassert>
#include <cmath>
#include <cstdio>
#include <set>

#include "../dijkstra.hpp"
#include "../dsu.hpp"
#include "../grid.hpp"
#include "../hash_set.hpp"
#include "../index_set.hpp"
#include "../log_table.hpp"
#include "../normal_prob.hpp"
#include "../stamp_set.hpp"
#include "../timer.hpp"
#include "../xorshift.hpp"
#include "../zobrist.hpp"

static void test_xorshift() {
    Xorshift rng(12345);
    int count[10] = {};
    for (int i = 0; i < 100000; ++i) {
        const uint32_t v = rng.next_int(10u);
        assert(v < 10);
        ++count[v];
        const int w = rng.next_int(-3, 4);
        assert(-3 <= w && w < 4);
        const double d = rng.next_double();
        assert(0.0 <= d && d < 1.0);
    }
    for (int c : count) assert(9000 < c && c < 11000); // おおよそ一様
    Xorshift a(7), b(7);
    for (int i = 0; i < 100; ++i) assert(a.next() == b.next()); // シードが同じなら同じ列
}

static void test_index_set() {
    Xorshift rng(1);
    IndexSet s(50);
    std::set<int> ref;
    for (int i = 0; i < 20000; ++i) {
        const int v = rng.next_int(0, 50);
        if (rng.next_int(2u)) {
            s.add(v);
            ref.insert(v);
        } else {
            s.remove(v);
            ref.erase(v);
        }
        assert(s.size() == (int)ref.size());
        assert(s.contains(v) == (ref.count(v) > 0));
        if (!s.empty()) assert(ref.count(s.random(rng)) > 0);
    }
    for (int i = 0; i < s.size(); ++i) assert(ref.count(s[i]) > 0);
}

static void test_stamp_and_hash() {
    StampSet st(10);
    st.set(3);
    assert(st.test(3) && !st.test(4));
    st.clear();
    assert(!st.test(3));
    st.set(4);
    st.reset(4);
    assert(!st.test(4));

    HashSet hs(10);
    Xorshift rng(2);
    std::set<uint64_t> ref;
    for (int round = 0; round < 3; ++round) {
        hs.clear();
        ref.clear();
        for (int i = 0; i < 200; ++i) {
            const uint64_t key = rng.next() % 300;
            assert(hs.insert(key) == ref.insert(key).second);
            assert(hs.contains(key));
        }
        assert(!hs.contains(100000));
    }
}

static void test_zobrist() {
    Zobrist z(3, 16);
    std::set<uint64_t> values;
    for (int k = 0; k < 3; ++k)
        for (int c = 0; c < 16; ++c) values.insert(z(k, c));
    assert(values.size() == 48); // すべて違う値
    // 置く順番が違っても同じハッシュ値になり、取り除くと元に戻る
    const uint64_t h1 = z(0, 1) ^ z(2, 5) ^ z(1, 9);
    const uint64_t h2 = z(1, 9) ^ z(0, 1) ^ z(2, 5);
    assert(h1 == h2);
    assert((h1 ^ z(2, 5)) == (z(0, 1) ^ z(1, 9)));
}

static void test_dsu() {
    Dsu d(6);
    assert(d.unite(0, 1));
    assert(d.unite(2, 3));
    assert(!d.same(1, 2));
    assert(d.unite(1, 3));
    assert(!d.unite(0, 2));
    assert(d.same(0, 3) && d.size(0) == 4 && d.size(5) == 1);
}

static void test_grid_and_dijkstra() {
    // 5x5 で、(1,0)〜(1,3) を壁にする
    Grid g(5, 5);
    for (int c = 0; c < 4; ++c) g.wall[g.id(1, c)] = 1;
    const auto dist = g.bfs(g.id(0, 0));
    assert(dist[g.id(0, 0)] == 0);
    assert(dist[g.id(0, 4)] == 4);
    assert(dist[g.id(2, 0)] == 10); // 右端を回り込む
    assert(dist[g.id(1, 1)] == -1); // 壁には届かない
    assert(g.row(g.id(3, 2)) == 3 && g.col(g.id(3, 2)) == 2);

    // 同じグリッドを重み 1 のグラフにして、ダイクストラ法と BFS の結果を比べる
    std::vector<std::vector<std::pair<int, long long>>> graph(g.size());
    for (int v = 0; v < g.size(); ++v) {
        if (g.wall[v]) continue;
        for (int k = 0; k < 4; ++k)
            if (!g.wall[v + g.d[k]]) graph[v].push_back({v + g.d[k], 1});
    }
    std::vector<int> prev;
    const auto dd = dijkstra(graph, g.id(0, 0), &prev);
    for (int v = 0; v < g.size(); ++v) {
        if (dist[v] >= 0) assert(dd[v] == dist[v]);
    }
    int steps = 0;
    for (int v = g.id(2, 0); v != g.id(0, 0); v = prev[v]) ++steps;
    assert(steps == 10);
}

static void test_normal_prob() {
    assert(std::abs(normal_cdf(0.0, 0.0, 1.0) - 0.5) < 1e-12);
    // 四捨五入した値の確率を全部足すと 1 になる
    double sum = 0;
    for (int r = -100; r <= 100; ++r) sum += rounded_normal_prob(r, 3.2, 5.0);
    assert(std::abs(sum - 1.0) < 1e-9);
    // 下限 0 がある場合も合計は 1
    sum = 0;
    for (int r = 0; r <= 100; ++r) sum += rounded_normal_prob(r, 3.2, 5.0, 0.0);
    assert(std::abs(sum - 1.0) < 1e-9);

    std::vector<double> ll = {-1000.0, -1001.0, -1002.0};
    log_likelihood_to_prob(ll);
    assert(std::abs(ll[0] + ll[1] + ll[2] - 1.0) < 1e-12);
    assert(std::abs(ll[1] / ll[0] - std::exp(-1.0)) < 1e-12);
}

static void test_log_table_and_timer() {
    static const LogTable table;
    double sum = 0;
    for (int i = 0; i < (1 << LogTable::BITS); ++i) {
        assert(table(i) < 0);
        sum += table(i);
    }
    // 一様乱数の log の平均は -1
    assert(std::abs(sum / (1 << LogTable::BITS) + 1.0) < 1e-3);

    Timer timer;
    const double a = timer.elapsed();
    const double b = timer.elapsed();
    assert(0 <= a && a <= b);
}

int main() {
    test_xorshift();
    test_index_set();
    test_stamp_and_hash();
    test_zobrist();
    test_dsu();
    test_grid_and_dijkstra();
    test_normal_prob();
    test_log_table_and_timer();
    std::puts("test_basic: OK");
    return 0;
}
