// __SITE_URL__/snippets/#normal-prob
#pragma once
#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

// 正規分布 N(mu, sigma^2) の累積分布関数。
inline double normal_cdf(double x, double mu, double sigma) {
    return 0.5 * (1.0 + std::erf((x - mu) / (sigma * std::sqrt(2.0))));
}

// N(mu, sigma^2) に従う値を四捨五入した結果が r になる確率。
// 値に下限 lo があり、lo 未満が lo に切り上げられる場合は lo を渡す。
// 確率が 0 になると対数をとれないので、とても小さい値で下から抑える。
inline double rounded_normal_prob(double r, double mu, double sigma,
                                  double lo = -std::numeric_limits<double>::infinity()) {
    const double upper = normal_cdf(r + 0.5, mu, sigma);
    const double lower = (r <= lo) ? 0.0 : normal_cdf(r - 0.5, mu, sigma);
    return std::max(upper - lower, 1e-300);
}

// 対数尤度の配列を、合計が 1 の確率に直す。最大値を引いてから exp をとり、あふれを防ぐ。
inline void log_likelihood_to_prob(std::vector<double>& v) {
    if (v.empty()) return;
    const double mx = *std::max_element(v.begin(), v.end());
    double sum = 0;
    for (double& x : v) {
        x = std::exp(x - mx);
        sum += x;
    }
    for (double& x : v) x /= sum;
}
