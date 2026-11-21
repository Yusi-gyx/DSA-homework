#!/usr/bin/env bash
set -euo pipefail

project_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
bench_dir=$(mktemp -d)
trap 'rm -rf -- "$bench_dir"' EXIT

if (( $# == 0 )); then
    set -- 50000 100000 250000 500000 1000000
fi

cat > "$bench_dir/linked_list_bench.cpp" <<'CPP'
#include "src/linked_list.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using Clock = std::chrono::steady_clock;
using Nanoseconds = std::chrono::duration<double, std::nano>;
using Positions = std::vector<std::pair<int, int>>;

constexpr int random_pairs = 512;
// Reserve two pool slots for sentinels and one for the random insertion.
constexpr int max_size = (1 << 20) - 3;

template <class List>
std::pair<double, double> benchmark_ends(int size) {
    List list;
    const auto start = Clock::now();
    for (int i = 0; i < size; ++i) {
        list.insert(i, i);
    }
    const auto filled = Clock::now();
    for (int i = 0; i < size; ++i) {
        list.remove(0);
    }
    const auto end = Clock::now();

    if (list.size() != 0 || list.begin() != list.end()) {
        throw std::logic_error("the list should be empty after head removals");
    }
    return {Nanoseconds(filled - start).count() / size,
            Nanoseconds(end - filled).count() / size};
}

template <class List>
std::pair<double, double> benchmark_random_positions(int size, const Positions& positions) {
    List list;
    for (int i = 0; i < size; ++i) {
        list.insert(i, i);
    }

    Nanoseconds insert_time{0};
    Nanoseconds remove_time{0};
    for (const auto& [insert_index, remove_index] : positions) {
        const auto start = Clock::now();
        list.insert(insert_index, -1);
        const auto inserted = Clock::now();
        list.remove(remove_index);
        const auto removed = Clock::now();
        insert_time += inserted - start;
        remove_time += removed - inserted;
    }

    if (list.size() != static_cast<std::size_t>(size)) {
        throw std::logic_error("the list size should be restored after random operations");
    }
    return {insert_time.count() / positions.size(), remove_time.count() / positions.size()};
}

template <class List>
void print_measurements(const char* name, int size, const Positions& positions) {
    const auto [tail_insert, head_remove] = benchmark_ends<List>(size);
    const auto [random_insert, random_remove] = benchmark_random_positions<List>(size, positions);
    std::cout << size << ',' << name << ',' << tail_insert << ',' << head_remove << ','
              << random_insert << ',' << random_remove << std::endl;
}

int main(int argc, char** argv) {
    try {
        std::vector<int> sizes;
        for (int arg = 1; arg < argc; ++arg) {
            const std::string input = argv[arg];
            std::size_t parsed = 0;
            const int size = std::stoi(input, &parsed);
            if (parsed != input.size() || size <= 0 || size > max_size) {
                throw std::invalid_argument("expected a size from 1 to " + std::to_string(max_size));
            }
            sizes.push_back(size);
        }

        std::cout << std::fixed << std::setprecision(2)
                  << "n,container,tail_insert_ns_per_op,head_remove_ns_per_op,"
                     "random_insert_ns_per_op,random_remove_ns_per_op" << std::endl;
        for (int size : sizes) {
            std::mt19937 engine(42);
            std::uniform_int_distribution<int> index(0, size);
            Positions positions;
            positions.reserve(random_pairs);
            for (int i = 0; i < random_pairs; ++i) {
                const int insert_index = index(engine);
                const int remove_index = index(engine);
                positions.emplace_back(insert_index, remove_index);
            }

            print_measurements<PtrLinkedList<int>>("PtrLinkedList", size, positions);
            print_measurements<PoolPtrLinkedList<int>>("PoolPtrLinkedList", size, positions);
            print_measurements<ArrayLinkedList<int>>("ArrayLinkedList", size, positions);
        }
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
CPP

"${CXX:-g++}" -std=c++17 -O2 -Wall -Wextra -I "$project_dir" \
    "$bench_dir/linked_list_bench.cpp" -o "$bench_dir/linked_list_bench"
"$bench_dir/linked_list_bench" "$@"
