#include <bits/stdc++.h>
using namespace std;

using u32 = uint32_t;
using clk = chrono::high_resolution_clock;

void quick_sort_wrapper(vector<u32>& a) {
    sort(a.begin(), a.end());
}

void merge_sort_rec(vector<u32>& a, int l, int r, vector<u32>& temp) {
    if (l >= r) return;
    int m = l + (r - l) / 2;

    merge_sort_rec(a, l, m, temp);
    merge_sort_rec(a, m + 1, r, temp);

    int i = l, j = m + 1, k = l;

    while (i <= m && j <= r) {
        if (a[i] <= a[j]) temp[k++] = a[i++];
        else temp[k++] = a[j++];
    }

    while (i <= m) temp[k++] = a[i++];
    while (j <= r) temp[k++] = a[j++];

    for (int p = l; p <= r; ++p) a[p] = temp[p];
}

void merge_sort_wrapper(vector<u32>& a) {
    if (a.empty()) return;
    vector<u32> temp(a.size());
    merge_sort_rec(a, 0, (int)a.size() - 1, temp);
}

void heap_sort_wrapper(vector<u32>& a) {
    make_heap(a.begin(), a.end());
    sort_heap(a.begin(), a.end());
}

static const int TIM_RUN = 32;

void insertion_sort_range(vector<u32>& a, int l, int r) {
    for (int i = l + 1; i <= r; ++i) {
        u32 key = a[i];
        int j = i - 1;
        while (j >= l && a[j] > key) {
            a[j + 1] = a[j];
            --j;
        }
        a[j + 1] = key;
    }
}

void merge_tim(vector<u32>& a, int l, int m, int r) {
    int n1 = m - l + 1;
    int n2 = r - m;

    vector<u32> left(n1), right(n2);

    for (int i = 0; i < n1; ++i) left[i] = a[l + i];
    for (int i = 0; i < n2; ++i) right[i] = a[m + 1 + i];

    int i = 0, j = 0, k = l;

    while (i < n1 && j < n2) {
        if (left[i] <= right[j]) a[k++] = left[i++];
        else a[k++] = right[j++];
    }

    while (i < n1) a[k++] = left[i++];
    while (j < n2) a[k++] = right[j++];
}

void tim_sort_wrapper(vector<u32>& a) {
    int n = (int)a.size();

    for (int i = 0; i < n; i += TIM_RUN) {
        insertion_sort_range(a, i, min(i + TIM_RUN - 1, n - 1));
    }

    for (int size = TIM_RUN; size < n; size *= 2) {
        for (int left = 0; left < n; left += 2 * size) {
            int mid = min(left + size - 1, n - 1);
            int right = min(left + 2 * size - 1, n - 1);
            if (mid < right) merge_tim(a, left, mid, right);
        }
    }
}

void radix_sort_wrapper(vector<u32>& a) {
    if (a.empty()) return;

    vector<u32> out(a.size());
    const int BASE = 256;

    for (int shift = 0; shift < 32; shift += 8) {
        int count[BASE] = {0};

        for (u32 x : a) {
            ++count[(x >> shift) & 0xFF];
        }

        for (int i = 1; i < BASE; ++i) {
            count[i] += count[i - 1];
        }

        for (int i = (int)a.size() - 1; i >= 0; --i) {
            int idx = (a[i] >> shift) & 0xFF;
            out[--count[idx]] = a[i];
        }

        a.swap(out);
    }
}

bool read_binary_file(const string& filename, vector<u32>& data) {
    ifstream in(filename, ios::binary);
    if (!in) return false;

    in.seekg(0, ios::end);
    streamsize bytes = in.tellg();
    in.seekg(0, ios::beg);

    if (bytes < 0 || bytes % sizeof(u32) != 0) return false;

    size_t n = (size_t)bytes / sizeof(u32);
    data.resize(n);

    if (!in.read(reinterpret_cast<char*>(data.data()), bytes)) return false;
    return true;
}

bool write_binary_file(const string& filename, const vector<u32>& data) {
    ofstream out(filename, ios::binary);
    if (!out) return false;

    out.write(reinterpret_cast<const char*>(data.data()),
              (streamsize)(data.size() * sizeof(u32)));
    return (bool)out;
}

bool is_sorted_ok(const vector<u32>& a) {
    for (size_t i = 1; i < a.size(); ++i) {
        if (a[i - 1] > a[i]) return false;
    }
    return true;
}

int main(int argc, char* argv[]) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string inputFile, outputFile, alg;

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "-input" && i + 1 < argc) inputFile = argv[++i];
        else if (arg == "-output" && i + 1 < argc) outputFile = argv[++i];
        else if (arg == "-alg" && i + 1 < argc) alg = argv[++i];
    }

    if (inputFile.empty() || outputFile.empty() || alg.empty()) {
        cerr << "Uso: ./baseline_sorter -input archivo.bin -output salida.bin -alg QUICK|RADIX|TIM|MERGE|HEAP\n";
        return 1;
    }

    vector<u32> data;
    if (!read_binary_file(inputFile, data)) {
        cerr << "Error leyendo archivo de entrada\n";
        return 1;
    }

    auto start = clk::now();

    if (alg == "QUICK") quick_sort_wrapper(data);
    else if (alg == "RADIX") radix_sort_wrapper(data);
    else if (alg == "TIM") tim_sort_wrapper(data);
    else if (alg == "MERGE") merge_sort_wrapper(data);
    else if (alg == "HEAP") heap_sort_wrapper(data);
    else {
        cerr << "Algoritmo no valido\n";
        return 1;
    }

    auto end = clk::now();

    if (!is_sorted_ok(data)) {
        cerr << "Error: salida no ordenada\n";
        return 1;
    }

    if (!write_binary_file(outputFile, data)) {
        cerr << "Error escribiendo archivo de salida\n";
        return 1;
    }

    auto ms = chrono::duration_cast<chrono::milliseconds>(end - start).count();

    cout << "Tiempo baseline: " << ms << " ms\n";
    cout << "Algoritmo: " << alg << "\n";

    return 0;
}