#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""把一个错例化简到数据规模更小、但错误模式相同的版本。

用法：
    python3 minimize.py <程序编号> <错例文件> [错误类型]

错误类型（可省略，自动检测）：
    RE   程序非正常结束（rc != 0 / 超时）
    WA   程序正常结束但输出与 base 不同

化简动作（每次尝试一个，接受能让测例变小的动作）：
    1. 删掉一条操作；
    2. 删掉初始序列中的一个字符；
    3. 把两种颜色合并成一种（保持合法：初始序列不能有 3 连）。
每一步都用 base 校验测例合法（base 正常结束），并校验目标程序仍然
以相同错误类型失败。
"""
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
BUILD = os.path.join(HERE, 'build')


def run(exe, data, timeout=10):
    try:
        p = subprocess.run([exe], input=data.encode(), stdout=subprocess.PIPE,
                           stderr=subprocess.DEVNULL, timeout=timeout)
        return p.returncode, p.stdout.decode('utf-8', 'replace')
    except subprocess.TimeoutExpired:
        return 'TLE', ''


def parse(text):
    lines = text.split('\n')
    init = lines[0]
    m = int(lines[1])
    ops = []
    for i in range(m):
        a, b = lines[2 + i].split()
        ops.append((int(a), b))
    return init, ops


def render(init, ops):
    return '\n'.join([init, str(len(ops))] + ['%d %s' % o for o in ops]) + '\n'


def stable(s):
    for i in range(2, len(s)):
        if s[i] == s[i - 1] == s[i - 2]:
            return False
    return True


def fails_like(prog_exe, base_exe, text, kind):
    rc_b, out_b = run(base_exe, text)
    if rc_b != 0:
        return False                     # 测例本身不合法
    rc, out = run(prog_exe, text)
    if kind == 'RE':
        return rc != 0
    return rc == 0 and out != out_b


def size(init, ops):
    return len(init) + len(ops)


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        sys.exit(1)
    pid = sys.argv[1]
    path = sys.argv[2]
    kind = sys.argv[3] if len(sys.argv) > 3 else None

    prog = os.path.join(BUILD, 'p' + pid)
    base = os.path.join(BUILD, 'base')

    with open(path) as f:
        text = f.read()
    init, ops = parse(text)

    if kind is None:
        rc_b, out_b = run(base, text)
        rc, out = run(prog, text)
        kind = 'RE' if rc != 0 else 'WA'
    if not fails_like(prog, base, render(init, ops), kind):
        print('该错例在 %s 上不是预期的 %s 失败模式' % (pid, kind))
        sys.exit(1)

    print('初始规模：初始串 %d 字符 + %d 次操作（错误类型 %s）'
          % (len(init), len(ops), kind))
    print('原始测例：')
    print(text)

    changed = True
    while changed:
        changed = False

        # 1) 尝试删掉操作
        for i in range(len(ops)):
            cand_ops = ops[:i] + ops[i + 1:]
            cand = render(init, cand_ops)
            if size(init, cand_ops) < size(init, ops) and fails_like(prog, base, cand, kind):
                ops = cand_ops
                changed = True
                break
        if changed:
            continue

        # 2) 尝试删掉初始串中的字符
        for i in range(len(init)):
            cand_init = init[:i] + init[i + 1:]
            if len(cand_init) >= 2 and not stable(cand_init):
                continue
            cand = render(cand_init, ops)
            if fails_like(prog, base, cand, kind):
                init = cand_init
                changed = True
                break
        if changed:
            continue

        # 3) 尝试合并颜色
        colors = sorted(set(init) | {c for _, c in ops})
        for i in range(len(colors)):
            for j in range(len(colors)):
                if i == j:
                    continue
                a, b = colors[i], colors[j]
                cand_init = init.replace(a, b)
                if not stable(cand_init):
                    continue
                cand_ops = [(p, b if c == a else c) for p, c in ops]
                cand = render(cand_init, cand_ops)
                if fails_like(prog, base, cand, kind):
                    init, ops = cand_init, cand_ops
                    changed = True
                    break
            if changed:
                break
        if changed:
            continue

    # 颜色重命名，让输出更易读（按出现顺序映射到 A、B、C…）
    mapping = {}
    for ch in init + ''.join(c for _, c in ops):
        if ch not in mapping:
            mapping[ch] = chr(ord('A') + len(mapping))
    init2 = ''.join(mapping[c] for c in init)
    ops2 = [(p, mapping[c]) for p, c in ops]
    out = render(init2, ops2)
    rc_b, out_b = run(base, out)
    rc, o = run(prog, out)
    print('最小化结果：初始串 %d 字符 + %d 次操作（rc=%s，base=%r，%s=%r）'
          % (len(init2), len(ops2), rc, out_b, pid, o))
    print(out)


if __name__ == '__main__':
    main()
