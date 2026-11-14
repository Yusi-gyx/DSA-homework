#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Zuma 随机测例生成器（对拍用）。

用法：
    python3 gen.py <seed> [alphabet] [max_len] [max_m] [ans_path] [pos_mode] [color_mode]

- seed       随机种子（必填，保证可复现）
- alphabet   字符集大小，1..26，默认 3（越小越容易触发消除）
- max_len    初始序列最大长度，默认 30
- max_m      操作次数上限，默认 20
- ans_path   若提供，把最终的珠子序列写入该文件（用于交叉校验 base）
- pos_mode   插入位置的随机方式，默认 uniform
             uniform  在 [0, len] 上均匀分布
             front    偏向序列头部（位置落在前 1/16 长度内）
             back     偏向序列尾部
             edge     一半偏向头部、一半偏向尾部
             block    位置取在 2048 的块边界附近（针对分块实现）
- min_len    初始序列最小长度，默认 0
- color_mode 插入颜色的随机方式，默认 uniform
             uniform  在字母表中均匀选取
             pair     以 2/3 的概率选取插入点相邻珠子的颜色（更容易凑出同色块）

生成器会模拟 Zuma 消除规则，保证：
  1. 初始序列不存在连续 3 个同色珠子；
  2. 每次插入的位置 pos 都满足 0 <= pos <= 当前长度；
  3. 输出的最终顺序与 base.cpp 的规则一致（可用于交叉校验）。

输出格式（与 base.cpp 输入一致）：
  <初始序列>
  <操作次数 m>
  <pos> <color>   (共 m 行)
"""
import sys
import random


def zuma_apply(s, pos, color):
    """在 pos 处插入 color，然后按 Zuma 规则反复消除 >=3 的连续同色块。"""
    s = s[:pos] + color + s[pos:]
    changed = True
    while changed:
        changed = False
        out = []
        i, n = 0, len(s)
        while i < n:
            j = i
            while j < n and s[j] == s[i]:
                j += 1
            if j - i >= 3:
                changed = True
            else:
                out.append(s[i:j])
            i = j
        s = ''.join(out)
    return s


def pick_pos(rng, s, mode):
    n = len(s)
    if n == 0:
        return 0
    win = max(1, n // 16)
    if mode == 'front':
        return rng.randint(0, min(n, win))
    if mode == 'back':
        return n - rng.randint(0, min(n, win))
    if mode == 'edge':
        if rng.random() < 0.5:
            return rng.randint(0, min(n, win))
        return n - rng.randint(0, min(n, win))
    if mode == 'block':
        # 取某个 2048 边界的前后若干位置，专门攻击分块实现
        base = rng.randrange(0, n // 2048 + 1) * 2048
        return max(0, min(n, base + rng.randint(-4, 4)))
    return rng.randint(0, n)


def pick_color(rng, s, pos, letters, mode):
    if mode == 'pair' and s and rng.random() < 2.0 / 3.0:
        # 取插入点左/右邻居的颜色，容易形成 1~2 个同色，进而凑成 3 连
        if pos > 0 and (pos == len(s) or rng.random() < 0.5):
            return s[pos - 1]
        if pos < len(s):
            return s[pos]
    return rng.choice(letters)


def generate(seed, alpha=3, max_len=30, max_m=20,
             pos_mode='uniform', color_mode='uniform', min_len=0):
    """按给定参数生成一组合法测例，返回 (输入文本, 最终珠子序列)。"""
    alpha = max(1, min(26, alpha))
    rng = random.Random(seed)
    letters = [chr(ord('A') + i) for i in range(alpha)]

    # 初始序列：无连续 3 个同色
    min_len = max(0, min(min_len, max_len))
    init_len = rng.randint(min_len, max_len)
    chars = []
    for _ in range(init_len):
        while True:
            c = rng.choice(letters)
            if len(chars) >= 2 and chars[-1] == c == chars[-2]:
                continue
            chars.append(c)
            break
    s = ''.join(chars)

    m = rng.randint(0, max_m)
    lines = [s, str(m)]
    for _ in range(m):
        pos = pick_pos(rng, s, pos_mode)
        c = pick_color(rng, s, pos, letters, color_mode)
        lines.append('%d %c' % (pos, c))
        s = zuma_apply(s, pos, c)

    return '\n'.join(lines) + '\n', s


def main():
    args = sys.argv[1:]
    if not args:
        print(__doc__)
        sys.exit(1)

    seed = int(args[0])
    alpha = int(args[1]) if len(args) > 1 else 3
    max_len = int(args[2]) if len(args) > 2 else 30
    max_m = int(args[3]) if len(args) > 3 else 20
    ans_path = args[4] if len(args) > 4 else None
    pos_mode = args[5] if len(args) > 5 else 'uniform'
    color_mode = args[6] if len(args) > 6 else 'uniform'
    min_len = int(args[7]) if len(args) > 7 else 0

    text, s = generate(seed, alpha, max_len, max_m, pos_mode, color_mode, min_len)
    sys.stdout.write(text)

    if ans_path:
        with open(ans_path, 'w') as f:
            f.write(s + '\n')


if __name__ == '__main__':
    main()
