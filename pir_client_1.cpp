// pir_client.cpp
#include "pir_common.h"
#include "dpf_base/dpf.h"
#include <fstream>
#include <random>

int main() {
    int N = 128;               // Database size
    int alpha = 5;             // Desired index to query
    int prf_method = 0;        // 0 = DUMMY PRF
    uint128_t beta = 1;        // Set to 1 for unit vector

    std::mt19937 g_gen(0);
    SeedsCodewords* s = GenerateSeedsAndCodewordsLog(alpha, beta, N, g_gen, prf_method);

    SeedsCodewordsFlat flat0, flat1;
    FlattenCodewords(s, 0, &flat0);
    FlattenCodewords(s, 1, &flat1);

    // Write flat0 to key0.bin
    std::ofstream out0("key0.bin", std::ios::binary);
    out0.write((char*)&flat0, sizeof(SeedsCodewordsFlat));

    // Write flat1 to key1.bin
    std::ofstream out1("key1.bin", std::ios::binary);
    out1.write((char*)&flat1, sizeof(SeedsCodewordsFlat));

    std::cout << "[Client] PIR keys generated and written to key0.bin and key1.bin\n";

    return 0;
}
