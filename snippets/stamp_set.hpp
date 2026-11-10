// https://tyokousagi.github.io/heuristic-tips/snippets/#stamp-set
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

// 一括クリアが O(1) の真偽値配列。BFS の訪問済みの記録などを何度も作り直すときに使う。
struct StampSet {
    std::vector<uint32_t> stamp;
    uint32_t cur = 1;

    explicit StampSet(int n) : stamp(n, 0) {}

    void clear() {
        if (++cur == 0) { // カウンタが一周したときだけ全体を初期化する
            std::fill(stamp.begin(), stamp.end(), 0);
            cur = 1;
        }
    }

    void set(int i) { stamp[i] = cur; }
    void reset(int i) { stamp[i] = 0; }
    bool test(int i) const { return stamp[i] == cur; }
};
