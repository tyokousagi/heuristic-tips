// __SITE_URL__/snippets/#beam-search
#pragma once
#include <algorithm>
#include <cstdint>
#include <unordered_set>
#include <utility>
#include <vector>

// 状態をコピーする素直なビームサーチ。評価値は大きいほどよい。
//
// State に必要なもの:
//   using Action = ...;                                              // 行動を表す型
//   void expand(std::vector<std::pair<double, Action>>& out) const;  // (遷移後の評価値, 行動) を列挙する
//   void apply(const Action& a);                                     // 行動を適用する
//   uint64_t hash() const;                                           // 重複の判定に使う値
//
// max_turn 手まで進め、最後に評価値が最もよい状態までの行動の列を返す。
template <class State>
std::vector<typename State::Action> beam_search(const State& init, int max_turn, int beam_width) {
    using Action = typename State::Action;
    struct Node {
        State state;
        int history; // 履歴の木の中での番号。初期状態は -1
    };
    struct Hist {
        int parent;
        Action action;
    };
    struct Cand {
        double eval;
        int parent; // beam の中での番号
        Action action;
    };

    std::vector<Hist> hist; // 行動の履歴を木で持ち、状態ごとのコピーを避ける
    std::vector<Node> beam, next;
    std::vector<Cand> cands;
    std::vector<std::pair<double, Action>> buf;
    std::unordered_set<uint64_t> seen;
    beam.push_back({init, -1});

    for (int turn = 0; turn < max_turn; ++turn) {
        cands.clear();
        for (int i = 0; i < (int)beam.size(); ++i) {
            buf.clear();
            beam[i].state.expand(buf);
            for (auto& [eval, action] : buf) cands.push_back({eval, i, action});
        }
        if (cands.empty()) break;

        // 上位だけを残してから並べる。重複で捨てる分を見込んで、幅の 4 倍まで残す。
        const auto better = [](const Cand& a, const Cand& b) { return a.eval > b.eval; };
        const size_t keep = (size_t)beam_width * 4;
        if (cands.size() > keep) {
            std::nth_element(cands.begin(), cands.begin() + keep, cands.end(), better);
            cands.resize(keep);
        }
        std::sort(cands.begin(), cands.end(), better);

        next.clear();
        seen.clear();
        for (const Cand& c : cands) {
            State s = beam[c.parent].state;
            s.apply(c.action);
            if (!seen.insert(s.hash()).second) continue; // 同じ状態は評価のよいほうだけを残す
            hist.push_back({beam[c.parent].history, c.action});
            next.push_back({std::move(s), (int)hist.size() - 1});
            if ((int)next.size() >= beam_width) break;
        }
        beam.swap(next);
    }

    // 評価の順に入れているので、beam の先頭が最良。履歴を根までたどって行動の列を復元する。
    std::vector<Action> actions;
    for (int h = beam.front().history; h >= 0; h = hist[h].parent) actions.push_back(hist[h].action);
    std::reverse(actions.begin(), actions.end());
    return actions;
}
