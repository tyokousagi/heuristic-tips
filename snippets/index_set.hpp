// https://tyokousagi.github.io/heuristic-tips/snippets/#index-set
#pragma once
#include <cstdint>
#include <vector>

// 0 以上 n 未満の整数の集合。追加、削除、所属の判定、ランダムな要素の取得がすべて O(1)。
struct IndexSet {
    std::vector<int> items; // 集合の要素（順不同）
    std::vector<int> pos;   // pos[v]: items の中での v の位置。含まれなければ -1

    explicit IndexSet(int n) : pos(n, -1) { items.reserve(n); }

    bool contains(int v) const { return pos[v] >= 0; }
    int size() const { return (int)items.size(); }
    bool empty() const { return items.empty(); }
    int operator[](int i) const { return items[i]; }

    void add(int v) {
        if (pos[v] >= 0) return;
        pos[v] = (int)items.size();
        items.push_back(v);
    }

    void remove(int v) {
        const int p = pos[v];
        if (p < 0) return;
        const int last = items.back();
        items[p] = last; // 末尾の要素を空いた場所へ移す
        pos[last] = p;
        items.pop_back();
        pos[v] = -1;
    }

    // rng.next_int(n) は [0, n) の整数を返す。集合が空のときは呼ばない。
    template <class Rng>
    int random(Rng& rng) const {
        return items[rng.next_int((uint32_t)items.size())];
    }
};
