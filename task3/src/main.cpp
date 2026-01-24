#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>    // for sort, reverse
#include <random>       // for mt19937, uniform_int_distribution
#include <chrono>
#include <functional>   // for std::function
#include <cmath>        // for double arithmetic

using namespace std;

// --- Structures ---
struct SearchResult {
    int index;                 // -1 if not found
    unsigned long long probes; // iterations/probes performed
};

struct BenchSummary {
    double avg_ns;
    double avg_probes;
    double found_rate;
};

// --- Search Functions ---
SearchResult binary_search_custom(const vector<int>& a, int key) {
    long long lo = 0, hi = (long long)a.size() - 1;
    unsigned long long probes = 0;

    while (lo <= hi) {
        probes++;
        long long mid = lo + (hi - lo) / 2;
        if (a[mid] == key) return {(int)mid, probes};
        if (a[mid] < key) lo = mid + 1;
        else hi = mid - 1;
    }
    return {-1, probes};
}

SearchResult interpolation_search_custom(const vector<int>& a, int key) {
    long long lo = 0, hi = (long long)a.size() - 1;
    unsigned long long probes = 0;

    while (lo <= hi && key >= a[lo] && key <= a[hi]) {
        probes++;

        if (a[hi] == a[lo]) { // avoid divide by zero
            if (a[lo] == key) return {(int)lo, probes};
            else return {-1, probes};
        }

        long long pos = lo + (long long)((double)(hi - lo) * (key - a[lo]) / (a[hi] - a[lo]));
        if (pos < lo) pos = lo;
        if (pos > hi) pos = hi;

        if (a[pos] == key) return {(int)pos, probes};
        if (a[pos] < key) lo = pos + 1;
        else hi = pos - 1;
    }
    return {-1, probes};
}

// --- Array Generators ---
vector<int> make_uniformish(size_t n, mt19937& rng) {
    uniform_int_distribution<int> dist(0, 50000000);
    vector<int> a(n);
    for (size_t i = 0; i < n; ++i) a[i] = dist(rng);
    sort(a.begin(), a.end());
    return a;
}

vector<int> make_clustered(size_t n, mt19937& rng) {
    uniform_int_distribution<int> dist(0, 1000);
    vector<int> a(n);
    for (size_t i = 0; i < n; ++i) a[i] = dist(rng);
    sort(a.begin(), a.end());
    return a;
}

// --- Benchmark ---
BenchSummary bench_one(const vector<int>& a, mt19937& rng,
                       function<SearchResult(const vector<int>&, int)> searchFn,
                       int queries) {
    uniform_int_distribution<int> pickIndex(0, (int)a.size() - 1);
    uniform_int_distribution<int> coin(0, 1);
    uniform_int_distribution<int> outside(60000000, 80000000);

    volatile int sink = 0;
    unsigned long long total_probes = 0;
    int found = 0;

    auto t1 = chrono::high_resolution_clock::now();
    for (int i = 0; i < queries; ++i) {
        int key;
        if (coin(rng) == 0) key = a[pickIndex(rng)]; // present
        else key = outside(rng);                     // likely absent

        auto r = searchFn(a, key);
        total_probes += r.probes;
        if (r.index != -1) found++;
        sink ^= (r.index + 1);
    }
    auto t2 = chrono::high_resolution_clock::now();
    auto ns = chrono::duration_cast<chrono::nanoseconds>(t2 - t1).count();

    if (sink == 1234567) cerr << sink << "\n";

    BenchSummary s;
    s.avg_ns = (double)ns / (double)queries;
    s.avg_probes = (double)total_probes / (double)queries;
    s.found_rate = (double)found / (double)queries;
    return s;
}

// --- Run Suite ---
void run_suite(const string& label,
               function<vector<int>(size_t, mt19937&)> gen) {
    mt19937 rng(12345);

    vector<size_t> sizes = {1000, 5000, 10000, 50000, 100000, 250000, 500000};
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

    for (size_t n : sizes) {
        auto a = gen(n, rng);

        auto bin = bench_one(a, rng, binary_search_custom, QUERIES);
        auto itp = bench_one(a, rng, interpolation_search_custom, QUERIES);

        cout << setw(10) << n
             << setw(18) << (long long)bin.avg_ns
             << setw(18) << (long long)itp.avg_ns
             << setw(18) << fixed << setprecision(2) << bin.avg_probes
             << setw(18) << fixed << setprecision(2) << itp.avg_probes
             << setw(14) << fixed << setprecision(2) << bin.found_rate
             << setw(14) << fixed << setprecision(2) << itp.found_rate
             << "\n";
    }

    cout << "\nNotes:\n";
    cout << "  Binary search is consistently O(log n) probes.\n";
    cout << "  Interpolation search can be very fast on uniform-ish data,\n";
    cout << "  but can degrade toward O(n) behavior on clustered / skewed data (or many duplicates).\n";
}

// --- Main ---
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cout << "Program #3: Binary Search vs Interpolation Search\n";
    cout << "Generates sorted arrays of various sizes and benchmarks both searches.\n";
    cout << "Outputs average time (ns/query) and average probes.\n";

    run_suite("Uniform-ish (wide range random)", make_uniformish);
    run_suite("Clustered (narrow range random, many duplicates)", make_clustered);

    return 0;
}
