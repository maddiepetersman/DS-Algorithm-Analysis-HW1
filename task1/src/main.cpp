#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <chrono>
#include <climits>
#include <cstdint>

using namespace std;

struct PowStats {
    bool ok;                 // did not overflow
    int64_t value;           // valid only if ok == true
    unsigned int muls;       // number of multiplications performed
};

static inline bool multiply_overflow(int64_t a, int64_t b, int64_t& out) {
    if (a == 0 || b == 0) {
        out = 0;
        return false;
    }

    if (a > 0) {
        if (b > 0 && a > LLONG_MAX / b) return true;
        if (b < 0 && b < LLONG_MIN / a) return true;
    } else {
        if (b > 0 && a < LLONG_MIN / b) return true;
        if (b < 0 && a != 0 && b < LLONG_MAX / a) return true;
    }

    out = a * b;
    return false;
}

// Right-to-Left (binary exponentiation)
PowStats pow_right_to_left(int64_t x, int64_t n) {
    PowStats s{ true, 1, 0 };
    if (n < 0) { s.ok = false; return s; }

    int64_t base = x;
    int64_t result = 1;

    while (n > 0) {
        if (n & 1) {
            int64_t tmp;
            s.muls++;
            if (multiply_overflow(result, base, tmp)) {
                s.ok = false;
                return s;
            }
            result = tmp;
        }

        n >>= 1;
        if (n > 0) {
            int64_t tmp;
            s.muls++;
            if (multiply_overflow(base, base, tmp)) {
                s.ok = false;
                return s;
            }
            base = tmp;
        }
    }

    s.value = result;
    return s;
}

// Left-to-Right (square-and-multiply, MSB -> LSB)
PowStats pow_left_to_right(int64_t x, int64_t n) {
    PowStats s{ true, 1, 0 };
    if (n < 0) { s.ok = false; return s; }
    if (n == 0) { s.value = 1; return s; }

    int64_t msb = 63;
    while (msb > 0 && ((n >> msb) & 1LL) == 0) msb--;

    int64_t result = 1;

    for (int64_t i = msb; i >= 0; --i) {
        int64_t tmp;

        // square
        s.muls++;
        if (multiply_overflow(result, result, tmp)) {
            s.ok = false;
            return s;
        }
        result = tmp;

        // multiply if bit is set
        if ((n >> i) & 1LL) {
            s.muls++;
            if (multiply_overflow(result, x, tmp)) {
                s.ok = false;
                return s;
            }
            result = tmp;
        }
    }

    s.value = result;
    return s;
}


static inline string ll_or_overflow(const PowStats& s) {
    return s.ok ? to_string(s.value) : "OVERFLOW";
}

int main() {
    int64_t x = -2;

    vector<int64_t> num_samples = {
        0, -1, -2, -3, 4, 5, 6, 7, 8, 9,
        10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
        20, 21, 22, 23, 30, 50, 75, 100, 1000
    };

    const int64_t WARMUP = 2000;
    const int64_t REPS   = 20000;

    cout << "Task #1: x^n using Right-to-Left and Left-to-Right\n";
    cout << "x = " << x << "\n\n";

    volatile int64_t sink = 0;

    // Warm-up
    for (int64_t i = 0; i < WARMUP; ++i) {
        auto a = pow_right_to_left(x, 20);
        auto b = pow_left_to_right(x, 20);
        if (a.ok) sink ^= a.value;
        if (b.ok) sink ^= b.value;
    }

    cout << left
         << setw(6)  << "n"
         << setw(20) << "RTL result"
         << setw(20) << "LTR result"
         << setw(10) << "RTL mul"
         << setw(10) << "LTR mul"
         << setw(18) << "RTL avg ns"
         << setw(18) << "LTR avg ns"
         << "\n";

    cout << string(100, '-') << "\n";

    for (int64_t n : num_samples) {
        PowStats rtl = pow_right_to_left(x, n);
        PowStats ltr = pow_left_to_right(x, n);

        auto t1 = chrono::high_resolution_clock::now();
        for (int64_t i = 0; i < REPS; ++i) {
            auto r = pow_right_to_left(x, n);
            if (r.ok) sink ^= r.value;
        }
        auto t2 = chrono::high_resolution_clock::now();

        auto t3 = chrono::high_resolution_clock::now();
        for (int64_t i = 0; i < REPS; ++i) {
            auto r = pow_left_to_right(x, n);
            if (r.ok) sink ^= r.value;
        }
        auto t4 = chrono::high_resolution_clock::now();

        long long rtl_ns =
            chrono::duration_cast<chrono::nanoseconds>(t2 - t1).count() / REPS;
        long long ltr_ns =
            chrono::duration_cast<chrono::nanoseconds>(t4 - t3).count() / REPS;

        cout << setw(6)  << n
             << setw(20) << ll_or_overflow(rtl)
             << setw(20) << ll_or_overflow(ltr)
             << setw(10) << rtl.muls
             << setw(10) << ltr.muls
             << setw(18) << rtl_ns
             << setw(18) << ltr_ns
             << "\n";
    }

    if (sink == 42) cerr << sink << "\n";
    return 0;
}
