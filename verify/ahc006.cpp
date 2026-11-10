// AHC006 Food Delivery の検証。
// 1000件の注文から50件を選び、(400,400) から出て、各注文のレストラン→配達先の順に回って戻る。
// 得点は round(10^8 / (1000 + 総移動距離))。
//   nn       : 近いレストランを50軒回り、そのあと配達先を近い順に回る（terry_u16 の段階1）
//   nn_near  : 同じだが、レストランと配達先がともにオフィスから距離400以内の注文に限る（段階2）
//   insert   : 経路への挿入コストが最小の注文を1件ずつ足す（段階5・6。候補は全注文）
//   insert_lns : insert のあと、1件抜いて最良の注文を挿入し直す局所探索（候補は距離400以内）
// 使い方: g++ -std=c++20 -O2 ahc006.cpp -o ahc006 && ./ahc006 [ケース数=20] [局所探索の回数=4000]
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>

struct Pt { int x, y; };
int dist(Pt a, Pt b) { return std::abs(a.x - b.x) + std::abs(a.y - b.y); }
const Pt O{400, 400};
struct Order { Pt r, d; };

std::vector<Order> generate(uint64_t seed) {
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<int> u(0, 800);
    std::vector<Order> o(1000);
    for (auto& e : o) {
        do { e.r = {u(rng), u(rng)}; e.d = {u(rng), u(rng)}; } while (dist(e.r, e.d) < 100);
    }
    return o;
}

long long route_len(const std::vector<Pt>& r) {
    long long t = 0;
    for (size_t i = 0; i + 1 < r.size(); i++) t += dist(r[i], r[i + 1]);
    return t;
}

double score(long long t) { return 1e8 / (1000.0 + t); }

long long nn(const std::vector<Order>& o, bool near_only) {
    std::vector<int> cand;
    for (int i = 0; i < 1000; i++)
        if (!near_only || (dist(o[i].r, O) <= 400 && dist(o[i].d, O) <= 400)) cand.push_back(i);
    std::vector<Pt> route{O};
    std::vector<bool> used(1000, false);
    std::vector<int> chosen;
    Pt cur = O;
    for (int k = 0; k < 50; k++) {
        int best = -1;
        for (int i : cand)
            if (!used[i] && (best < 0 || dist(cur, o[i].r) < dist(cur, o[best].r))) best = i;
        used[best] = true;
        chosen.push_back(best);
        cur = o[best].r;
        route.push_back(cur);
    }
    std::vector<bool> done(1000, false);
    for (int k = 0; k < 50; k++) {
        int best = -1;
        for (int i : chosen)
            if (!done[i] && (best < 0 || dist(cur, o[i].d) < dist(cur, o[best].d))) best = i;
        done[best] = true;
        cur = o[best].d;
        route.push_back(cur);
    }
    route.push_back(O);
    return route_len(route);
}

// 経路 r に注文 e を挿入したときの増分の最小値と挿入位置（p を i の後、q を j の後。i <= j）
long long best_insert(const std::vector<Pt>& r, const Order& e, int& bi, int& bj) {
    int L = r.size();
    long long best = 1LL << 60;
    long long pre = 1LL << 60;
    int pre_i = -1;
    for (int j = 0; j + 1 < L; j++) {
        // i == j の場合
        long long same = dist(r[j], e.r) + dist(e.r, e.d) + dist(e.d, r[j + 1]) - dist(r[j], r[j + 1]);
        if (same < best) { best = same; bi = j; bj = j; }
        // i < j の場合（i は j より前の最良の位置）
        long long dq = dist(r[j], e.d) + dist(e.d, r[j + 1]) - dist(r[j], r[j + 1]);
        if (pre_i >= 0 && pre + dq < best) { best = pre + dq; bi = pre_i; bj = j; }
        long long dp = dist(r[j], e.r) + dist(e.r, r[j + 1]) - dist(r[j], r[j + 1]);
        if (dp < pre) { pre = dp; pre_i = j; }
    }
    return best;
}

void do_insert(std::vector<Pt>& r, std::vector<int>& owner, const Order& e, int id, int i, int j) {
    // owner[k] は r[k] がどの注文のどちらか（id*2 がレストラン、id*2+1 が配達先、-1 はオフィス）
    r.insert(r.begin() + j + 1, e.d);
    owner.insert(owner.begin() + j + 1, id * 2 + 1);
    r.insert(r.begin() + i + 1, e.r);
    owner.insert(owner.begin() + i + 1, id * 2);
}

long long insertion(const std::vector<Order>& o, int lns_iters, uint64_t seed) {
    std::vector<Pt> r{O, O};
    std::vector<int> owner{-1, -1};
    std::vector<bool> used(1000, false);
    for (int k = 0; k < 50; k++) {
        long long best = 1LL << 60;
        int bid = -1, bi = 0, bj = 0;
        for (int i = 0; i < 1000; i++) {
            if (used[i]) continue;
            int ti, tj;
            long long c = best_insert(r, o[i], ti, tj);
            if (c < best) { best = c; bid = i; bi = ti; bj = tj; }
        }
        used[bid] = true;
        do_insert(r, owner, o[bid], bid, bi, bj);
    }
    if (lns_iters == 0) return route_len(r);
    std::vector<int> near;
    for (int i = 0; i < 1000; i++)
        if (dist(o[i].r, O) <= 400 && dist(o[i].d, O) <= 400) near.push_back(i);
    std::mt19937_64 rng(seed);
    long long cur = route_len(r);
    for (int it = 0; it < lns_iters; it++) {
        // 選ばれている注文を1件抜く
        int k = 1 + rng() % (r.size() - 2);
        int id = owner[k] / 2;
        std::vector<Pt> r2;
        std::vector<int> ow2;
        for (size_t t = 0; t < r.size(); t++)
            if (owner[t] < 0 || owner[t] / 2 != id) { r2.push_back(r[t]); ow2.push_back(owner[t]); }
        long long base = route_len(r2), best = 1LL << 60;
        int bid = -1, bi = 0, bj = 0;
        used[id] = false;
        for (int i : near) {
            if (used[i]) continue;
            int ti, tj;
            long long c = best_insert(r2, o[i], ti, tj);
            if (c < best) { best = c; bid = i; bi = ti; bj = tj; }
        }
        // 抜いた注文自身も候補に入れる（near に入っていなくても戻せるように）
        { int ti, tj; long long c = best_insert(r2, o[id], ti, tj); if (c < best) { best = c; bid = id; bi = ti; bj = tj; } }
        if (base + best <= cur) {
            do_insert(r2, ow2, o[bid], bid, bi, bj);
            r = std::move(r2);
            owner = std::move(ow2);
            used[bid] = true;
            cur = base + best;
        } else {
            used[id] = true;
        }
    }
    return cur;
}

int main(int argc, char** argv) {
    int cases = argc > 1 ? std::atoi(argv[1]) : 20;
    int iters = argc > 2 ? std::atoi(argv[2]) : 4000;
    double s[4] = {};
    for (int c = 0; c < cases; c++) {
        auto o = generate(7000 + c);
        s[0] += score(nn(o, false));
        s[1] += score(nn(o, true));
        s[2] += score(insertion(o, 0, c));
        s[3] += score(insertion(o, iters, c));
    }
    const char* names[] = {"nn", "nn_near", "insert", "insert_lns"};
    std::printf("cases=%d lns=%d\n", cases, iters);
    for (int k = 0; k < 4; k++) std::printf("%-10s 平均 %.0f\n", names[k], s[k] / cases);
}
