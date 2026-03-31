#include <iostream>
using namespace std;

class PagedArray {
    struct PageSlot {
        int* data;
        long long pageNumber;
        bool occupied;
        bool dirty;
        long long lastUsed;
    };

    // Extras
    long long pageHits;
    long long pageFaults;
    long long usageClock;

    string filePath;
    fstream file;

    long long totalInts;
    int pageSize;
    int pageCount;

    PageSlot* slots;

    // Funciones de index
    long long getPageNumber(long long index) const {
        return index / pageSize;
    }

    int getOffset(long long index) const {
        return index % pageSize;
    }

    long long pageStartIndex(long long pageNumber) const {
        return pageNumber * pageSize;
    }

    int intsInPage(long long pageNumber) const {
        long long start = pageStartIndex(pageNumber);
        long long remaining = totalInts - start;

        if (remaining >= pageSize) return pageSize;
        return remaining;
    }

};

// =========================
// Utilidad básica
// =========================
inline void swapPaged(PagedArray& arr, long long i, long long j) {
    int temp = arr[i];
    arr[i] = arr[j];
    arr[j] = temp;
}

// =========================
// 1) Bubble Sort
// =========================
void bubbleSort(PagedArray& arr) {
    long long n = arr.size();

    for (long long i = 0; i < n - 1; i++) {
        bool swapped = false;

        for (long long j = 0; j < n - 1 - i; j++) {
            if ((int)arr[j] > (int)arr[j + 1]) {
                swapPaged(arr, j, j + 1);
                swapped = true;
            }
        }

        if (!swapped) break;
    }
}

// =========================
// 2) Selection Sort
// =========================
void selectionSort(PagedArray& arr) {
    long long n = arr.size();

    for (long long i = 0; i < n - 1; i++) {
        long long minIndex = i;

        for (long long j = i + 1; j < n; j++) {
            if ((int)arr[j] < (int)arr[minIndex]) {
                minIndex = j;
            }
        }

        if (minIndex != i) {
            swapPaged(arr, i, minIndex);
        }
    }
}

// =========================
// 3) Insertion Sort
// =========================
void insertionSort(PagedArray& arr) {
    long long n = arr.size();

    for (long long i = 1; i < n; i++) {
        int key = arr[i];
        long long j = i - 1;

        while (j >= 0 && (int)arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }

        arr[j + 1] = key;
    }
}

// =========================
// 4) Quick Sort
// =========================
long long partitionPaged(PagedArray& arr, long long low, long long high) {
    int pivot = arr[high];
    long long i = low - 1;

    for (long long j = low; j <= high - 1; j++) {
        if ((int)arr[j] < pivot) {
            i++;
            swapPaged(arr, i, j);
        }
    }

    swapPaged(arr, i + 1, high);
    return i + 1;
}

void quickSort(PagedArray& arr, long long low, long long high) {
    if (low < high) {
        long long pi = partitionPaged(arr, low, high);
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

// envoltura cómoda
void quickSort(PagedArray& arr) {
    long long n = arr.size();
    if (n > 0) quickSort(arr, 0, n - 1);
}

// =========================
// 5) Heap Sort
// =========================
void heapify(PagedArray& arr, long long n, long long i) {
    long long largest = i;
    long long left = 2 * i + 1;
    long long right = 2 * i + 2;

    if (left < n && (int)arr[left] > (int)arr[largest]) {
        largest = left;
    }

    if (right < n && (int)arr[right] > (int)arr[largest]) {
        largest = right;
    }

    if (largest != i) {
        swapPaged(arr, i, largest);
        heapify(arr, n, largest);
    }
}

void heapSort(PagedArray& arr) {
    long long n = arr.size();

    for (long long i = n / 2 - 1; i >= 0; i--) {
        heapify(arr, n, i);
        if (i == 0) break; // evita problema por ser long long
    }

    for (long long i = n - 1; i > 0; i--) {
        swapPaged(arr, 0, i);
        heapify(arr, i, 0);
    }
}

// =========================
// Main Function
// =========================

int main(int argc, char* argv[]) {

    // Solo para verificar que entraron argumentos
    if (argc > 1) {
        cout << "hi" << endl;
    }

    return 0;
}