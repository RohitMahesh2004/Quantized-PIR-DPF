// pir_common.h
#ifndef PIR_COMMON_H
#define PIR_COMMON_H

#include <iostream>
#include <fstream>
#include <string>

typedef unsigned __int128 uint128_t;

// Utilities to write/read uint128_t
inline void write_uint128(std::ostream& out, const uint128_t& x) {
    out.write((char*)&x, sizeof(uint128_t));
}

inline uint128_t read_uint128(std::istream& in) {
    uint128_t x;
    in.read((char*)&x, sizeof(uint128_t));
    return x;
}

#endif
