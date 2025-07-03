// utils.h
#pragma once
#include <vector>
#include <cstdint>

inline std::vector<uint64_t> generate_database(int N) {
    std::vector<uint64_t> DB(N);
    for (int i = 0; i < N; ++i)
        DB[i] = i * 10;
    return DB;
}