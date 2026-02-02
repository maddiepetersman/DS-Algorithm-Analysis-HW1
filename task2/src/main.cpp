#include <iostream>
#include <string>
#include <algorithm>   // for reverse
#include <stdexcept>   // for invalid_argument

using namespace std;

// Convert positive integer p to base b (2..36)
string to_base(int p, int b) {
    const string digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    if (b < 2 || b > 36) throw invalid_argument("Base must be in [2,36]");
    if (p < 0) throw invalid_argument("p must be positive");
    if (p == 0) return "0";

    string out;
    while (p > 0) {
        int r = p % b;
        out.push_back(digits[r]);
        p /= b;
    }

    reverse(out.begin(), out.end());
    return out;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cout << "Program #2: Convert positive integer p to base b (2 <= b <= 36)\n\n";

    int p;
    int b;

    cout << "Enter p (positive integer): ";
    cin >> p;
    cout << "Enter base b (2..36): ";
    cin >> b;

    try {
        string ans = to_base(p, b);
        cout << "\nResult: " << p << " in base " << b << " = " << ans << "\n\n";
    } catch (const exception& e) {
        cout << "Error: " << e.what() << "\n";
    }

    return 0;
}
