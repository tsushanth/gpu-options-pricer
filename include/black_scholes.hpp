#pragma once
#include <cmath>

// Shared between CPU (plain g++/clang++) and GPU (nvcc) builds: the
// __host__ __device__ qualifiers only mean something under nvcc, so
// they're guarded out entirely for a normal C++ compiler.
#ifdef __CUDACC__
#define BS_QUALIFIERS __host__ __device__
#else
#define BS_QUALIFIERS
#endif

namespace bs {

// Standard normal CDF via erf — no external stats library needed.
BS_QUALIFIERS inline double norm_cdf(double x) {
    return 0.5 * erfc(-x * M_SQRT1_2);
}

struct Inputs {
    double spot;
    double strike;
    double rate;       // risk-free rate, annualized
    double vol;         // volatility, annualized
    double expiry;      // time to expiry, years
};

// Closed-form Black-Scholes European call price.
BS_QUALIFIERS inline double call_price(const Inputs& in) {
    double sqrtT = sqrt(in.expiry);
    double d1 = (log(in.spot / in.strike) + (in.rate + 0.5 * in.vol * in.vol) * in.expiry)
                / (in.vol * sqrtT);
    double d2 = d1 - in.vol * sqrtT;
    return in.spot * norm_cdf(d1) - in.strike * exp(-in.rate * in.expiry) * norm_cdf(d2);
}

BS_QUALIFIERS inline double put_price(const Inputs& in) {
    double sqrtT = sqrt(in.expiry);
    double d1 = (log(in.spot / in.strike) + (in.rate + 0.5 * in.vol * in.vol) * in.expiry)
                / (in.vol * sqrtT);
    double d2 = d1 - in.vol * sqrtT;
    return in.strike * exp(-in.rate * in.expiry) * norm_cdf(-d2) - in.spot * norm_cdf(-d1);
}

} // namespace bs
