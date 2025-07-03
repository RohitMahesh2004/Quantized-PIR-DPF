// pir_server.cpp
#include "pir_common.h"
#include "dpf_base/dpf.h"
#include <fstream>
#include <vector>
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: ./pir_server <server_id>\n";
        return 1;
    }

    int server_id = std::stoi(argv[1]);
    std::string key_file = "key" + std::to_string(server_id) + ".bin";

    SeedsCodewordsFlat flat;
    std::ifstream in(key_file, std::ios::binary);
    in.read((char*)&flat, sizeof(SeedsCodewordsFlat));

    int N = 128;
    int prf_method = 0;

    std::vector<uint64_t> db(N);
    for (int i = 0; i < N; ++i)
        db[i] = i * 10;

    uint64_t result = 0;
    for (int i = 0; i < N; ++i) {
        uint128_t share = EvaluateFlat(&flat, i, prf_method);
        result += (uint64_t)share * db[i];  // assuming share is 0 or 1
    }

    std::cout << "[Server " << server_id << "] Response: " << result << std::endl;

    return 0;
}
