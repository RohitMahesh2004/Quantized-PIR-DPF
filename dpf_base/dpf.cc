
// Copyright (c) Meta Platforms, Inc. and affiliates. All Rights Reserved.

/*
  Serial CPU dpf function based on the sqrt(n) grid trick described
  - https://www.youtube.com/watch?v=y2aVgxD7DJc
  - https://www.iacr.org/archive/eurocrypt2014/84410245/84410245.pdf
 */

#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <assert.h>
#include <random>
#include <memory>
#include <vector>
#include "dpf.h"

/*void test_custom_case(int N, int alpha, uint128_t beta, int prf_method) {
  std::mt19937 g_gen(42);  // fixed seed for reproducibility

  std::cout << "\nCUSTOM TEST:" << std::endl;
  std::cout << "N = " << N << ", alpha = " << alpha << ", beta = " << (uint64_t)beta << std::endl;

  // Generate DPF keys
  SeedsCodewords *s = GenerateSeedsAndCodewordsLog(alpha, beta, N, g_gen, prf_method);

  // Evaluate both shares
  for (int i = 0; i < N; i++) {
    uint128_t v0 = Evaluate(s, i, 0, prf_method);
    uint128_t v1 = Evaluate(s, i, 1, prf_method);
    uint128_t diff = v0 - v1;
    uint128_t expected = (i == alpha) ? beta : 0;
    if (diff != expected) {
      std::cerr << "Mismatch at i=" << i << ": got " << (uint64_t)diff << ", expected " << (uint64_t)expected << std::endl;
      exit(1);
    }
  }

  std::cout << "Custom test passed!" << std::endl;

  FreeSeedsCodewords(s);
}
int main(int argc, char *argv[]) {
 test_log_n_method();
  test_sqrt_n_method();
  benchmark_log_n_method_perf();
  test_flat_codewords();

  // 🔧 Add your own cases here:
  test_custom_case(128, 5, 999, 0);    // Small N, custom alpha & beta
  test_custom_case(1024, 100, 42, 0);  // Medium size
  test_custom_case(2048, 0, 1, 0);     // Edge case: alpha = 0
}*/


