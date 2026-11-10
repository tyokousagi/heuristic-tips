// __SITE_URL__/snippets/#timer
#pragma once
#include <chrono>

// 経過時間を秒で返すタイマー。プログラムの先頭で1つ作る。
struct Timer {
    std::chrono::steady_clock::time_point start;

    Timer() : start(std::chrono::steady_clock::now()) {}

    double elapsed() const {
        return std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    }
};
