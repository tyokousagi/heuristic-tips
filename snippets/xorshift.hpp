// https://tyokousagi.github.io/heuristic-tips/snippets/#xorshift
#pragma once
#include <cstdint>

// xorshift64 による乱数。シードを固定すれば同じ結果を再現できる。
struct Xorshift {
    uint64_t x;

    explicit Xorshift(uint64_t seed = 88172645463325252ULL) : x(seed ? seed : 88172645463325252ULL) {}

    uint64_t next() {
        x ^= x << 13;
        x ^= x >> 7;
        x ^= x << 17;
        return x;
    }

    // [0, n) の整数。剰余を使わず、上位32ビットに n を掛けて求める。
    uint32_t next_int(uint32_t n) { return (uint32_t)(((next() >> 32) * (uint64_t)n) >> 32); }

    // [l, r) の整数。
    int next_int(int l, int r) { return l + (int)next_int((uint32_t)(r - l)); }

    // [0, 1) の実数。
    double next_double() { return (double)(next() >> 11) * (1.0 / 9007199254740992.0); }
};
