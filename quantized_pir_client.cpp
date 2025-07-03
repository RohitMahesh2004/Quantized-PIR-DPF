#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <random>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <Highs.h>
#include "dpf_base/dpf.h"
#include "utils.h"

constexpr double INF = 1e20;

int round_up_to_pow2(int x) {
    return pow(2, ceil(log2(x)));
}

std::vector<int> solve_lp(int n, int k, int lambda, const std::vector<double>& alpha) {
    Highs highs;
    HighsLp lp;

    lp.num_col_ = k;
    lp.num_row_ = 2;

    lp.col_cost_.resize(k);
    for (int i = 0; i < k; ++i)
        lp.col_cost_[i] = -alpha[i];

    lp.col_lower_ = std::vector<double>(k, 0.0);
    lp.col_upper_ = std::vector<double>(k, INF);

    lp.row_lower_ = {static_cast<double>(n), static_cast<double>(lambda)};
    lp.row_upper_ = {static_cast<double>(n), static_cast<double>(lambda)};

    lp.a_matrix_.format_ = MatrixFormat::kColwise;
    lp.a_matrix_.start_.resize(k + 1);
    lp.a_matrix_.index_.resize(2 * k);
    lp.a_matrix_.value_.resize(2 * k);

    for (int j = 0; j < k; ++j) {
        lp.a_matrix_.start_[j] = 2 * j;
        lp.a_matrix_.index_[2 * j] = 0;
        lp.a_matrix_.index_[2 * j + 1] = 1;
        lp.a_matrix_.value_[2 * j] = 1.0;
        lp.a_matrix_.value_[2 * j + 1] = static_cast<double>(j + 1);
    }
    lp.a_matrix_.start_[k] = 2 * k;

    highs.passModel(lp);
    HighsStatus status = highs.run();

    std::vector<int> sizes(k, 0);
    if (status == HighsStatus::kOk) {
        auto sol = highs.getSolution().col_value;
        for (int i = 0; i < k; ++i)
            sizes[i] = std::round(sol[i]);
    } else {
        std::cerr << "[LP ERROR] Failed to solve with HiGHS.\n";
        exit(1);
    }

    return sizes;
}

uint128_t request_from_server(const char* ip, int port, SeedsCodewordsFlat* sf, int start, int N) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &serv_addr.sin_addr);
    connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr));

    send(sock, (char*)sf, sizeof(*sf), 0);
    send(sock, &start, sizeof(start), 0);  // Send offset
    send(sock, &N, sizeof(N), 0);          // Send domain size

    uint128_t result;
    recv(sock, (char*)&result, sizeof(result), 0);
    close(sock);
    return result;
}

int find_interval(int index, const std::vector<std::pair<int, int>>& intervals) {
    for (int j = 0; j < intervals.size(); ++j) {
        if (index >= intervals[j].first && index <= intervals[j].second)
            return j;
    }
    return -1;
}

int main() {
    int n = 100000;
    std::cout << "Enter number of intervals k: ";
    int k; std::cin >> k;

    std::cout << "Enter computational budget lambda: ";
    int lambda; std::cin >> lambda;

    std::vector<double> alpha(k);
    std::cout << "Enter " << k << " weights α₁ to αₖ:\n";
    for (int i = 0; i < k; ++i)
        std::cin >> alpha[i];

    std::vector<int> sizes = solve_lp(n, k, lambda, alpha);

    std::vector<std::pair<int, int>> intervals;
    int curr = 0;
    for (int i = 0; i < k; ++i) {
        int l = sizes[i];
        intervals.emplace_back(curr, curr + l - 1);
        curr += l;
    }

    std::cout << "\nOptimized Intervals:\n";
    for (int i = 0; i < k; ++i)
        std::cout << "I_" << i + 1 << " = [" << intervals[i].first << ", " << intervals[i].second
                  << "] (size " << sizes[i] << ")\n";

    int target_index;
    std::cout << "\nEnter the index to query (target_index): ";
    std::cin >> target_index;

    int interval_id = find_interval(target_index, intervals);
    if (interval_id == -1) {
        std::cerr << "[ERROR] Index not covered by any interval.\n";
        return 1;
    }

    int start = intervals[interval_id].first;
    int end   = intervals[interval_id].second;
    int width = end - start + 1;
    int N = round_up_to_pow2(width);

    std::vector<uint64_t> subDB(N, 0);
    auto fullDB = generate_database(n);
    for (int i = 0; i < width; ++i)
        subDB[i] = fullDB[start + i];

    int offset = target_index - start;
    uint128_t beta = 1;
    int prf_method = 0;

    std::mt19937 g_gen(123);
    SeedsCodewords* s = GenerateSeedsAndCodewordsLog(offset, beta, N, g_gen, prf_method);

    SeedsCodewordsFlat sf0, sf1;
    FlattenCodewords(s, 0, &sf0);
    FlattenCodewords(s, 1, &sf1);

    uint128_t r0 = request_from_server("127.0.0.1", 9000, &sf0, start, N);
    uint128_t r1 = request_from_server("127.0.0.1", 9001, &sf1, start, N);

    uint128_t result = r0 - r1;

    std::cout << "\n[PIR] Queried Interval I_" << interval_id + 1
              << " = [" << start << ", " << end << "]\n";
    std::cout << "[PIR] PIR result = " << (uint64_t)result << "\n";
    std::cout << "[PIR] Expected   = " << fullDB[target_index] << "\n";

    double P_j = log2((interval_id + 1) * width);
    std::cout << "[PIR] Quantized Privacy Level (P_j) = " << P_j << " bits\n";

    FreeSeedsCodewords(s);
    return 0;
}
