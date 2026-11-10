// ビームサーチと chokudaiサーチのテスト。
// 題材: H×W のマスに点数があり、左上から右か下へ進んで右下まで行く。通ったマスの点数の合計を最大化する。
// DP で最適値がわかるので、探索の結果と比べる。
#include <cassert>
#include <cstdio>

#include "../beam_search.hpp"
#include "../chokudai_search.hpp"
#include "../timer.hpp"
#include "../xorshift.hpp"

static int H, W;
static std::vector<std::vector<int>> board;

// #region state
struct PathState {
    using Action = char; // 'R' か 'D'
    int r = 0, c = 0;
    long long sum = 0;

    // 遷移後の評価値と行動を列挙する。状態はコピーしない
    void expand(std::vector<std::pair<double, Action>>& out) const {
        if (c + 1 < W) out.push_back({(double)(sum + board[r][c + 1]), 'R'});
        if (r + 1 < H) out.push_back({(double)(sum + board[r + 1][c]), 'D'});
    }
    void apply(const Action& a) {
        if (a == 'R') ++c; else ++r;
        sum += board[r][c];
    }
    // 同じマスにいる状態を重複とみなす
    uint64_t hash() const { return (uint64_t)r * W + c; }
};
// #endregion state

static long long optimum() {
    std::vector<std::vector<long long>> dp(H, std::vector<long long>(W, 0));
    for (int r = 0; r < H; ++r)
        for (int c = 0; c < W; ++c) {
            long long best = 0;
            if (r > 0) best = std::max(best, dp[r - 1][c]);
            if (c > 0) best = std::max(best, dp[r][c - 1]);
            dp[r][c] = best + ((r == 0 && c == 0) ? 0 : board[r][c]);
        }
    return dp[H - 1][W - 1];
}

static long long score_of(const std::vector<char>& actions) {
    PathState s;
    for (char a : actions) s.apply(a);
    assert(s.r == H - 1 && s.c == W - 1);
    return s.sum;
}

int main() {
    Xorshift rng(3);
    Timer timer;

    // ビームサーチ: 同じマスの状態は評価のよいほうだけが残るので、幅が十分なら最適解と一致する
    H = 30, W = 30;
    board.assign(H, std::vector<int>(W));
    for (auto& row : board)
        for (int& v : row) v = rng.next_int(0, 100);
    const long long opt = optimum();
    const auto wide = beam_search(PathState{}, H + W - 2, 30);
    assert((int)wide.size() == H + W - 2);
    assert(score_of(wide) == opt);
    // 幅 1 は貪欲法。最適値を超えることはない
    const auto greedy = beam_search(PathState{}, H + W - 2, 1);
    assert(score_of(greedy) <= opt);
    std::printf("beam: optimum %lld, width 1 -> %lld\n", opt, score_of(greedy));

    // chokudaiサーチ: 小さい盤面なら全部の経路を展開しきるので、最適解と一致する
    H = 6, W = 6;
    board.assign(H, std::vector<int>(W));
    for (auto& row : board)
        for (int& v : row) v = rng.next_int(0, 100);
    const auto path = chokudai_search(PathState{}, H + W - 2, 1, timer, timer.elapsed() + 5.0);
    assert(score_of(path) == optimum());

    std::puts("test_beam: OK");
    return 0;
}
