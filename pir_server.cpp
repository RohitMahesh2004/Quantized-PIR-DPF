#include <iostream>
#include <vector>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include "dpf_base/dpf.h"
#include "utils.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: ./pir_server <server_id>" << std::endl;
        return 1;
    }
    int server_id = atoi(argv[1]);  // 0 or 1

    int port = 9000 + server_id;
    int N = 1024;
    auto DB = generate_database(N);
    int prf_method = 0;

    // Setup socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    bind(server_fd, (sockaddr*)&address, sizeof(address));
    listen(server_fd, 1);
    std::cout << "[Server " << server_id << "] Listening on port " << port << std::endl;

    int addrlen = sizeof(address);
    int client_fd = accept(server_fd, (sockaddr*)&address, (socklen_t*)&addrlen);
    std::cout << "[Server " << server_id << "] Connected to client." << std::endl;

    // Receive flat seed structure
    SeedsCodewordsFlat flat;
    recv(client_fd, &flat, sizeof(flat), 0);

    // Evaluate
    uint128_t sum = 0;
    for (int i = 0; i < N; ++i) {
        uint128_t val = EvaluateFlat(&flat, i, prf_method);
        sum += val * DB[i];
    }

    // Send result
    send(client_fd, &sum, sizeof(sum), 0);
    std::cout << "[Server " << server_id << "] Response: " << (uint64_t)sum << std::endl;

    close(client_fd);
    close(server_fd);
    return 0;
}
