#include <iostream>
#include <iomanip>
#include <algorithm>   // sort
#include <random>      // mt19937, uniform_int_distribution
#include <chrono>

using namespace std;

// ------------------------------------------------------------
// Search Functions (return index, -1 if not found)
// probes is passed by reference so we can count iterations
// ------------------------------------------------------------
int binary_search_custom(const int a[], int n, int key, int& probes) {
    int lo = 0, hi = n - 1;
    probes = 0;

    while (lo <= hi) {
        probes++;
        int mid = lo + (hi - lo) / 2;

        if (a[mid] == key) return mid;
        if (a[mid] < key) lo = mid + 1;
        else hi = mid - 1;
    }
    return -1;
}

int interpolation_search_custom(const int a[], int n, int key, int& probes) {
    int lo = 0, hi = n - 1;
    probes = 0;

    while (lo <= hi && key >= a[lo] && key <= a[hi]) {
        probes++;

        // avoid divide by zero if all values in range are the same
        if (a[hi] == a[lo]) {
            if (a[lo] == key) return lo;
            return -1;
        }

        // estimate the likely position
        int pos = lo + (int)((double)(hi - lo) * (key - a[lo]) / (a[hi] - a[lo]));

        // clamp pos just in case rounding pushes it outside bounds
        if (pos < lo) pos = lo;
        if (pos > hi) pos = hi;

        if (a[pos] == key) return pos;
        if (a[pos] < key) lo = pos + 1;
        else hi = pos - 1;
    }

    return -1;
}

// ------------------------------------------------------------
// Array generation
// ------------------------------------------------------------
void fill_random(int a[], int n, mt19937& rng, int maxValue) {
    uniform_int_distribution<int> dist(0, maxValue);
    for (int i = 0; i < n; i++) a[i] = dist(rng);
}

// ------------------------------------------------------------
// Benchmark one search method
// Keeps timing style very similar to your original bench_one()
// ------------------------------------------------------------
void bench_one(const int a[], int n, mt19937& rng,
               bool useInterpolation,
               int queries,
               long long& avg_ns_out,
               double& avg_probes_out,
               double& found_rate_out) {

    uniform_int_distribution<int> pickIndex(0, n - 1);
    uniform_int_distribution<int> coin(0, 1);
    uniform_int_distribution<int> outside(60000000, 80000000);

    volatile int sink = 0;
    int total_probes = 0;
    int found = 0;

    auto t1 = chrono::high_resolution_clock::now();

    for (int i = 0; i < queries; ++i) {
        int key;

        // half the time pick a value that exists in the array
        // half the time pick a value likely not in the array
        if (coin(rng) == 0) key = a[pickIndex(rng)];
        else key = outside(rng);

        int probes = 0;
        int index;

        if (!useInterpolation)
            index = binary_search_custom(a, n, key, probes);
        else
            index = interpolation_search_custom(a, n, key, probes);

        total_probes += probes;
        if (index != -1) found++;
        sink ^= (index + 1);
    }

    auto t2 = chrono::high_resolution_clock::now();
    auto ns = chrono::duration_cast<chrono::nanoseconds>(t2 - t1).count();

    // prevent compiler optimizing everything away
    if (sink == 1234567) cerr << sink << "\n";

    avg_ns_out = (long long)(ns / queries);
    avg_probes_out = (double)total_probes / queries;
    found_rate_out = (double)found / queries;
}

// ------------------------------------------------------------
// Run suite for different sizes
// Still prints the same table style
// ------------------------------------------------------------
void run_suite(const string& label, int maxValue) {
    mt19937 rng(12345);

    // no vector, just a plain array of sizes
    const int sizes[] = {1000, 5000, 10000, 50000, 100000, 250000, 500000};
    const int NUM_SIZES = sizeof(sizes) / sizeof(sizes[0]);

    const int QUERIES = 20000;

    cout << "\n==== " << label << " ====\n";
    cout << left
         << setw(10) << "n"
         << setw(18) << "Bin avg ns"
         << setw(18) << "Int avg ns"
         << setw(18) << "Bin probes"
         << setw(18) << "Int probes"
         << setw(14) << "Bin found"
         << setw(14) << "Int found"
         << "\n";
    cout << string(110, '-') << "\n";

    for (int si = 0; si < NUM_SIZES; si++) {
        int n = sizes[si];

        // allocate plain array
        int* a = new int[n];

        fill_random(a, n, rng, maxValue);
        sort(a, a + n);

        long long bin_ns, itp_ns;
        double bin_probes, itp_probes;
        double bin_found, itp_found;

        bench_one(a, n, rng, false, QUERIES, bin_ns, bin_probes, bin_found);
        bench_one(a, n, rng, true,  QUERIES, itp_ns, itp_probes, itp_found);

        cout << setw(10) << n
             << setw(18) << bin_ns
             << setw(18) << itp_ns
             << setw(18) << fixed << setprecision(2) << bin_probes
             << setw(18) << fixed << setprecision(2) << itp_probes
             << setw(14) << fixed << setprecision(2) << bin_found
             << setw(14) << fixed << setprecision(2) << itp_found
             << "\n";

        delete[] a;
    }

    cout << "\nNotes:\n";
    cout << "  Binary search is consistently O(log n) probes.\n";
    cout << "  Interpolation search can be very fast on uniform-ish data,\n";
    cout << "  but can degrade toward O(n) behavior on clustered / skewed data (or many duplicates).\n";
}

// ------------------------------------------------------------
// Main
// ------------------------------------------------------------
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cout << "Program #3: Binary Search vs Interpolation Search\n";
    cout << "Generates sorted arrays of various sizes and benchmarks both searches.\n";
    cout << "Outputs average time (ns/query) and average probes.\n";

    // two distributions (same idea as before)
    run_suite("Uniform-ish (wide range random)", 50000000);
    run_suite("Clustered (narrow range random, many duplicates)", 1000);

    return 0;
}
