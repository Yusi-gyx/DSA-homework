#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""对拍统计工具：测量每份代码"首次出现错误"所需要的对拍次数。

它复用 gen.py 的生成逻辑（同一份代码，保证与命令行生成器完全一致），
用 base 的输出作为标准答案，统计在若干独立试验中第几次生成的测例
让程序首次出错，并给出均值/标准差。

用法示例：
    python3 bench.py --progs 01,02,04,05,08,09,10 --trials 10 --K 200
    python3 bench.py --progs 06 --alpha 26 --len 3000 --m 3000 --pos front \
                     --trials 10 --K 20
"""
import argparse
import importlib.util
import os
import statistics
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
BUILD = os.path.join(HERE, 'build')


def load_gen():
    spec = importlib.util.spec_from_file_location('zumagen', os.path.join(HERE, 'gen.py'))
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def run(exe, data, timeout=10):
    try:
        p = subprocess.run([exe], input=data.encode(), stdout=subprocess.PIPE,
                           stderr=subprocess.DEVNULL, timeout=timeout)
        return p.returncode, p.stdout.decode('utf-8', 'replace')
    except subprocess.TimeoutExpired:
        return 'TLE', ''


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--progs', default='01,02,04,05,08,09,10')
    ap.add_argument('--alpha', type=int, default=2)
    ap.add_argument('--len', type=int, default=30)
    ap.add_argument('--min-len', type=int, default=0)
    ap.add_argument('--m', type=int, default=20)
    ap.add_argument('--pos', default='uniform')
    ap.add_argument('--color', default='uniform')
    ap.add_argument('--trials', type=int, default=10)
    ap.add_argument('--K', type=int, default=200)
    ap.add_argument('--seed-base', type=int, default=0)
    ap.add_argument('--quiet', action='store_true')
    args = ap.parse_args()

    gen = load_gen()
    progs = [p.strip() for p in args.progs.split(',') if p.strip()]

    exe_base = os.path.join(BUILD, 'base')
    exes = {p: os.path.join(BUILD, 'p' + p) for p in progs}
    for e in [exe_base] + list(exes.values()):
        if not os.path.exists(e):
            sys.exit('缺少可执行文件：%s（先运行 duipai.sh 或手动编译）' % e)

    results = {p: [] for p in progs}   # 记录每次试验首次失败的 k（K+1 表示未发现）
    types = {p: [] for p in progs}

    for t in range(args.trials):
        base_seed = args.seed_base + t * 100000
        pending = set(progs)
        first = {p: None for p in progs}
        for k in range(1, args.K + 1):
            data, ans = gen.generate(base_seed + k, args.alpha, args.len, args.m,
                                     args.pos, args.color, args.min_len)
            rc_base, out_base = run(exe_base, data)
            if rc_base != 0 or out_base != ans + '\n':
                # base 与生成器模拟不一致：跳过该轮（正常不应发生）
                continue
            for p in list(pending):
                rc, out = run(exes[p], data)
                bad = (rc != 0) or (out != out_base)
                if bad:
                    kind = 'RE/TLE' if rc != 0 else 'WA'
                    first[p] = (k, str(rc), kind)
                    results[p].append(k)
                    types[p].append(kind)
                    pending.discard(p)
            if not pending:
                break
        for p in pending:
            results[p].append(args.K + 1)
            types[p].append('none')
        if not args.quiet:
            done = {p: (first[p][0] if first[p] else None) for p in progs}
            print('trial %2d (seed_base=%d): %s' % (t + 1, base_seed, done), flush=True)

    print()
    print('参数：alpha=%d len=%d..%d m=%d pos=%s color=%s trials=%d K=%d'
          % (args.alpha, args.min_len, args.len, args.m, args.pos, args.color,
             args.trials, args.K))
    print('%-4s %8s %8s %8s %8s %8s  %s'
          % ('prog', 'found', 'mean', 'std', 'min', 'max', 'types'))
    for p in progs:
        xs = results[p]
        found = [x for x in xs if x <= args.K]
        if found:
            mean = statistics.mean(found)
            std = statistics.pstdev(found) if len(found) > 1 else 0.0
            print('%-4s %5d/%-3d %8.1f %8.1f %8d %8d  %s'
                  % (p, len(found), args.trials, mean, std, min(found), max(found),
                     ','.join(sorted(set(types[p])))))
        else:
            print('%-4s %5d/%-3d %8s %8s %8s %8s  %s'
                  % (p, 0, args.trials, '-', '-', '-', '-',
                     ','.join(sorted(set(types[p])))))


if __name__ == '__main__':
    main()
