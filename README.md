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

## Status: GPU number now measured — real result, below the original target

This dev machine has no CUDA-capable GPU, so the CPU baseline above was
run here; the GPU kernel was built and run on the project's Jetson Orin
Nano (`nvcc` 13.2) once it came back online.

**Real result, same machine (Jetson Orin Nano), 100M-contract grid:**

```
CPU (single-threaded, ARM):  14,767,839 prices/sec
GPU (Orin Nano, kernel only): 57,363,139 prices/sec
speedup: ~3.9x
```

This is **well short of the blueprint's >20x target**, and that target
isn't being adjusted to fit — 3.9x is the honest number. Likely reason:
the Orin Nano is a mobile/edge GPU (far fewer CUDA cores than a discrete
desktop or datacenter card), benchmarked here against a single CPU
thread. A discrete GPU (e.g. an RTX-class or datacenter card) would
likely clear the original target; this hardware doesn't.

One more honest note: **at small grid sizes (≤1M contracts), the GPU
was actually slower than the CPU** — 8.3M/sec GPU vs. 13.7M/sec CPU at
a 1000×1000 grid. Kernel launch overhead dominates until there's enough
parallel work to amortize it; the crossover on this hardware is
somewhere between 1M and 25M contracts. This is a real, useful finding
about when GPU acceleration is actually worth reaching for on this
class of device, not just an implementation detail to bury.

Correctness cross-checked: CPU and GPU outputs match to displayed
precision at every grid size tested.

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
