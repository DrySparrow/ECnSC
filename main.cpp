#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>
#include <cstdint>
#include <xmmintrin.h>
#include <limits>
#include "cblas.h"

const int SIZE = 1024; 
const int M_ITER = 10;

uint64_t getCpuTicks() {
    unsigned int lo, hi;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return (static_cast<unsigned long long>(hi) << 32) | lo;
}

// Вспомогательная функция для вычисления норм и матрицы B = A^T / (||A||1 * ||A||inf)
void prepare_B(const float* A, float* B, int N) {
    float norm1 = 0, normInf = 0;
    for (int j = 0; j < N; ++j) {
        float colSum = 0;
        for (int i = 0; i < N; ++i) colSum += std::abs(A[i * N + j]);
        if (colSum > norm1) norm1 = colSum;
    }
    for (int i = 0; i < N; ++i) {
        float rowSum = 0;
        for (int j = 0; j < N; ++j) rowSum += std::abs(A[i * N + j]);
        if (rowSum > normInf) normInf = rowSum;
    }
    float divisor = norm1 * normInf;
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            B[i * N + j] = A[j * N + i] / divisor;
        }
    }
}

// Scalar
void inverse_scalar(const float* A, float* A_inv, int N, int M) {
    std::vector<float> B(N * N), R(N * N), temp(N * N), cur_pow(N * N);
    prepare_B(A, B.data(), N);

    // R = I - B*A
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            float sum = 0;
            for (int k = 0; k < N; ++k) sum += B[i * N + k] * A[k * N + j];
            R[i * N + j] = (i == j ? 1.0f : 0.0f) - sum;
        }
    }

    std::copy(B.begin(), B.end(), A_inv);
    std::copy(B.begin(), B.end(), cur_pow.begin());

    for (int m = 1; m < M; ++m) {
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                float sum = 0;
                for (int k = 0; k < N; ++k) sum += R[i * N + k] * cur_pow[k * N + j];
                temp[i * N + j] = sum;
            }
        }
        for (int i = 0; i < N * N; ++i) {
            cur_pow[i] = temp[i];
            A_inv[i] += temp[i];
        }
    }
}

// SSE
void inverse_sse(const float* A, float* A_inv, int N, int M) {
    std::vector<float> B(N * N), R(N * N), temp(N * N), cur_pow(N * N);
    prepare_B(A, B.data(), N);

    // R = I - B*A
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; j += 4) {
            __m128 sum_v = _mm_setzero_ps();
            for (int k = 0; k < N; ++k) {
                sum_v = _mm_add_ps(sum_v, _mm_mul_ps(_mm_set1_ps(B[i * N + k]), _mm_loadu_ps(&A[k * N + j])));
            }
            __m128 ident = _mm_set_ps(i==j+3?1:0, i==j+2?1:0, i==j+1?1:0, i==j?1:0);
            _mm_storeu_ps(&R[i * N + j], _mm_sub_ps(ident, sum_v));
        }
    }

    std::copy(B.begin(), B.end(), A_inv);
    std::copy(B.begin(), B.end(), cur_pow.begin());

    for (int m = 1; m < M; ++m) {
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; j += 4) {
                __m128 sum_v = _mm_setzero_ps();
                for (int k = 0; k < N; ++k) {
                    sum_v = _mm_add_ps(sum_v, _mm_mul_ps(_mm_set1_ps(R[i * N + k]), _mm_loadu_ps(&cur_pow[k * N + j])));
                }
                _mm_storeu_ps(&temp[i * N + j], sum_v);
            }
        }
        for (int i = 0; i < N * N; i += 4) {
            _mm_storeu_ps(&A_inv[i], _mm_add_ps(_mm_loadu_ps(&A_inv[i]), _mm_loadu_ps(&temp[i])));
            _mm_storeu_ps(&cur_pow[i], _mm_loadu_ps(&temp[i]));
        }
    }
}

// BLAS
void inverse_blas(const float* A, float* A_inv, int N, int M) {
    std::vector<float> B(N * N), R(N * N), temp(N * N), cur_pow(N * N);
    prepare_B(A, B.data(), N);

    // R = -B*A
    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, N, N, N, -1.0f, B.data(), N, A, N, 0.0f, R.data(), N);
    for (int i = 0; i < N; ++i) R[i * N + i] += 1.0f; // R = I - BA

    std::copy(B.begin(), B.end(), A_inv);
    std::copy(B.begin(), B.end(), cur_pow.begin());

    for (int m = 1; m < M; ++m) {
        cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, N, N, N, 1.0f, R.data(), N, cur_pow.data(), N, 0.0f, temp.data(), N);
        std::copy(temp.begin(), temp.end(), cur_pow.begin());
        cblas_saxpy(N * N, 1.0f, temp.data(), 1, A_inv, 1);
    }
}

int main() {
    const int RUNS = 5;
    std::vector<float> A(SIZE * SIZE);
    std::vector<float> res(SIZE * SIZE, 0.0f);

    // Заполнение матрицы A (сделаем её диагонально доминирующей для сходимости ряда)
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            A[i * SIZE + j] = (i == j) ? 2.0f * SIZE : 1.0f;
        }
    }

    uint64_t start, end, minTicks;

    // Scalar
    minTicks = std::numeric_limits<uint64_t>::max();
    for (int i = 0; i < RUNS; ++i) {
        start = getCpuTicks();
        inverse_scalar(A.data(), res.data(), SIZE, M_ITER);
        end = getCpuTicks();
        minTicks = std::min(minTicks, end - start);
    }
    std::cout << "Scalar ticks: " << minTicks << std::endl;

    // SSE
    minTicks = std::numeric_limits<uint64_t>::max();
    for (int i = 0; i < RUNS; ++i) {
        start = getCpuTicks();
        inverse_sse(A.data(), res.data(), SIZE, M_ITER);
        end = getCpuTicks();
        minTicks = std::min(minTicks, end - start);
    }
    std::cout << "SSE ticks:    " << minTicks << std::endl;

    // BLAS
    minTicks = std::numeric_limits<uint64_t>::max();
    for (int i = 0; i < RUNS; ++i) {
        start = getCpuTicks();
        inverse_blas(A.data(), res.data(), SIZE, M_ITER);
        end = getCpuTicks();
        minTicks = std::min(minTicks, end - start);
    }
    std::cout << "BLAS ticks:   " << minTicks << std::endl;

    return 0;
}