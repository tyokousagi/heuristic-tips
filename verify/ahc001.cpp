// AHC001 AtCoder Ad の検証。
// 10000×10000 に n 個の長方形を重ならないように置く。企業 i は点を含む面積 r_i を希望し、
// 満足度は 1 − (1 − min(r,s)/max(r,s))^2。初期解は各点の 1×1。
// 同じ時間で近傍の違いを比べる（どれも辺を d 動かす。d は 1〜1000 を対数一様に選ぶ）。
//   expand   : 山登り。辺を外へ伸ばすだけ
//   move     : 山登り。伸ばす＋長方形の平行移動
//   sa       : 焼きなまし。伸ばす・縮める・平行移動
//   push     : 焼きなまし。sa に加えて「伸ばした先にある長方形を、その分だけ縮める」近傍
// 使い方: g++ -std=c++20 -O2 ahc001.cpp -o ahc001 && ./ahc001 [ケース数=20] [1手法あたりの秒数=0.4]
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <set>
#include <vector>

struct Rect { int x1, y1, x2, y2; };
struct Case { int n; std::vector<int> x, y; std::vector<long long> r; };

Case generate(uint64_t seed) {
    std::mt19937_64 rng(seed);
    std::uniform_real_distribution<double> u(0, 1);
    Case c;
    c.n = (int)std::lround(50 * std::pow(4.0, u(rng)));
    std::set<std::pair<int, int>> used;
    while ((int)c.x.size() < c.n) {
        int x = rng() % 10000, y = rng() % 10000;
        if (used.insert({x, y}).second) { c.x.push_back(x); c.y.push_back(y); }
    }
    std::set<long long> qs;
    while ((int)qs.size() < c.n - 1) qs.insert(1 + rng() % 99999999);
    std::vector<long long> q{0};
    for (auto v : qs) q.push_back(v);
    q.push_back(100000000);
    for (int i = 0; i < c.n; i++) c.r.push_back(q[i + 1] - q[i]);
    return c;
}

double sat(long long r, const Rect& a) {
    long long s = (long long)(a.x2 - a.x1) * (a.y2 - a.y1);
    double t = 1.0 - (double)std::min(r, s) / std::max(r, s);
    return 1.0 - t * t;
}

bool overlap(const Rect& a, const Rect& b) {
    return std::max(a.x1, b.x1) < std::min(a.x2, b.x2) && std::max(a.y1, b.y1) < std::min(a.y2, b.y2);
}

double solve(const Case& c, int mode, double seconds, uint64_t seed) {
    std::mt19937_64 rng(seed);
    int n = c.n;
    std::vector<Rect> R(n);
    std::vector<double> p(n);
    for (int i = 0; i < n; i++) { R[i] = {c.x[i], c.y[i], c.x[i] + 1, c.y[i] + 1}; p[i] = sat(c.r[i], R[i]); }
    auto valid = [&](int i, const Rect& a, int skip = -1) {
        if (a.x1 < 0 || a.y1 < 0 || a.x2 > 10000 || a.y2 > 10000) return false;
        if (!(a.x1 <= c.x[i] && c.x[i] < a.x2 && a.y1 <= c.y[i] && c.y[i] < a.y2)) return false;
        for (int j = 0; j < n; j++)
            if (j != i && j != skip && overlap(a, R[j])) return false;
        return true;
    };
    auto start = std::chrono::steady_clock::now();
    bool sa = mode >= 2;
    double t0 = 0.02, t1 = 1e-5, temp = t0;
    std::uniform_real_distribution<double> u(0, 1);
    for (long long it = 0;; it++) {
        if ((it & 255) == 0) {
            double el = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
            if (el > seconds) break;
            temp = t0 * std::pow(t1 / t0, el / seconds);
        }
        int i = rng() % n;
        int d = (int)std::exp(u(rng) * std::log(1000.0));
        int side = rng() % 4;  // 0:左 1:右 2:上 3:下
        int kind = rng() % 4;  // 0,1: 伸ばす 2: 縮める 3: 平行移動
        if (mode == 0) kind = 0;
        if (mode == 1 && kind == 2) kind = 0;
        Rect a = R[i];
        if (kind <= 1 || kind == 2) {
            int s = kind == 2 ? -d : d;
            if (side == 0) a.x1 -= s; else if (side == 1) a.x2 += s; else if (side == 2) a.y1 -= s; else a.y2 += s;
            if (a.x1 >= a.x2 || a.y1 >= a.y2) continue;
        } else {
            int dx = side == 0 ? -d : side == 1 ? d : 0, dy = side == 2 ? -d : side == 3 ? d : 0;
            a.x1 += dx; a.x2 += dx; a.y1 += dy; a.y2 += dy;
        }
        if (valid(i, a)) {
            double np = sat(c.r[i], a);
            double delta = np - p[i];
            if (delta >= 0 || (sa && u(rng) < std::exp(delta / temp))) { R[i] = a; p[i] = np; }
            continue;
        }
        // push: 伸ばした先で重なる長方形を、重ならない位置まで縮める
        if (mode != 3 || kind > 1) continue;
        if (a.x1 < 0 || a.y1 < 0 || a.x2 > 10000 || a.y2 > 10000) continue;
        std::vector<std::pair<int, Rect>> changed;
        bool ok = true;
        for (int j = 0; j < n && ok; j++) {
            if (j == i || !overlap(a, R[j])) continue;
            Rect b = R[j];
            if (side == 0) b.x2 = a.x1; else if (side == 1) b.x1 = a.x2; else if (side == 2) b.y2 = a.y1; else b.y1 = a.y2;
            if (b.x1 >= b.x2 || b.y1 >= b.y2 || !(b.x1 <= c.x[j] && c.x[j] < b.x2 && b.y1 <= c.y[j] && c.y[j] < b.y2)) ok = false;
            else changed.push_back({j, b});
        }
        if (!ok) continue;
        // 縮めた長方形どうしと、伸ばした長方形が他と重ならないかを確かめる
        std::vector<Rect> old;
        for (auto& [j, b] : changed) { old.push_back(R[j]); R[j] = b; }
        bool fine = valid(i, a);
        double delta = sat(c.r[i], a) - p[i];
        for (auto& [j, b] : changed) delta += sat(c.r[j], b) - p[j];
        if (fine && (delta >= 0 || u(rng) < std::exp(delta / temp))) {
            R[i] = a; p[i] = sat(c.r[i], a);
            for (auto& [j, b] : changed) p[j] = sat(c.r[j], b);
        } else {
            for (size_t k = 0; k < changed.size(); k++) R[changed[k].first] = old[k];
        }
    }
    double s = 0;
    for (int i = 0; i < n; i++) s += sat(c.r[i], R[i]);
    return s / n;
}

int main(int argc, char** argv) {
    int cases = argc > 1 ? std::atoi(argv[1]) : 20;
    double sec = argc > 2 ? std::atof(argv[2]) : 0.4;
    const char* names[] = {"expand", "move", "sa", "push"};
    double s[4] = {};
    for (int k = 0; k < cases; k++) {
        Case c = generate(11000 + k);
        for (int m = 0; m < 4; m++) s[m] += solve(c, m, sec, k);
    }
    std::printf("cases=%d seconds=%.2f\n", cases, sec);
    for (int m = 0; m < 4; m++) std::printf("%-7s 満足度の平均 %.2f%%\n", names[m], 100 * s[m] / cases);
}
