# Raw benchmark log

Unedited command output from every actual run this project's numbers are
based on. The README's comparison table is a summary of this; this file
is the evidence it's a summary *of* — nothing here is projected or
adjusted after the fact.

---

## Machine 1: Jetson Orin Nano (`jetson-02`, CUDA 13.2, R39)

```
$ nvidia-smi -L
GPU 0: Orin (nvgpu) (UUID: a9485f58-0a6e-5a97-b73d-87cdf435c65e)

$ ./build/cpu_price 1000 1000
grid:        1000 strikes x 1000 expiries = 1000000 contracts
cpu time:    0.0728 s
throughput:  13740843 prices/sec (single-threaded)
sample:      price[0]=50.0749 price[last]=16.3938

$ ./build/gpu_price 1000 1000
grid:        1000 strikes x 1000 expiries = 1000000 contracts
gpu time:    0.1207 s (kernel only, excludes H2D/D2H)
throughput:  8285527 prices/sec
sample:      price[0]=50.0749 price[last]=16.3938

$ ./build/cpu_price 5000 5000
grid:        5000 strikes x 5000 expiries = 25000000 contracts
cpu time:    1.7025 s
throughput:  14684489 prices/sec (single-threaded)
sample:      price[0]=50.0749 price[last]=16.3910

$ ./build/gpu_price 5000 5000
grid:        5000 strikes x 5000 expiries = 25000000 contracts
gpu time:    0.4368 s (kernel only, excludes H2D/D2H)
throughput:  57230631 prices/sec
sample:      price[0]=50.0749 price[last]=16.3910

$ ./build/cpu_price 10000 10000
grid:        10000 strikes x 10000 expiries = 100000000 contracts
cpu time:    6.7715 s
throughput:  14767839 prices/sec (single-threaded)
sample:      price[0]=50.0749 price[last]=16.3906

$ ./build/gpu_price 10000 10000
grid:        10000 strikes x 10000 expiries = 100000000 contracts
gpu time:    1.7433 s (kernel only, excludes H2D/D2H)
throughput:  57363139 prices/sec
sample:      price[0]=50.0749 price[last]=16.3906
```

**Result: 3.9x speedup at 100M contracts. GPU slower than CPU at 1M contracts (0.6x).**

---

## Machine 2: RunPod, RTX A4000 (rented, pod `s5bh2p5c1ty0mn`, terminated after use)

```
$ nvidia-smi -L
GPU 0: NVIDIA RTX A4000 (UUID: GPU-269d2ad4-3547-a23a-63e0-b91ef52b9fef)

$ ./build/cpu_price 316 316
grid:        316 strikes x 316 expiries = 99856 contracts
cpu time:    0.0099 s
throughput:  10040196 prices/sec (single-threaded)
sample:      price[0]=50.0749 price[last]=4.0060

$ ./build/gpu_price 316 316
grid:        316 strikes x 316 expiries = 99856 contracts
gpu time:    0.0759 s (kernel only, excludes H2D/D2H)
throughput:  1315060 prices/sec
sample:      price[0]=50.0749 price[last]=4.0060

$ ./build/cpu_price 5000 5000
grid:        5000 strikes x 5000 expiries = 25000000 contracts
cpu time:    2.2248 s
throughput:  11236785 prices/sec (single-threaded)
sample:      price[0]=50.0749 price[last]=16.3910

$ ./build/gpu_price 5000 5000
grid:        5000 strikes x 5000 expiries = 25000000 contracts
gpu time:    0.0476 s (kernel only, excludes H2D/D2H)
throughput:  525325918 prices/sec
sample:      price[0]=50.0749 price[last]=16.3910

$ ./build/cpu_price 10000 10000
grid:        10000 strikes x 10000 expiries = 100000000 contracts
cpu time:    8.5213 s
throughput:  11735335 prices/sec (single-threaded)
sample:      price[0]=50.0749 price[last]=16.3906

$ ./build/gpu_price 10000 10000
grid:        10000 strikes x 10000 expiries = 100000000 contracts
gpu time:    0.1890 s (kernel only, excludes H2D/D2H)
throughput:  529237642 prices/sec
sample:      price[0]=50.0749 price[last]=16.3906
```

**Result: 45.1x speedup at 100M contracts. GPU slower than CPU at 100k contracts (0.13x).**

