// __SITE_URL__/snippets/#chokudai-search
#pragma once
#include <algorithm>
#include <queue>
#include <utility>
#include <vector>

// chokudaiサーチ。幅 width の細いビームサーチを、制限時間まで繰り返す。評価値は大きいほどよい。
//
// State に必要なもの（beam_search と同じ。hash は使わない）:
//   using Action = ...;
//   void expand(std::vector<std::pair<double, Action>>& out) const;
//   void apply(const Action& a);
//
// timer.elapsed() が time_limit を超えるか、展開する状態がなくなったら終了する。
template <class State, class Timer>
std::vector<typename State::Action> chokudai_search(const State& init, int max_turn, int width, const Timer& timer,
                                                    double time_limit) {
    using Action = typename State::Action;
    struct Node {
        double eval;
        State state;
        int history;
        bool operator<(const Node& o) const { return eval < o.eval; }
    };
    struct Hist {
        int parent;
        Action action;
    };

    std::vector<Hist> hist;
    std::vector<std::priority_queue<Node>> pq(max_turn + 1); // 深さごとのキュー
    std::vector<std::pair<double, Action>> buf;
    pq[0].push({0.0, init, -1});

    bool expanded = true;
    while (expanded && timer.elapsed() < time_limit) {
        expanded = false;
        for (int t = 0; t < max_turn; ++t) {
            for (int k = 0; k < width && !pq[t].empty(); ++k) {
                Node node = pq[t].top();
                pq[t].pop();
                expanded = true;
                buf.clear();
                node.state.expand(buf);
                for (auto& [eval, action] : buf) {
                    State s = node.state;
                    s.apply(action);
                    hist.push_back({node.history, action});
                    pq[t + 1].push({eval, std::move(s), (int)hist.size() - 1});
                }
            }
        }
    }

    // 最も深い段階にある、評価値が最もよい状態を答えにする。
    std::vector<Action> actions;
    for (int t = max_turn; t >= 0; --t) {
        if (pq[t].empty()) continue;
        for (int h = pq[t].top().history; h >= 0; h = hist[h].parent) actions.push_back(hist[h].action);
        break;
    }
    std::reverse(actions.begin(), actions.end());
    return actions;
}
