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

## Status: measured on four different machines — results vary a lot by hardware

This dev machine has no CUDA-capable GPU, so its CPU baseline (top of
this README) was run here. The GPU kernel has since been built and run
for real on three different CUDA machines: the project's Jetson Orin
Nano, a rented RunPod RTX A4000, and a rented Vast.ai RTX 3060 Ti.
**Every number below is measured, not projected** — including the ones
that fall short of the original >20x target.

| Hardware | CPU baseline (same box) | GPU, 100M contracts | Speedup | Speedup at 100k contracts |
|---|---|---|---|---|
| Jetson Orin Nano (edge/mobile GPU) | 14.8M/sec | 57.4M/sec | **3.9x** | GPU *slower* (0.6x) |
| RunPod RTX A4000 (rented, $/hr) | 11.7M/sec | 529.2M/sec | **45.1x** | GPU *slower* (0.13x) |
| Vast.ai RTX 3060 Ti (rented, $/hr) | 16.2M/sec | 484.0M/sec | **29.9x** | GPU *faster* (19.6x) |

Correctness cross-checked at every grid size on every machine — GPU and
CPU outputs match to displayed precision throughout. Raw, unedited
command output from every run (including the failed one) is in
[`results/benchmark_log.md`](results/benchmark_log.md) — this table is
a summary of that, not a replacement for it.

**What actually varies and why:**
- The Jetson's edge/mobile GPU falls well short of the blueprint's
  >20x target; both rented discrete cards clear it comfortably. That
  target was calibrated for real GPU hardware, not an edge device —
  the Jetson number isn't a bug, it's a legitimate finding about what
  class of hardware this workload needs.
- **Small-grid behavior is inconsistent across cards, and that's the
  more interesting result.** On the Jetson and the RunPod A4000, the
  GPU is *slower* than CPU below ~1M contracts — kernel launch overhead
  dominates. On the Vast.ai 3060 Ti, the GPU was already faster at
  100k contracts. This means "how big does the grid need to be before
  GPU is worth it" doesn't have one universal answer — it's a
  per-device number you'd have to measure, not assume, before deciding
  whether to route a given workload to GPU or CPU in a real system.
- A first RunPod-adjacent attempt on a different Vast.ai host failed
  silently (0.0000s, all-zero output) before this — its driver
  (535.309.01) was too old for the CUDA 12.4 runtime this code links
  against ("forward compatibility was attempted on non supported HW").
  The original code had no CUDA error checking, so the failure was
  silent rather than a clear error — worth fixing before trusting this
  kind of benchmark blind on unfamiliar rented hardware in the future.

## Reproducing this

CPU baseline (any machine):
```
clang++ -std=c++17 -O3 -DNDEBUG -Iinclude bench/cpu_price.cpp -o build/cpu_price
./build/cpu_price 10000 10000
```

GPU (on the Jetson or any CUDA machine):
```
nvcc -O3 -Iinclude src/gpu_price.cu -o build/gpu_price
./build/gpu_price 10000 10000
```

Always compare both on the **same machine** — a GPU number from one
machine and a CPU number from another isn't a valid speedup claim.

## Build (CPU-only parts)

```
clang++ -std=c++17 -O3 -DNDEBUG -Iinclude tests/test_black_scholes.cpp -o build/tests
clang++ -std=c++17 -O3 -DNDEBUG -Iinclude bench/cpu_price.cpp -o build/cpu_price
```
