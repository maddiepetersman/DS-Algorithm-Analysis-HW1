#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <queue>
#include <stack>
#include <deque>
#include <utility>
#include <limits>
#include <iomanip>
#include <chrono>
#include <climits>

using namespace std;

using int64 = long long;

struct PowStats {
    bool ok;                 // did not overflow (fits in int64)
    int64 value;             // valid only if ok==true
    unsigned long long muls; // number of multiplications performed
};

// MSVC-safe overflow detection for signed long long
static inline bool mul_overflow_ll(int64 a, int64 b, int64& out) {
    if (a == 0 || b == 0) {
        out = 0;
        return false;
    }

    if (a > 0) {
        if (b > 0) {
            if (a > LLONG_MAX / b) return true;
        } else {
            if (b < LLONG_MIN / a) return true;
        }
    } else {
        if (b > 0) {
            if (a < LLONG_MIN / b) return true;
        } else {
            if (a != 0 && b < LLONG_MAX / a) return true;
        }
    }

    out = a * b;
    return false;
}

// Right-to-Left (binary exponentiation)
PowStats pow_right_to_left(int64 x, long long n) {
    PowStats s{ true, 1, 0 };
    if (n < 0) { s.ok = false; return s; }

    int64 base = x;
    int64 res = 1;

    while (n > 0) {
        if (n & 1LL) {
            int64 tmp;
            s.muls++;
            if (mul_overflow_ll(res, base, tmp)) { s.ok = false; return s; }
            res = tmp;
        }
        n >>= 1LL;
        if (n > 0) {
            int64 tmp;
            s.muls++;
            if (mul_overflow_ll(base, base, tmp)) { s.ok = false; return s; }
            base = tmp;
        }
    }
    s.value = res;
    return s;
}

// Left-to-Right (square-and-multiply, MSB → LSB)
PowStats pow_left_to_right(int64 x, long long n) {
    PowStats s{ true, 1, 0 };
    if (n < 0) { s.ok = false; return s; }
    if (n == 0) { s.value = 1; return s; }

    int msb = 63;
    while (msb > 0 && ((n >> msb) & 1LL) == 0) msb--;

    int64 res = 1;
    for (int i = msb; i >= 0; --i) {
        int64 tmp;
        s.muls++;
        if (mul_overflow_ll(res, res, tmp)) { s.ok = false; return s; }
        res = tmp;

        if ((n >> i) & 1LL) {
            s.muls++;
            if (mul_overflow_ll(res, x, tmp)) { s.ok = false; return s; }
            res = tmp;
        }
    }
    s.value = res;
    return s;
}

static inline string ll_or_overflow(const PowStats& s) {
    if (!s.ok) return "OVERFLOW/INVALID";
    return to_string(s.value);
}

int main(int argc, char** argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int64 x = 7;
    if (argc >= 2) x = stoll(argv[1]);

    vector<long long> ns = {
        0,1,2,3,4,5,6,7,8,9,
        10,11,12,13,14,15,16,17,18,19,
        20,21,22,23
    };

    const int WARMUP = 2000;
    const int REPS   = 20000;

    cout << "Program #1: x^n using Right-to-Left and Left-to-Right\n";
    cout << "x = " << x << "\n\n";

    volatile long long sink = 0;
    for (int i = 0; i < WARMUP; ++i) {
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

    for (long long n : ns) {
        PowStats rtl = pow_right_to_left(x, n);
        PowStats ltr = pow_left_to_right(x, n);

        auto t1 = chrono::high_resolution_clock::now();
        for (int i = 0; i < REPS; ++i) {
            auto r = pow_right_to_left(x, n);
            if (r.ok) sink ^= r.value;
        }
        auto t2 = chrono::high_resolution_clock::now();
        long long rtl_ns =
            chrono::duration_cast<chrono::nanoseconds>(t2 - t1).count() / REPS;

        auto t3 = chrono::high_resolution_clock::now();
        for (int i = 0; i < REPS; ++i) {
            auto r = pow_left_to_right(x, n);
            if (r.ok) sink ^= r.value;
        }
        auto t4 = chrono::high_resolution_clock::now();
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

    if (sink == 42) cerr << "sink=" << sink << "\n";
    return 0;
}
