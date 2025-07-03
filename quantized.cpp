#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <Highs.h>

// Define infinity constant for HiGHS upper bounds if not defined
constexpr double INF = 1e20;

// Generate a simple database
std::vector<uint64_t> generate_database(int N) {
    std::vector<uint64_t> DB(N);
    for (int i = 0; i < N; ++i)
        DB[i] = i * 10;
    return DB;
}

// Solve LP using HiGHS
std::vector<int> solve_lp(int n, int k, int lambda, const std::vector<double>& alpha) {
    Highs highs;
    HighsLp lp;

    lp.num_col_ = k;
    lp.num_row_ = 2;

    // Objective: maximize sum α_j * l_j => minimize -α_j * l_j
    lp.col_cost_.resize(k);
    for (int i = 0; i < k; ++i)
        lp.col_cost_[i] = -alpha[i];

    // Bounds: l_j ≥ 0, no upper bound
    lp.col_lower_ = std::vector<double>(k, 0.0);
    lp.col_upper_ = std::vector<double>(k, INF);

    // Two constraints:
    // Row 1: sum l_j = n
    // Row 2: sum (j+1)*l_j = λ
    lp.row_lower_ = {static_cast<double>(n), static_cast<double>(lambda)};
    lp.row_upper_ = {static_cast<double>(n), static_cast<double>(lambda)};

    // Constraint matrix in column format
    lp.a_matrix_.format_ = MatrixFormat::kColwise;
    lp.a_matrix_.start_.resize(k + 1);
    lp.a_matrix_.index_.resize(2 * k);
    lp.a_matrix_.value_.resize(2 * k);

    for (int j = 0; j < k; ++j) {
        lp.a_matrix_.start_[j] = 2 * j;
        lp.a_matrix_.index_[2 * j] = 0;      // constraint 1: sum l_j
        lp.a_matrix_.index_[2 * j + 1] = 1;  // constraint 2: weighted sum
        lp.a_matrix_.value_[2 * j] = 1.0;
        lp.a_matrix_.value_[2 * j + 1] = static_cast<double>(j + 1);
    }
    lp.a_matrix_.start_[k] = 2 * k;

    // Solve the LP
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

// Return interval that contains index
int find_interval(int index, const std::vector<std::pair<int, int>>& intervals) {
    for (int j = 0; j < intervals.size(); ++j) {
        if (index >= intervals[j].first && index <= intervals[j].second)
            return j;
    }
    return -1;
}

// Server returns sub-DB for interval
std::vector<uint64_t> server_respond(const std::vector<uint64_t>& DB, int start, int end) {
    std::vector<uint64_t> response;
    for (int i = start; i <= end; ++i)
        response.push_back(DB[i]);
    return response;
}

int main() {
    int n, k, lambda;
    std::cout << "Enter DB size n: ";
    std::cin >> n;
    std::cout << "Enter number of intervals k: ";
    std::cin >> k;
    std::cout << "Enter computational budget lambda: ";
    std::cin >> lambda;

    std::vector<double> alpha(k);
    std::cout << "Enter " << k << " weights α₁ to αₖ:\n";
    for (int i = 0; i < k; ++i)
        std::cin >> alpha[i];

    // Solve LP
    std::vector<int> sizes = solve_lp(n, k, lambda, alpha);

    // Build intervals
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

    // Query process
    int target_index;
    std::cout << "\nEnter the index to query (target_index): ";
    std::cin >> target_index;

    auto DB = generate_database(n);
    int interval_id = find_interval(target_index, intervals);

    if (interval_id == -1) {
        std::cerr << "[ERROR] Index " << target_index << " not found in any interval.\n";
        return 1;
    }

    auto [start, end] = intervals[interval_id];
    std::vector<uint64_t> response = server_respond(DB, start, end);
    uint64_t result = response[target_index - start];

    std::cout << "\n[PIR] Queried Interval I_" << interval_id + 1 << " = [" << start << ", " << end << "]\n";
    std::cout << "[PIR] PIR result = " << result << "\n";
    std::cout << "[PIR] Expected   = " << DB[target_index] << "\n";

    // Quantized Privacy Level
    int depth = interval_id + 1;
    int width = end - start + 1;
    double P_j = log2(depth * width);
    std::cout << "[PIR] Quantized Privacy Level (P_j) = " << P_j << " bits\n";

    return 0;
}
