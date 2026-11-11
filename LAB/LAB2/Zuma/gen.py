#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Zuma 随机测例生成器（对拍用）。

用法：
    python3 gen.py <seed> [alphabet] [max_len] [max_m] [ans_path]

- seed      随机种子（必填，保证可复现）
- alphabet  字符集大小，1..26，默认 3（越小越容易触发消除）
- max_len   初始序列最大长度，默认 30
- max_m     操作次数上限，默认 20
- ans_path  若提供，把最终的珠子序列写入该文件（用于交叉校验 base）

生成器会模拟 Zuma 消除规则，保证：
  1. 初始序列不存在连续 3 个同色珠子；
  2. 每次插入的位置 pos 都满足 0 <= pos <= 当前长度。

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

    alpha = max(1, min(26, alpha))
    rng = random.Random(seed)
    letters = [chr(ord('A') + i) for i in range(alpha)]

    # 初始序列：无连续 3 个同色
    init_len = rng.randint(0, max_len)
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
        pos = rng.randint(0, len(s))
        c = rng.choice(letters)
        lines.append('%d %c' % (pos, c))
        s = zuma_apply(s, pos, c)

    sys.stdout.write('\n'.join(lines) + '\n')

    if ans_path:
        with open(ans_path, 'w') as f:
            f.write(s + '\n')


if __name__ == '__main__':
    main()
