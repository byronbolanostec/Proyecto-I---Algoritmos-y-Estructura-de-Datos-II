#include <iostream>
#include <cstdio>
#include <string>
#include <chrono>
#include <cstdint>

using namespace std;



//Función de arreglo paginado.
class PagedArray {
    //slot de ram (info)
    struct Page {
        int32_t* data;
        long long page;
        bool loaded, dirty;
    };

    FILE* file;
    long long n, totalPages;
    int pageSize, pageCount;
    long long hits = 0, faults = 0;

    Page* slots;
    long long* pageToSlot;
    int* prev;
    int* next;

    int head = -1, tail = -1, freeHead = 0;

    long long getPage(long long i) const { return i / pageSize; }
    int getOffset(long long i) const { return (int)(i % pageSize); }

    int pageSizeReal(long long p) const {
        long long start = p * 1LL * pageSize;
        long long rem = n - start;
        return rem >= pageSize ? pageSize : (int)rem;
    }

    //Remplazo de LRU, el slot victima es el que esta al final de la lista (tail).
    void detach(int s) {
        int p = prev[s], nx = next[s];
        if (p != -1) next[p] = nx; else head = nx;
        if (nx != -1) prev[nx] = p; else tail = p;
        prev[s] = -1;
        next[s] = -1;
    }

    void push(int s) {
        prev[s] = -1;
        next[s] = head;
        if (head != -1) prev[head] = s;
        else tail = s;
        head = s;
    }

    int victimSlot() {
        if (freeHead != -1) {
            int s = freeHead;
            freeHead = next[s];
            if (freeHead != -1) prev[freeHead] = -1;
            next[s] = -1;
            prev[s] = -1;
            return s;
        }
        return tail;
    }

    void write(int s) {
        if (!slots[s].loaded || !slots[s].dirty) return;

        fseek(file, slots[s].page * 1LL * pageSize * sizeof(int32_t), SEEK_SET);
        fwrite(slots[s].data, sizeof(int32_t), pageSizeReal(slots[s].page), file);
        slots[s].dirty = false;
    }

    void load(long long p, int s) {
        if (slots[s].loaded) {
            write(s);
            pageToSlot[slots[s].page] = -1;
            detach(s);
        }

        fseek(file, p * 1LL * pageSize * sizeof(int32_t), SEEK_SET);
        fread(slots[s].data, sizeof(int32_t), pageSizeReal(p), file);

        slots[s].page = p;
        slots[s].loaded = true;
        slots[s].dirty = false;
        pageToSlot[p] = s;

        push(s);
    }

    int ensure(long long p) {
        int s = (int)pageToSlot[p];
        if (s != -1) {
            hits++;
            detach(s);
            push(s);
            return s;
        }

        faults++;
        s = victimSlot();
        load(p, s);
        return s;
    }

public:
    class Ref {
        PagedArray* a;
        long long i;
    public:
        Ref(PagedArray* a, long long i) : a(a), i(i) {}

        operator int32_t() const {
            int s = a->ensure(a->getPage(i));
            return a->slots[s].data[a->getOffset(i)];
        }

        Ref& operator=(int32_t x) {
            int s = a->ensure(a->getPage(i));
            a->slots[s].data[a->getOffset(i)] = x;
            a->slots[s].dirty = true;
            return *this;
        }

        friend void swap(Ref a, Ref b) {
            int32_t t = a;
            a = (int32_t)b;
            b = t;
        }
    };

    PagedArray(const string& path, int ps, int pc) {
        file = fopen(path.c_str(), "r+b");
        if (!file) {
            cerr << "Error: no se pudo abrir archivo output.\n";
            exit(1);
        }

        if (ps <= 0 || pc <= 0) {
            cerr << "Error: pageSize y pageCount deben ser mayores que 0.\n";
            fclose(file);
            exit(1);
        }

        fseek(file, 0, SEEK_END);
        n = ftell(file) / sizeof(int32_t);
        fseek(file, 0, SEEK_SET);

        pageSize = ps;
        pageCount = pc;
        totalPages = (n + ps - 1) / ps;

        slots = new Page[pageCount];
        prev = new int[pageCount];
        next = new int[pageCount];
        pageToSlot = new long long[totalPages];

        for (int i = 0; i < pageCount; i++) {
            slots[i].data = new int32_t[pageSize];
            slots[i].page = -1;
            slots[i].loaded = false;
            slots[i].dirty = false;
            prev[i] = i - 1;
            next[i] = (i + 1 < pageCount ? i + 1 : -1);
        }

        for (long long i = 0; i < totalPages; i++) pageToSlot[i] = -1;
    }

