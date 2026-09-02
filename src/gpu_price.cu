#include "black_scholes.hpp"
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <cuda_runtime.h>

// Every CUDA call in this file is wrapped in this -- a real failure
// (e.g. driver too old for the CUDA runtime this was built against)
// otherwise fails SILENTLY: cudaMalloc returns garbage, the kernel
// never runs, and the output is all zeros with no error message. That
// exact failure mode happened once on a rented GPU host during this
// project's benchmarking before this check existed.
#define CUDA_CHECK(call) do { \
    cudaError_t err = (call); \
    if (err != cudaSuccess) { \
        std::fprintf(stderr, "CUDA error at %s:%d: %s\n", __FILE__, __LINE__, \
                     cudaGetErrorString(err)); \
        std::exit(1); \
    } \
} while (0)

// GPU counterpart to bench/cpu_price.cpp: one CUDA thread per
// (strike, expiry) grid point, all priced in parallel. This is the
// embarrassingly-parallel workload from the blueprint -- unlike the
// matching engine, there's no data dependency between grid points, so
// it's a legitimate fit for the GPU rather than a forced one.
//
// NOT RUN as part of this project's benchmark numbers -- there was no
// CUDA-capable GPU reachable when this was written (see README). Build
// and run this on the Jetson (or any CUDA machine) to get a real,
// honest speedup number against bench/cpu_price's output on the SAME
// machine -- comparing GPU-here vs CPU-there is not a valid comparison.

__global__ void price_grid_kernel(double spot, double rate,
                                   int n_strikes, int n_expiries,
                                   double* out_prices) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;  // strike index
    int j = blockIdx.y * blockDim.y + threadIdx.y;  // expiry index
    if (i >= n_strikes || j >= n_expiries) return;

    double strike = 50.0 + 100.0 * i / n_strikes;
    double expiry = 0.05 + 2.0 * j / n_expiries;
    double vol = 0.15 + 0.35 * ((i + j) % 100) / 100.0;

    bs::Inputs in{spot, strike, rate, vol, expiry};
    out_prices[i * n_expiries + j] = bs::call_price(in);
}

int main(int argc, char** argv) {
    const int n_strikes = argc > 1 ? std::atoi(argv[1]) : 1000;
    const int n_expiries = argc > 2 ? std::atoi(argv[2]) : 1000;
    const size_t n = static_cast<size_t>(n_strikes) * n_expiries;
    const size_t bytes = n * sizeof(double);

    double* d_prices;
    CUDA_CHECK(cudaMalloc(&d_prices, bytes));

    dim3 block(16, 16);
    dim3 grid((n_strikes + block.x - 1) / block.x, (n_expiries + block.y - 1) / block.y);

    cudaEvent_t start, stop;
    CUDA_CHECK(cudaEventCreate(&start));
    CUDA_CHECK(cudaEventCreate(&stop));

    CUDA_CHECK(cudaEventRecord(start));
    price_grid_kernel<<<grid, block>>>(100.0, 0.03, n_strikes, n_expiries, d_prices);
    CUDA_CHECK(cudaGetLastError());       // catches a bad launch (e.g. config/arch issue)
    CUDA_CHECK(cudaEventRecord(stop));
    CUDA_CHECK(cudaEventSynchronize(stop)); // catches a fault during kernel execution

    float ms = 0;
    CUDA_CHECK(cudaEventElapsedTime(&ms, start, stop));

    std::vector<double> h_prices(n);
    CUDA_CHECK(cudaMemcpy(h_prices.data(), d_prices, bytes, cudaMemcpyDeviceToHost));

    std::printf("grid:        %d strikes x %d expiries = %zu contracts\n", n_strikes, n_expiries, n);
    std::printf("gpu time:    %.4f s (kernel only, excludes H2D/D2H)\n", ms / 1000.0);
    std::printf("throughput:  %.0f prices/sec\n", n / (ms / 1000.0));
    std::printf("sample:      price[0]=%.4f price[last]=%.4f\n", h_prices.front(), h_prices.back());

    cudaFree(d_prices);
    cudaEventDestroy(start);
    cudaEventDestroy(stop);
    return 0;
}
