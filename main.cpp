#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>
#include <cstdint>
#include <xmmintrin.h>
#include "cblas.h"

const int SIZE = 1024; // Размер матрицы 1024x1024


uint64_t getCpuTicks() {
    unsigned int lo, hi;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return (static_cast<unsigned long long>(hi) << 32) | lo;
}

// 1. Scalar
void multiply_scalar(const float* mat, const float* vec, float* res) {
    for (int i = 0; i < SIZE; ++i) {
        float sum = 0.0f;
        for (int j = 0; j < SIZE; ++j) {
            sum += mat[i * SIZE + j] * vec[j];
        }
        res[i] = sum;
    }
}

// 2. SSE Intrinsics
void multiply_sse(const float* mat, const float* vec, float* res) {
    for (int i = 0; i < SIZE; ++i) {
        __m128 row_sum = _mm_setzero_ps();
        for (int j = 0; j < SIZE; j += 4) {
            __m128 m = _mm_loadu_ps(&mat[i * SIZE + j]);
            __m128 v = _mm_loadu_ps(&vec[j]);
            row_sum = _mm_add_ps(row_sum, _mm_mul_ps(m, v));
        }
        __m128 shuf = _mm_movehl_ps(row_sum, row_sum);
        __m128 sums = _mm_add_ps(row_sum, shuf);
        shuf = _mm_shuffle_ps(sums, sums, 1);
        sums = _mm_add_ss(sums, shuf);
        _mm_store_ss(&res[i], sums);
    }
}

// 3. BLAS
void multiply_blas(const float* mat, const float* vec, float* res) {
    // cblas_sgemv — умножение матрицы на вектор
    // CblasRowMajor — данные в матрице расположены по строкам
    // CblasNoTrans — не транспонировать матрицу
    // 1.0f и 0.0f — коэффициенты alpha и beta (стандарт для умножения)
    cblas_sgemv(CblasRowMajor, CblasNoTrans, 
                SIZE, SIZE, 1.0f, mat, SIZE, 
                vec, 1, 0.0f, res, 1);
}

void fill_matrix(std::vector<float>& mat, int size) {
    for (int i = 0; i < size * size; ++i) {
        // Значения будут идти так: 1.1, 2.1, 3.1 ... 1024.1, а затем снова 1.1
        mat[i] = 1.1f + static_cast<float>(i % size);
    }
}

void fill_vector(std::vector<float>& vec, int size) {
    for (int i = 0; i < size; ++i) {
        vec[i] = 1.1f + static_cast<float>(i % size);
    }
}

int main() {
    const int RUNS = 100; 
    const int N_TIMES = 10; 
    
    std::vector<float> matrix(SIZE * SIZE);
    std::vector<float> vec(SIZE);
    std::vector<float> res(SIZE, 0.0f);

    fill_matrix(matrix, SIZE);
    fill_vector(vec, SIZE);

    uint64_t start, end;
    uint64_t minTicks = std::numeric_limits<uint64_t>::max();
    // Тест 1: Scalar
    for (int i = 0; i < N_TIMES; ++i) {
        start = getCpuTicks();
        for (int i = 0; i < RUNS; ++i) multiply_scalar(matrix.data(), vec.data(), res.data());
        end = getCpuTicks();
        minTicks = std::min(minTicks, end - start);
    }
    std::cout << "Scalar average ticks:  " << minTicks << std::endl;
    minTicks = std::numeric_limits<uint64_t>::max();

    // Тест 2: SSE
    for (int i = 0; i < N_TIMES; ++i) {
        start = getCpuTicks();
        for (int i = 0; i < RUNS; ++i) multiply_sse(matrix.data(), vec.data(), res.data());
        end = getCpuTicks();
        minTicks = std::min(minTicks, end - start);
    }
    std::cout << "SSE average ticks:     " << minTicks << std::endl;
    minTicks = std::numeric_limits<uint64_t>::max();

    // Тест 3: BLAS
    for (int i = 0; i < N_TIMES; ++i) {
        start = getCpuTicks();
        for (int i = 0; i < RUNS; ++i) multiply_blas(matrix.data(), vec.data(), res.data());
        end = getCpuTicks();
        minTicks = std::min(minTicks, end - start);
    }
    std::cout << "BLAS average ticks:    " << minTicks << std::endl;

    return 0;
}