// __SITE_URL__/snippets/#sa
#pragma once
#include <cmath>

struct AnnealConfig {
    double time_limit; // 終了時刻（秒）。Timer の経過時間で判定する
    double t0;         // 開始温度
    double t1;         // 終了温度
};

struct AnnealResult {
    double best_score;
    long long iterations;
    long long accepted;
};

// スコアを最大化する焼きなまし。最小化したいときはスコアの符号を反転する。
//
// timer.elapsed()   : 経過時間を秒で返す
// rng.next_double() : [0, 1) の乱数を返す
// propose(threshold): 近傍を1つ選び、スコアの差分を返す。状態はまだ変えない。
//                     差分が threshold 未満だとわかった時点で計算を打ち切り、
//                     threshold より小さい値を返してよい。
// commit()          : 直前の propose が選んだ近傍を状態に反映する。
// on_best()         : 最良解を更新したときに呼ばれる。ここで解を保存する。
template <class Timer, class Rng, class Propose, class Commit, class OnBest>
AnnealResult anneal(const Timer& timer, Rng& rng, AnnealConfig cfg, double init_score,
                    Propose propose, Commit commit, OnBest on_best) {
    AnnealResult res{init_score, 0, 0};
    const double begin = timer.elapsed();
    const double duration = cfg.time_limit - begin;
    if (duration <= 0) return res;

    double score = init_score;
    double temp = cfg.t0;
    while (true) {
        // 時刻の取得は重いので 256 回に 1 回だけ行う
        if ((res.iterations & 255) == 0) {
            const double progress = (timer.elapsed() - begin) / duration;
            if (progress >= 1.0) break;
            temp = cfg.t0 * std::pow(cfg.t1 / cfg.t0, progress); // 指数スケジュール
        }
        ++res.iterations;

        // 受理条件 exp(delta / temp) > r は delta > temp * log(r) と同じ。
        // しきい値を先に決めるので exp を呼ばずに済み、propose の中で打ち切りにも使える。
        const double threshold = temp * std::log(rng.next_double());
        const double delta = propose(threshold);
        if (delta >= threshold) {
            commit();
            score += delta;
            ++res.accepted;
            if (score > res.best_score) {
                res.best_score = score;
                on_best();
            }
        }
    }
    return res;
}
