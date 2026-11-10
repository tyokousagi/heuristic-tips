# __SITE_URL__/snippets/#run-tests
"""複数のテストケースを並列に実行し、スコアを集計する。

使い方の例:
    python run_tests.py --cmd ./a.out --in-dir tools/in --out-dir out --cases 100 --jobs 8

スコアは、解答プログラムが標準エラー出力に書いた「Score = 数値」の行から読む。
公式ツールで採点する場合は --scorer に採点コマンドを渡す。{in} と {out} は入出力のファイル名に置き換わる。
    --scorer "./tools/target/release/vis {in} {out}"

ケースごとの最高スコアを --best のファイルに記録し、それを 100 とした相対スコアも表示する。
"""
import argparse
import concurrent.futures
import json
import math
import os
import re
import shlex
import subprocess
import sys
import time

SCORE_RE = re.compile(r"Score\s*=\s*(-?\d+)")


def split_command(cmd):
    # Windows ではコマンドの文字列をそのまま渡す。shlex はパスの \ を取り除いてしまうため
    return cmd if os.name == "nt" else shlex.split(cmd)


def run_case(seed, args):
    in_path = os.path.join(args.in_dir, f"{seed:04d}.txt")
    out_path = os.path.join(args.out_dir, f"{seed:04d}.txt")
    start = time.perf_counter()
    try:
        with open(in_path, "rb") as fin, open(out_path, "wb") as fout:
            proc = subprocess.run(split_command(args.cmd), stdin=fin, stdout=fout,
                                  stderr=subprocess.PIPE, timeout=args.timeout)
    except subprocess.TimeoutExpired:
        return seed, None, args.timeout, "timeout"
    elapsed = time.perf_counter() - start
    if proc.returncode != 0:
        return seed, None, elapsed, f"exit code {proc.returncode}"
    text = proc.stderr.decode(errors="replace")
    if args.scorer:
        cmd = args.scorer.replace("{in}", in_path).replace("{out}", out_path)
        scored = subprocess.run(split_command(cmd), capture_output=True)
        text = scored.stdout.decode(errors="replace") + scored.stderr.decode(errors="replace")
    found = SCORE_RE.findall(text)
    if not found:
        return seed, None, elapsed, "no score"
    return seed, int(found[-1]), elapsed, ""


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cmd", required=True, help="解答プログラムの実行コマンド")
    parser.add_argument("--in-dir", default="tools/in")
    parser.add_argument("--out-dir", default="out")
    parser.add_argument("--cases", type=int, default=100, help="seed 0 から何ケース実行するか")
    parser.add_argument("--jobs", type=int, default=os.cpu_count() or 1)
    parser.add_argument("--timeout", type=float, default=30.0)
    parser.add_argument("--scorer", default="", help="採点コマンド。{in} と {out} を置き換える")
    parser.add_argument("--best", default="best_scores.json")
    parser.add_argument("--minimize", action="store_true", help="スコアが小さいほどよい問題")
    args = parser.parse_args()

    os.makedirs(args.out_dir, exist_ok=True)
    best = {}
    if os.path.exists(args.best):
        with open(args.best, encoding="utf-8") as f:
            best = json.load(f)

    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        results = sorted(pool.map(lambda s: run_case(s, args), range(args.cases)))

    scores, relatives, failed, max_time = [], [], [], 0.0
    for seed, score, elapsed, error in results:
        max_time = max(max_time, elapsed)
        # 0 点以下と、スコアが読めなかったケースは失敗として数える
        if score is None or score <= 0:
            failed.append((seed, error or f"score {score}"))
            continue
        key = str(seed)
        old = best.get(key)
        if old is None or (score < old if args.minimize else score > old):
            best[key] = score
        relatives.append(100.0 * best[key] / score if args.minimize else 100.0 * score / best[key])
        scores.append(score)

    with open(args.best, "w", encoding="utf-8") as f:
        json.dump(best, f)

    n = len(scores)
    print(f"cases        : {args.cases}")
    print(f"accepted     : {n}")
    if n:
        print(f"mean score   : {sum(scores) / n:.1f}")
        print(f"mean log10   : {sum(math.log10(s) for s in scores) / n:.4f}")
        print(f"mean relative: {sum(relatives) / n:.3f}")
    print(f"max time     : {max_time:.3f} s")
    for seed, error in failed:
        print(f"FAILED seed {seed}: {error}")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
