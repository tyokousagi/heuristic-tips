// https://tyokousagi.github.io/heuristic-tips/snippets/#log-table
#pragma once
#include <cmath>
#include <cstdint>
#include <vector>

// log(乱数) の表。焼きなましの受理のしきい値 temp * log(r) を、log を呼ばずに求める。
// 使い方: static const LogTable log_table;  threshold = temp * log_table(rng.next());
struct LogTable {
    static constexpr int BITS = 16;
    std::vector<double> table;

    LogTable() : table(1 << BITS) {
        for (int i = 0; i < (1 << BITS); ++i) table[i] = std::log((i + 0.5) / (1 << BITS));
    }

    // r は一様な乱数。下位 16 ビットで表を引く。
    double operator()(uint64_t r) const { return table[r & ((1 << BITS) - 1)]; }
};
