#include "black_scholes.hpp"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <vector>

// Single-threaded CPU baseline: prices every (strike, expiry) pair in a
// grid, one at a time. This is the number gpu_price.cu's kernel should
// be compared against on a CUDA-capable machine (see README) -- the
// speedup only means something measured on the same machine.
int main(int argc, char** argv) {
    const int n_strikes = argc > 1 ? std::atoi(argv[1]) : 1000;
    const int n_expiries = argc > 2 ? std::atoi(argv[2]) : 1000;
    const size_t n = static_cast<size_t>(n_strikes) * n_expiries;

    const double spot = 100.0, rate = 0.03;
    std::vector<double> prices(n);

    auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < n_strikes; ++i) {
        double strike = 50.0 + 100.0 * i / n_strikes;       // 50..150
        for (int j = 0; j < n_expiries; ++j) {
            double expiry = 0.05 + 2.0 * j / n_expiries;    // 0.05y..2y
            double vol = 0.15 + 0.35 * ((i + j) % 100) / 100.0; // synthetic vol surface
            bs::Inputs in{spot, strike, rate, vol, expiry};
            prices[i * n_expiries + j] = bs::call_price(in);
        }
    }
    auto t1 = std::chrono::steady_clock::now();
    double seconds = std::chrono::duration<double>(t1 - t0).count();

    std::printf("grid:        %d strikes x %d expiries = %zu contracts\n", n_strikes, n_expiries, n);
    std::printf("cpu time:    %.4f s\n", seconds);
    std::printf("throughput:  %.0f prices/sec (single-threaded)\n", n / seconds);
    std::printf("sample:      price[0]=%.4f price[last]=%.4f\n", prices.front(), prices.back());
    return 0;
}
