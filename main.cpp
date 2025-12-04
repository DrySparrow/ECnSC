#include <iostream>
#include <vector>
#include <random>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sys/time.h> // время
#include <chrono>

using namespace std;

uint64_t getCpuTicks() {
    unsigned int lo, hi;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return (static_cast<unsigned long long>(hi) << 32) | lo;
}

void warm_up(const vector<int>& arr) {
    volatile size_t k = 0;
    for (size_t i = 0; i < arr.size(); ++i) {
        k = arr[k];
    }

    if (k == 123456789) {
        cout << k << endl;
    }
}

void fill_forward(vector<int>& arr) {
    size_t size = arr.size();
    for (size_t i = 0; i < size - 1; ++i) {
        arr[i] = i + 1;
    }
    arr[size - 1] = 0;
}

void fill_backward(vector<int>& arr) {
    size_t size = arr.size();
    for (size_t i = size - 1; i > 0; --i) {
        arr[i] = i - 1;
    }
    arr[0] = size - 1;
}

void fill_random(vector<int>& arr) {
    int* temp = new int[arr.size()];
    for (size_t i = 0; i < arr.size() - 1; ++i) {
        temp[i] = i + 1;
    }
    temp[arr.size() - 1] = 0;
    std::mt19937 g(time(nullptr));
    std::shuffle(temp, temp + arr.size(), g);
    for (size_t i = 0; i < arr.size() - 1; ++i) {
        arr[temp[i]] = temp[i + 1];
    }
    arr[temp[arr.size() - 1]] = temp[0];
    delete[](temp);
}

uint64_t measure(vector<int>& arr) {
    size_t size = arr.size();
    volatile size_t k = 0;
    size_t i;

    const uint64_t start = getCpuTicks();
    for (k = 0, i = 0; i < size; ++i) {
        k = arr[k];
    }
    const uint64_t end = getCpuTicks();

    return (end - start) / size;
}

uint64_t measure_min_ticks(vector<int>& arr, const size_t RUNS) {
    size_t size = arr.size();
    uint64_t min_ticks = UINT64_MAX;

    for (size_t i = 0; i < RUNS; ++i) {
        uint64_t ticks = measure(arr);
        min_ticks = min_ticks > ticks ? ticks : min_ticks;
    }

    return min_ticks;
}

int main() {
    const size_t MIN_SIZE = 256;
    const size_t MAX_SIZE = 1024 * 1024 * 32 / sizeof(int);
    
    ofstream outfile("access_patterns.csv");
    outfile << "ArraySize(Bytes),ArraySize(Elements),Forward(ns),Backward(ns),Random(ns)\n";
    
    for (size_t size_elements = MIN_SIZE; size_elements < MAX_SIZE; size_elements *= 1.05) {
        vector<int> array(size_elements);
        
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<int> dis(1, 1000);
        
        for (size_t i = 0; i < size_elements; i++) {
            array[i] = dis(gen);
        }
        
        const int RUNS = 15;

        fill_forward(array);
        warm_up(array);
        double time_forward = measure_min_ticks(array, RUNS);
        fill_backward(array);
        warm_up(array);
        double time_backward = measure_min_ticks(array, RUNS);
        fill_random(array);
        warm_up(array);
        double time_random = measure_min_ticks(array, RUNS);
        
        // Размер массива в байтах
        size_t size_bytes = size_elements * sizeof(int);
        
        // Запись в CSV
        outfile << size_bytes << "," 
                << size_elements << "," 
                << time_forward << "," 
                << time_backward << "," 
                << time_random << endl;
        
        // Останавливаемся, если достигли максимального размера
        if (size_elements > MAX_SIZE / 2) break;
    }
    
    outfile.close();
    cout << "\nresults saved in access_patterns.csv\n";
    
    return 0;
}