---

## Machine 3 (failed): Vast.ai, RTX 3080 Ti (rented, instance `49687446`, terminated)

Driver 535.309.01 on this host was too old for the CUDA 12.4 runtime the
binary links against. This ran before `CUDA_CHECK` existed in the
source, so the failure was silent rather than an error — recorded here
precisely because it's a real failure mode worth keeping visible, not
because the numbers below mean anything.

```
$ ./build/cpu_price 316 316
grid:        316 strikes x 316 expiries = 99856 contracts
cpu time:    0.0028 s
throughput:  36082327 prices/sec (single-threaded)
sample:      price[0]=50.0749 price[last]=4.0060

$ ./build/gpu_price 316 316
grid:        316 strikes x 316 expiries = 99856 contracts
gpu time:    0.0000 s (kernel only, excludes H2D/D2H)
throughput:  inf prices/sec
sample:      price[0]=0.0000 price[last]=0.0000   <- all zero: cudaMalloc silently failed

$ /tmp/check   # diagnostic binary written on the spot to find the actual error
cudaMalloc: forward compatibility was attempted on non supported HW
device: , compute cap -2089242072.32764
```

**Result: invalid — hardware/driver mismatch, not a performance number.**
Fixed afterward by adding `CUDA_CHECK` around every CUDA call in
`src/gpu_price.cu`, and by filtering for `cuda_max_good >= 12.4` on the
next rental.

---

## Machine 4: Vast.ai, RTX 3060 Ti (rented, instance `49687724`, terminated)

```
$ nvidia-smi -L
GPU 0: NVIDIA GeForce RTX 3060 Ti (UUID: GPU-6887597a-f39d-1f4a-157f-cb236876ad3c)

$ ./build/cpu_price 316 316
grid:        316 strikes x 316 expiries = 99856 contracts
cpu time:    0.0092 s
throughput:  10806854 prices/sec (single-threaded)
sample:      price[0]=50.0749 price[last]=4.0060

$ ./build/gpu_price 316 316
grid:        316 strikes x 316 expiries = 99856 contracts
gpu time:    0.0005 s (kernel only, excludes H2D/D2H)
throughput:  211530645 prices/sec
sample:      price[0]=50.0749 price[last]=4.0060

$ ./build/cpu_price 5000 5000
grid:        5000 strikes x 5000 expiries = 25000000 contracts
cpu time:    1.5678 s
throughput:  15945999 prices/sec (single-threaded)
sample:      price[0]=50.0749 price[last]=16.3910

$ ./build/gpu_price 5000 5000
grid:        5000 strikes x 5000 expiries = 25000000 contracts
gpu time:    0.0561 s (kernel only, excludes H2D/D2H)
throughput:  445252086 prices/sec
sample:      price[0]=50.0749 price[last]=16.3910

$ ./build/cpu_price 10000 10000
grid:        10000 strikes x 10000 expiries = 100000000 contracts
cpu time:    6.1720 s
throughput:  16202282 prices/sec (single-threaded)
sample:      price[0]=50.0749 price[last]=16.3906

$ ./build/gpu_price 10000 10000
grid:        10000 strikes x 10000 expiries = 100000000 contracts
gpu time:    0.2066 s (kernel only, excludes H2D/D2H)
throughput:  483954711 prices/sec
sample:      price[0]=50.0749 price[last]=16.3906
```

**Result: 29.9x speedup at 100M contracts. GPU faster than CPU even at 100k contracts (19.6x) — unlike every other machine tested.**

---

## Summary table (also in main README)

| Hardware | CPU (100M, single-thread) | GPU (100M) | Speedup @ 100M | Speedup @ 100k |
|---|---|---|---|---|
| Jetson Orin Nano | 14.8M/s | 57.4M/s | 3.9x | 0.6x (slower) |
| RunPod RTX A4000 | 11.7M/s | 529.2M/s | 45.1x | 0.13x (slower) |
| Vast.ai RTX 3060 Ti | 16.2M/s | 484.0M/s | 29.9x | 19.6x (faster) |

Correctness: CPU and GPU `price[0]`/`price[last]` samples match to
displayed precision on every successful run above — cross-checked
across three completely different GPU architectures and driver stacks,
not just once.