    ~PagedArray() {
        for (int i = 0; i < pageCount; i++) write(i);
        fflush(file);

        for (int i = 0; i < pageCount; i++) delete[] slots[i].data;
        delete[] slots;
        delete[] prev;
        delete[] next;
        delete[] pageToSlot;
        fclose(file);
    }

    Ref operator[](long long i) { return Ref(this, i); }
    long long size() const { return n; }
    long long getHits() const { return hits; }
    long long getFaults() const { return faults; }
};



//Funcion para copiar el archivo inicial al de salida.
bool copyFile(const string& a, const string& b) {
    FILE* in = fopen(a.c_str(), "rb");
    if (!in) {
        cerr << "Error: no se pudo abrir input.\n";
        return false;
    }

    FILE* out = fopen(b.c_str(), "wb");
    if (!out) {
        cerr << "Error: no se pudo crear output.\n";
        fclose(in);
        return false;
    }

    char buf[1 << 20];
    size_t r;
    while ((r = fread(buf, 1, sizeof(buf), in)) > 0) {
        fwrite(buf, 1, r, out);
    }

    fclose(in);
    fclose(out);
    return true;
}



///////////////////////////////// SORTS (Algoritmos usados para ordenar el arreglo paginado)



void insertion(PagedArray& a, long long l, long long r) {
    for (long long i = l + 1; i <= r; i++) {
        int32_t x = a[i];
        long long j = i - 1;
        while (j >= l && a[j] > x) {
            a[j + 1] = a[j];
            j--;
        }
        a[j + 1] = x;
    }
}

//////////////////////// quick
void quick(PagedArray& a, long long l, long long r) {
    while (l < r) {
        if (r - l < 64) {
            insertion(a, l, r);
            return;
        }

        int32_t pivot = a[(l + r) >> 1];
        long long i = l, j = r;

        while (i <= j) {
            while (a[i] < pivot) i++;
            while (a[j] > pivot) j--;
            if (i <= j) swap(a[i++], a[j--]);
        }

        if (j - l < r - i) {
            if (l < j) quick(a, l, j);
            l = i;
        } else {
            if (i < r) quick(a, i, r);
            r = j;
        }
    }
}
void quickSort(PagedArray& a) {
    if (a.size() > 1) quick(a, 0, a.size() - 1);
}

//////////////////////// merge
void mergeRange(PagedArray& a, long long l, long long m, long long r) {
    long long n1 = m - l + 1;
    long long n2 = r - m;

    int32_t* L = new int32_t[n1];
    int32_t* R = new int32_t[n2];

    for (long long i = 0; i < n1; i++) L[i] = a[l + i];
    for (long long i = 0; i < n2; i++) R[i] = a[m + 1 + i];

    long long i = 0, j = 0, k = l;
    while (i < n1 && j < n2) {
        a[k++] = (L[i] <= R[j] ? L[i++] : R[j++]);
    }
    while (i < n1) a[k++] = L[i++];
    while (j < n2) a[k++] = R[j++];

    delete[] L;
    delete[] R;
}
void mergeSort(PagedArray& a, long long l, long long r) {
    if (l >= r) return;
    long long m = (l + r) / 2;
    mergeSort(a, l, m);
    mergeSort(a, m + 1, r);
    mergeRange(a, l, m, r);
}
void mergeSort(PagedArray& a) {
    if (a.size() > 1) mergeSort(a, 0, a.size() - 1);
}

