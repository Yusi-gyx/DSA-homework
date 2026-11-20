#!/usr/bin/env bash
set -euo pipefail

sizes=(20000 100000 500000)
ratios=(5/4 3/2 2/1 3/1)

bench_dir=$(mktemp -d)
trap 'rm -rf -- "$bench_dir"' EXIT

cat > "$bench_dir/vector_bench.cpp" <<'CPP'
#include "src/vector.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    using Clock = std::chrono::steady_clock;
    using Nanoseconds = std::chrono::duration<double, std::nano>;

    for (int arg = 1; arg < argc; ++arg) {
        const int n = std::stoi(argv[arg]);
        std::array<double, 5> push_times{}, pop_times{};

        for (std::size_t run = 0; run < push_times.size(); ++run) {
            Vector<int, GROWTH_NUM, GROWTH_DEN> v;

            const auto start = Clock::now();
            for (int i = 0; i < n; ++i) v.push_back(i);
            asm volatile("" : : "r"(v.begin()) : "memory");
            const auto filled = Clock::now();

            for (int i = 0; i < n; ++i) v.pop_back();
            asm volatile("" : : "r"(v.begin()) : "memory");
            const auto end = Clock::now();

            push_times[run] = Nanoseconds(filled - start).count() / n;
            pop_times[run] = Nanoseconds(end - filled).count() / n;
        }

        std::sort(push_times.begin(), push_times.end());
        std::sort(pop_times.begin(), pop_times.end());

        Vector<int, GROWTH_NUM, GROWTH_DEN> counted;
        int grow_count = 0;
        int shrink_count = 0;
        for (int i = 0; i < n; ++i) {
            const auto previous_capacity = counted.capacity();
            counted.push_back(i);
            if (counted.capacity() > previous_capacity) {
                ++grow_count;
            }
        }
        for (int i = 0; i < n; ++i) {
            const auto previous_capacity = counted.capacity();
            counted.pop_back();
            if (counted.capacity() < previous_capacity) {
                ++shrink_count;
            }
        }

        std::cout << n << ',' << GROWTH_NUM << '/' << GROWTH_DEN << ','
                  << push_times[2] << ',' << pop_times[2] << ','
                  << grow_count << ',' << shrink_count << '\n';
    }
}
CPP

printf '%s\n' 'n,ratio,push_ns_per_op,pop_ns_per_op,grow_count,shrink_count'
for ratio in "${ratios[@]}"; do
    IFS=/ read -r num den <<< "$ratio"
    "${CXX:-g++}" -std=c++17 -O2 -I . \
        "-DGROWTH_NUM=$num" "-DGROWTH_DEN=$den" \
        "$bench_dir/vector_bench.cpp" -o "$bench_dir/vector_bench"
    "$bench_dir/vector_bench" "${sizes[@]}"
done
