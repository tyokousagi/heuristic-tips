// https://tyokousagi.github.io/heuristic-tips/snippets/#zobrist
#pragma once
#include <cstdint>
#include <vector>

inline uint64_t splitmix64(uint64_t& s) {
    s += 0x9e3779b97f4a7c15ULL;
    uint64_t z = s;
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

// Zobrist hash 用の乱数表。「種類 kind の物がマス cell にある」ことに乱数を1つ割り当てる。
// 状態のハッシュ値は、成り立っている組の乱数をすべて XOR した値にする。
// 物を置くときも取り除くときも hash ^= zobrist(kind, cell) で更新できる。
struct Zobrist {
    int cells;
    std::vector<uint64_t> table;

    Zobrist(int kinds, int cells_, uint64_t seed = 1) : cells(cells_), table((size_t)kinds * cells_) {
        for (auto& x : table) x = splitmix64(seed);
    }

    uint64_t operator()(int kind, int cell) const { return table[(size_t)kind * cells + cell]; }
};
