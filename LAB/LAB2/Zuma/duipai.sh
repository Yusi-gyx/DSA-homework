#!/usr/bin/env bash
# 对拍脚本：把 01.cpp ~ 10.cpp 与 base.cpp 逐个对拍。
#
# 用法：
#   ./duipai.sh
#
# 可用环境变量（均有默认值）：
#   N          对拍轮数                默认 200
#   SEED_BASE  随机种子起始值          默认 1（第 k 轮种子 = SEED_BASE + k）
#   ALPHA      字符集大小              默认 3
#   MAX_LEN    初始序列最大长度         默认 30
#   MAX_M      操作次数上限             默认 20
#   MIN_LEN    初始序列最小长度         默认 0
#   POS_MODE   插入位置分布            默认 uniform（front/back/edge/block）
#   COLOR_MODE 插入颜色分布            默认 uniform（pair）
#   TIMEOUT    单次运行超时（秒）       默认 5
#   CXX        C++ 编译器              默认 g++
#   CXXFLAGS   编译选项                默认 "-O2 -std=c++17"
#
# 结果：失败用例保存在 build/fail/ 下
#   *.in            输入
#   *.out           该程序输出
#   *.base.out      参考输出（base.cpp）
set -u

cd "$(dirname "$0")"

BUILD=build
FAIL_DIR="$BUILD/fail"
mkdir -p "$BUILD" "$FAIL_DIR"

CXX=${CXX:-g++}
CXXFLAGS=${CXXFLAGS:--O2 -std=c++17}
N=${N:-200}
SEED_BASE=${SEED_BASE:-1}
ALPHA=${ALPHA:-3}
MAX_LEN=${MAX_LEN:-30}
MAX_M=${MAX_M:-20}
MIN_LEN=${MIN_LEN:-0}
POS_MODE=${POS_MODE:-uniform}
COLOR_MODE=${COLOR_MODE:-uniform}
TIMEOUT=${TIMEOUT:-5}

# 在后台运行并等待，避免 shell 打印 "Aborted" 之类的信号通知
run() {
    local exe=$1 in=$2 out=$3 err=$4 pid rc
    timeout "$TIMEOUT" "$exe" < "$in" > "$out" 2>"$err" &
    pid=$!
    wait "$pid" 2>/dev/null
    rc=$?
    return "$rc"
}

echo "==> 编译 base.cpp"
$CXX $CXXFLAGS -o "$BUILD/base" base.cpp || { echo "base.cpp 编译失败"; exit 1; }

echo "==> 编译 01.cpp ~ 10.cpp"
progs=()
for i in $(seq -w 1 10); do
    if $CXX $CXXFLAGS -o "$BUILD/p$i" "$i.cpp" 2>"$BUILD/p$i.compile.log"; then
        progs+=("$i")
    else
        echo "    !! $i.cpp 编译失败，已跳过（日志：$BUILD/p$i.compile.log）"
    fi
done

if [ ${#progs[@]} -eq 0 ]; then
    echo "没有任何可对拍的程序，退出。"
    exit 1
fi

echo "==> 开始对拍：N=$N, ALPHA=$ALPHA, MAX_LEN=$MAX_LEN, MAX_M=$MAX_M, POS=$POS_MODE, COLOR=$COLOR_MODE"
echo

total=0
fail=0

for ((k = 1; k <= N; k++)); do
    seed=$((SEED_BASE + k))
    input="$BUILD/case.in"
    ans="$BUILD/case.ans"
    base_out="$BUILD/base.out"

    python3 gen.py "$seed" "$ALPHA" "$MAX_LEN" "$MAX_M" "$ans" \
        "$POS_MODE" "$COLOR_MODE" "$MIN_LEN" > "$input" || {
        echo "    !! 第 $k 轮：生成器失败（seed=$seed）"
        continue
    }

    run "$BUILD/base" "$input" "$base_out" "$BUILD/base.err"
    base_rc=$?
    if [ $base_rc -ne 0 ]; then
        echo "    !! 第 $k 轮：base.cpp 异常退出 rc=$base_rc，跳过该轮"
        continue
    fi

    # 交叉校验 base 是否与生成器模拟结果一致（防御性检查）
    if ! cmp -s "$base_out" "$ans"; then
        echo "    !! 第 $k 轮：base.cpp 与生成器期望结果不一致（seed=$seed）"
        cp "$input" "$FAIL_DIR/base_case${k}.in"
    fi

    for id in "${progs[@]}"; do
        total=$((total + 1))
        out="$BUILD/p$id.out"
        run "$BUILD/p$id" "$input" "$out" "$BUILD/p$id.err"
        rc=$?

        if [ $rc -ne 0 ]; then
            fail=$((fail + 1))
            cp "$input" "$FAIL_DIR/${id}_case${k}_rc${rc}.in"
            echo "    !! 第 $k 轮：$id.cpp 异常退出 rc=$rc -> $FAIL_DIR/${id}_case${k}_rc${rc}.in"
            continue
        fi

        if ! cmp -s "$base_out" "$out"; then
            fail=$((fail + 1))
            cp "$input" "$FAIL_DIR/${id}_case${k}.in"
            cp "$base_out" "$FAIL_DIR/${id}_case${k}.base.out"
            cp "$out" "$FAIL_DIR/${id}_case${k}.out"
            echo "    !! 第 $k 轮：$id.cpp 与 base.cpp 输出不同 -> $FAIL_DIR/${id}_case${k}.in"
        fi
    done

    if [ $((k % 50)) -eq 0 ]; then
        echo "    ... 已完成 $k 轮"
    fi
done

echo
echo "=============================="
echo "对拍结束：共比对 $total 次，异常/差异 $fail 次"
if [ $fail -gt 0 ]; then
    echo "失败用例目录：$FAIL_DIR/"
fi
