#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <ctime>

using namespace std;

// Binary Search
int binarySearch(int a[], int n, int key, int &probes) {
    int low = 0, high = n - 1;
    probes = 0;

    while (low <= high) {
        probes++;
        int mid = (low + high) / 2;

        if (a[mid] == key)
            return mid;
        else if (a[mid] < key)
            low = mid + 1;
        else
            high = mid - 1;
    }
    return -1;
}

// Interpolation Search
int interpolationSearch(int a[], int n, int key, int &probes) {
    int low = 0, high = n - 1;
    probes = 0;

    while (low <= high && key >= a[low] && key <= a[high]) {
        probes++;

        if (a[high] == a[low]) {
            if (a[low] == key) return low;
            else return -1;
        }

        int pos = low + (double)(high - low) *
                  (key - a[low]) / (a[high] - a[low]);

        if (a[pos] == key)
            return pos;
        else if (a[pos] < key)
            low = pos + 1;
        else
            high = pos - 1;
    }
    return -1;
}

int main() {
    srand(time(0));

    int sizes[] = {50, 100, 500, 1000, 5000, 10000, 50000, 100000, 1000000, 5000000};
    int numSizes = 10;

    for (int s = 0; s < numSizes; s++) {
        int n = sizes[s];
        int *arr = new int[n];

        // Fill array with random values
        for (int i = 0; i < n; i++)
            arr[i] = rand() % 50000;

        sort(arr, arr + n);

        int totalBinProbes = 0;
        int totalIntProbes = 0;
        int trials = 1000;

        for (int i = 0; i < trials; i++) {
            int key = arr[rand() % n]; // guaranteed to exist

            int probes;
            binarySearch(arr, n, key, probes);
            totalBinProbes += probes;

            interpolationSearch(arr, n, key, probes);
            totalIntProbes += probes;
        }

        cout << "n = " << n << endl;
        cout << "  Binary Search avg probes: "
             << (double)totalBinProbes / trials << endl;
        cout << "  Interpolation Search avg probes: "
             << (double)totalIntProbes / trials << endl;
        cout << endl;

        delete[] arr;
    }

    return 0;
}
