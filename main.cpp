#include <iostream>
#include <random>
#include <fstream>

using namespace std;

uint64_t getCpuTicks() {
    unsigned int lo, hi;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return (static_cast<unsigned long long>(hi) << 32) | lo;
}

uint64_t measure(const int* arr, size_t size) {
    volatile size_t k = 0;
    
    size_t i;
    const uint64_t start = getCpuTicks();
    for (i = 0; i < size; ++i) {
        k = arr[k];
    }
    const uint64_t end = getCpuTicks();

    return (end - start) / size;
}

uint64_t measure_min_ticks(const int* arr, const size_t size, const size_t RUNS) {
    uint64_t min_ticks = UINT64_MAX;

    for (size_t i = 0; i < RUNS; ++i) {
        uint64_t ticks = measure(arr, size);
        min_ticks = min_ticks > ticks ? ticks : min_ticks;
    }

    return min_ticks;
}

int* getArray(const size_t n, const size_t size, const size_t offset) {
    const size_t arraySize = offset * n;
    int* arr = new int[arraySize];
    for (size_t i = 0; i < size / n; ++i) {
        for (size_t j = 0; j < n - 1; ++j) {
            arr[i + j * offset] = static_cast<int>(i + j * offset + offset);
        }
        arr[i + (n - 1) * offset] = static_cast<int>(i + 1);
    }
    arr[size / n - 1 + (n - 1) * offset] = 0;
    return arr;
}


int main() {
    constexpr size_t SIZE = 128 * 1024 / sizeof(int);            // 128KB
    constexpr size_t OFFSET = 16 * 1024 * 1024 / sizeof(int);   // 16MB
    constexpr size_t N = 128;
    constexpr size_t RUNS = 40;

    uint64_t ticks;

    ofstream outfile("access_ticks.csv");
    outfile << "Size(el),Time(ticks)\n";
    for (int n_times = 1; n_times < N + 1; ++ n_times) {
        const int* arr = getArray(n_times, SIZE, OFFSET);
        ticks = measure_min_ticks(arr, SIZE*n_times, RUNS);
        outfile << n_times << ',' << ticks << std::endl;
        delete[] arr;
    }

    
    outfile.close();
    cout << "\nresults saved in access_ticks.csv\n";
    
    return 0;
}