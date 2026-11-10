// __SITE_URL__/snippets/#hash-set
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

// 64 ビットのハッシュ値をそのままキーにする、開番地法の集合。挿入と一括クリアだけを行う。
// ビームサーチの重複除去に使う。表の大きさは、入れる要素数の 4 倍以上にする。
struct HashSet {
    std::vector<uint64_t> keys;
    std::vector<uint32_t> stamp;
    uint32_t cur = 1;
    uint64_t mask;

    // 表の大きさは 2^capacity_log2
    explicit HashSet(int capacity_log2)
        : keys((size_t)1 << capacity_log2), stamp((size_t)1 << capacity_log2, 0),
          mask(((uint64_t)1 << capacity_log2) - 1) {}

    void clear() {
        if (++cur == 0) {
            std::fill(stamp.begin(), stamp.end(), 0);
            cur = 1;
        }
    }

    // 新しく入れたら true、すでにあれば false を返す。
    bool insert(uint64_t key) {
        uint64_t i = key & mask;
        while (stamp[i] == cur) {
            if (keys[i] == key) return false;
            i = (i + 1) & mask;
        }
        stamp[i] = cur;
        keys[i] = key;
        return true;
    }

    bool contains(uint64_t key) const {
        uint64_t i = key & mask;
        while (stamp[i] == cur) {
            if (keys[i] == key) return true;
            i = (i + 1) & mask;
        }
        return false;
    }
};
