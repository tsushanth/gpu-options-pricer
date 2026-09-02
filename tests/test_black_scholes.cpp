#include "black_scholes.hpp"
#include <cassert>
#include <cmath>
#include <cstdio>

// Reference values are the standard textbook example (Hull):
// S=100, K=100, r=5%, sigma=20%, T=1yr -> call ~= 10.4506, put ~= 5.5735

static void test_known_reference_values() {
    bs::Inputs in{100.0, 100.0, 0.05, 0.20, 1.0};
    double call = bs::call_price(in);
    double put  = bs::put_price(in);

    assert(std::fabs(call - 10.4506) < 1e-3);
    assert(std::fabs(put  - 5.5735)  < 1e-3);
    std::puts("test_known_reference_values OK");
}

static void test_put_call_parity() {
    // C - P = S - K*e^(-rT) must hold exactly (to floating point precision)
    // for any consistent set of inputs -- a strong correctness check.
    bs::Inputs in{142.37, 130.0, 0.03, 0.35, 0.75};
    double call = bs::call_price(in);
    double put  = bs::put_price(in);
    double lhs = call - put;
    double rhs = in.spot - in.strike * std::exp(-in.rate * in.expiry);
    assert(std::fabs(lhs - rhs) < 1e-9);
    std::puts("test_put_call_parity OK");
}

static void test_deep_itm_call_converges_to_intrinsic() {
    // Deep in-the-money, near-zero vol/time: price should approach
    // max(S-K, 0) discounted -- a sanity bound, not just a magic number.
    bs::Inputs in{200.0, 100.0, 0.01, 0.001, 0.001};
    double call = bs::call_price(in);
    double intrinsic = in.spot - in.strike * std::exp(-in.rate * in.expiry);
    assert(std::fabs(call - intrinsic) < 0.5);
    std::puts("test_deep_itm_call_converges_to_intrinsic OK");
}

int main() {
    test_known_reference_values();
    test_put_call_parity();
    test_deep_itm_call_converges_to_intrinsic();
    std::puts("\nall tests passed");
    return 0;
}
