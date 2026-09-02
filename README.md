# gpu-options-pricer

Phase 2 of a scoped market-making systems project: Black-Scholes options
pricing across a strike/expiry grid, priced in parallel on GPU and
benchmarked against a single-threaded CPU baseline. This is the
"embarrassingly parallel" workload the project's blueprint identifies as
a legitimate fit for GPU — unlike the matching engine in Phase 1, there's
no data dependency between grid points.

## Design

- `include/black_scholes.hpp` — closed-form Black-Scholes call/put
  pricing, written once and shared between the CPU and GPU builds via a
  `__host__ __device__` guard that only activates under `nvcc`.
- `bench/cpu_price.cpp` — single-threaded CPU baseline: prices an
  `n_strikes x n_expiries` grid serially.
- `src/gpu_price.cu` — the same grid, one CUDA thread per (strike,
  expiry) pair.
- `tests/test_black_scholes.cpp` — correctness against known textbook
  reference values, put-call parity, and a deep-ITM sanity bound.

## Status: GPU number not yet measured

This machine has no CUDA-capable GPU, and the project's Jetson Orin Nano
(the intended CUDA target) was unreachable over SSH when this was
built. So:

- **Verified and run here:** correctness tests (all passing), CPU
  baseline benchmark.
- **Written but not run:** `src/gpu_price.cu` — correct CUDA code,
  compiles against the same shared header, but no speedup number is
  claimed until it's actually measured on CUDA hardware.

## CPU baseline (measured on this machine)

```
$ ./build/cpu_price 1000 1000
grid:        1000 strikes x 1000 expiries = 1000000 contracts
cpu time:    0.0376 s
throughput:  26,596,128 prices/sec (single-threaded)
```

## Running the GPU side (on a CUDA machine, e.g. the Jetson)

```
nvcc -O3 -Iinclude src/gpu_price.cu -o build/gpu_price
./build/gpu_price 1000 1000
```

Compare its `throughput` line against `cpu_price`'s **on the same
machine** — a GPU number from one machine and a CPU number from another
isn't a valid speedup claim. The blueprint's target is >20x speedup on a
100k-contract grid; that target is unverified until this step runs.

## Build (CPU-only parts)

```
clang++ -std=c++17 -O3 -DNDEBUG -Iinclude tests/test_black_scholes.cpp -o build/tests
clang++ -std=c++17 -O3 -DNDEBUG -Iinclude bench/cpu_price.cpp -o build/cpu_price
```
