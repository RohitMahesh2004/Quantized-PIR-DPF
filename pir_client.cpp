#include <iostream>
#include <vector>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include "dpf_base/dpf.h"
#include "utils.h"

uint128_t request_from_server(const char* ip, int port, SeedsCodewordsFlat* sf, int prf_method) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &serv_addr.sin_addr);
    connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr));

    send(sock, (char*)sf, sizeof(*sf), 0); 

    uint128_t result;
    recv(sock, (char*)&result, sizeof(result), 0);
    close(sock);
    return result;
}
void print_uint128(uint128_t x) {
    uint64_t hi = (uint64_t)(x >> 64);
    uint64_t lo = (uint64_t)x;
    std::cout << "[" << hi << ", " << lo << "]" << std::endl;
}
int main() {
    int N = 1024;
    int alpha = 42;
    uint128_t beta = 1;
    int prf_method = 0;

    std::mt19937 g_gen(123);
    SeedsCodewords *s = GenerateSeedsAndCodewordsLog(alpha, beta, N, g_gen, prf_method);

    SeedsCodewordsFlat sf0, sf1;
    FlattenCodewords(s, 0, &sf0);
    FlattenCodewords(s, 1, &sf1);

    uint128_t r0 = request_from_server("127.0.0.1", 9000, &sf0, prf_method);
    uint128_t r1 = request_from_server("127.0.0.1", 9001, &sf1, prf_method);

    uint128_t result = r0 - r1;
    std::cout << "[Client] PIR result = ";
    print_uint128(result);
    auto DB = generate_database(N);
    std::cout << "[Client] Expected = " << DB[alpha] << std::endl;

    FreeSeedsCodewords(s);
    return 0;
}