//////////////////////// tim
void timSort(PagedArray& a) {
    long long n = a.size();
    long long RUN = 32;

    for (long long i = 0; i < n; i += RUN) {
        long long r = i + RUN - 1;
        if (r >= n) r = n - 1;
        insertion(a, i, r);
    }

    for (long long sz = RUN; sz < n; sz *= 2) {
        for (long long l = 0; l < n; l += 2 * sz) {
            long long m = l + sz - 1;
            long long r = l + 2 * sz - 1;
            if (m >= n) m = n - 1;
            if (r >= n) r = n - 1;
            if (m < r) mergeRange(a, l, m, r);
        }
    }
}

//////////////////////// radix
void radixSort(PagedArray& a) {
    long long n = a.size();
    int32_t* t = new int32_t[n];

    for (int s = 0; s < 32; s += 8) {
        int c[256] = {0};

        for (long long i = 0; i < n; i++) {
            c[((((uint32_t)(int32_t)a[i]) ^ 0x80000000u) >> s) & 255]++;
        }

        for (int i = 1; i < 256; i++) c[i] += c[i - 1];

        for (long long i = n - 1; i >= 0; i--) {
            int x = ((((uint32_t)(int32_t)a[i]) ^ 0x80000000u) >> s) & 255;
            t[--c[x]] = a[i];
            if (i == 0) break;
        }

        for (long long i = 0; i < n; i++) a[i] = t[i];
    }

    delete[] t;
}

//////////////////////// heap
void heapify(PagedArray& a, long long n, long long i) {
    while (true) {
        long long l = 2 * i + 1, r = 2 * i + 2, largest = i;
        if (l < n && a[l] > a[largest]) largest = l;
        if (r < n && a[r] > a[largest]) largest = r;
        if (largest == i) break;
        swap(a[i], a[largest]);
        i = largest;
    }
}
void heapSort(PagedArray& a) {
    long long n = a.size();

    for (long long i = n / 2 - 1; i >= 0; i--) {
        heapify(a, n, i);
        if (i == 0) break;
    }

    for (long long i = n - 1; i > 0; i--) {
        swap(a[0], a[i]);
        heapify(a, i, 0);
    }
}



///////////////////////////////// MAIN



int main(int argc, char* argv[]) {
    if (argc != 11) {
        cerr << "Uso: sorter -input <archivo.bin> -output <archivo.bin> -alg <QUICK|RADIX|TIM|MERGE|HEAP> -pageSize <n> -pageCount <n>\n";
        return 1;
    }

    string in, out, alg;
    int ps = 0, pc = 0;

    for (int i = 1; i < argc; i += 2) {
        string k = argv[i], v = argv[i + 1];
        if (k == "-input") in = v;
        else if (k == "-output") out = v;
        else if (k == "-alg") alg = v;
        else if (k == "-pageSize") ps = stoi(v);
        else if (k == "-pageCount") pc = stoi(v);
        else {
            cerr << "Error: argumento invalido -> " << k << "\n";
            return 1;
        }
    }

    if (in.empty() || out.empty() || alg.empty()) {
        cerr << "Error: faltan argumentos.\n";
        return 1;
    }

    if (ps <= 0 || pc <= 0) {
        cerr << "Error: pageSize y pageCount deben ser > 0.\n";
        return 1;
    }

    if (alg != "QUICK" && alg != "RADIX" && alg != "TIM" && alg != "MERGE" && alg != "HEAP") {
        cerr << "Error: algoritmo no soportado.\n";
        return 1;
    }

    if (!copyFile(in, out)) return 1;

    auto t0 = chrono::high_resolution_clock::now();

    long long hits, faults;
    {
        PagedArray a(out, ps, pc);

        if (alg == "QUICK") quickSort(a);
        else if (alg == "RADIX") radixSort(a);
        else if (alg == "TIM") timSort(a);
        else if (alg == "MERGE") mergeSort(a);
        else if (alg == "HEAP") heapSort(a);

        hits = a.getHits();
        faults = a.getFaults();
    }

    auto t1 = chrono::high_resolution_clock::now();

    cout << "Tiempo: " << chrono::duration_cast<chrono::milliseconds>(t1 - t0).count() << " ms\n";
    cout << "Algoritmo: " << alg << "\n";
    cout << "Page hits: " << hits << "\n";
    cout << "Page faults: " << faults << "\n";

    return 0;
}