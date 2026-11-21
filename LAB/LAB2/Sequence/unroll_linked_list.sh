#!/usr/bin/env bash
set -euo pipefail

project_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
bench_dir=$(mktemp -d)
trap 'rm -rf -- "$bench_dir"' EXIT

if (( $# == 0 )); then
    set -- 50000 100000 250000 500000 1000000
fi

cat > "$bench_dir/unroll_linked_list_bench.cpp" <<'CPP'
#include <chrono>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "src/unroll_linked_list.hpp"

using Clock = std::chrono::steady_clock;
using Nanoseconds = std::chrono::duration<double, std::nano>;

constexpr int samples = 512;
constexpr int small_chunk = 64;
constexpr int large_chunk = 256;
constexpr int min_size = 2 * large_chunk;

struct Times {
    Nanoseconds insert{0};
    Nanoseconds remove{0};
};

template <class List>
void fill_list(List& list, int size) {
    for (int i = 0; i < size; ++i) {
        list.push_back(i);
    }
}

template <class List>
void check_state(const List& list, std::size_t size, std::size_t chunks) {
    if (list.size() != size || list.chunk_count() != chunks) {
        throw std::logic_error("unexpected element or chunk count");
    }
}

template <int ChunkSize>
Times benchmark_indexed(int size, const std::vector<int>& positions, bool split_full_chunk) {
    UnrollLinkedList<int, ChunkSize> list;
    fill_list(list, size);
    const auto original_chunks = list.chunk_count();
    const auto element_count = static_cast<std::size_t>(size);
    const auto size_before_insert = split_full_chunk ? element_count : element_count - 1;
    const auto chunks_after_insert = split_full_chunk ? original_chunks + 1 : original_chunks;

    Times times;
    for (int position : positions) {
        // Stay inside the left half so removing the inserted value merges the two halves again.
        const int index = (position / ChunkSize) * ChunkSize + ChunkSize / 4;
        if (!split_full_chunk) {
            list.remove(index);
            check_state(list, size_before_insert, original_chunks);
        }
        // Check the prepared value and visit the indexed path before timing either workload.
        const int expected_value = split_full_chunk ? index : index + 1;
        if (list.query(index) != expected_value) {
            throw std::logic_error("unexpected value at the prepared index");
        }

        const auto insert_start = Clock::now();
        list.insert(index, -1);
        times.insert += Clock::now() - insert_start;
        check_state(list, size_before_insert + 1, chunks_after_insert);

        const auto remove_start = Clock::now();
        list.remove(index);
        times.remove += Clock::now() - remove_start;
        check_state(list, size_before_insert, original_chunks);

        if (!split_full_chunk) {
            list.insert(index, index);
            check_state(list, size, original_chunks);
        }
    }
    return times;
}

template <int ChunkSize>
Times benchmark_block_ends(int size) {
    UnrollLinkedList<int, ChunkSize> list;
    fill_list(list, size);
    const auto original_chunks = list.chunk_count();

    Times times;
    for (int sample = 0; sample < samples; ++sample) {
        const auto insert_start = Clock::now();
        list.push_front(-1);
        times.insert += Clock::now() - insert_start;
        check_state(list, static_cast<std::size_t>(size) + 1, original_chunks + 1);

        const auto remove_start = Clock::now();
        list.pop_front();
        times.remove += Clock::now() - remove_start;
        check_state(list, size, original_chunks);
    }
    return times;
}

template <int ChunkSize>
Nanoseconds benchmark_query(int size, const std::vector<int>& positions) {
    UnrollLinkedList<int, ChunkSize> list;
    fill_list(list, size);

    Nanoseconds time{0};
    for (int index : positions) {
        const auto start = Clock::now();
        const int value = list.query(index);
        asm volatile("" : : "r"(value) : "memory");
        time += Clock::now() - start;
        if (value != index) {
            throw std::logic_error("query returned an unexpected value");
        }
    }
    return time;
}

template <int ChunkSize>
void print_measurements(int size, const std::vector<int>& positions) {
    const auto ordinary = benchmark_indexed<ChunkSize>(size, positions, false);
    const auto split_merge = benchmark_indexed<ChunkSize>(size, positions, true);
    const auto blocks = benchmark_block_ends<ChunkSize>(size);
    const auto query_time = benchmark_query<ChunkSize>(size, positions);
    std::cout << size << ',' << ChunkSize << ','
              << ordinary.insert.count() / samples << ',' << ordinary.remove.count() / samples << ','
              << split_merge.insert.count() / samples << ',' << split_merge.remove.count() / samples << ','
              << blocks.insert.count() / samples << ',' << blocks.remove.count() / samples << ','
              << query_time.count() / positions.size() << std::endl;
}

int main(int argc, char** argv) {
    try {
        std::vector<int> sizes;
        for (int arg = 1; arg < argc; ++arg) {
            const std::string input = argv[arg];
            std::size_t parsed = 0;
            const int size = std::stoi(input, &parsed);
            if (parsed != input.size() || size < min_size) {
                throw std::invalid_argument("expected a size of at least " + std::to_string(min_size));
            }
            sizes.push_back(size);
        }

        std::cout << std::fixed << std::setprecision(2)
                  << "n,chunk_size,insert_ns_per_op,remove_ns_per_op,split_insert_ns_per_op,"
                     "merge_remove_ns_per_op,block_insert_ns_per_op,block_remove_ns_per_op,"
                     "query_ns_per_op" << std::endl;
        for (int size : sizes) {
            std::mt19937 engine(42);
            // Leave a full successor block for either chunk size.
            std::uniform_int_distribution<int> position(0, size - min_size);
            std::vector<int> positions;
            positions.reserve(samples);
            for (int sample = 0; sample < samples; ++sample) {
                positions.push_back(position(engine));
            }

            print_measurements<small_chunk>(size, positions);
            print_measurements<large_chunk>(size, positions);
        }
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
CPP

"${CXX:-g++}" -std=c++17 -O2 -Wall -Wextra -I "$project_dir" \
    "$bench_dir/unroll_linked_list_bench.cpp" -o "$bench_dir/unroll_linked_list_bench"
"$bench_dir/unroll_linked_list_bench" "$@"
