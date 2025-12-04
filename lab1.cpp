#include <stdio.h>
#include <math.h>
#include <time.h>

double factorial(int n) {
    if (n < 0) {
        return -1;
    }
    if (n == 0 || n == 1) {
        return 1;
    }
    double result = 1;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

int main(void) {
    double e_x = 0.0;
    const double x = 1;
    const int N = 9000;
    double absolute_fault = 1000.0 / CLOCKS_PER_SEC;

    clock_t start = clock();
    for (int i = 0; i < N; ++i) {
        e_x += pow(x, i) / factorial(i);
    }
    clock_t end = clock();

    double duration = ((double)(end - start)) * absolute_fault;

    printf("e^x = %.12f\n", e_x);
    printf("time: %.0f ms\n", duration);
    printf("relative fault: %.4f%%\n", absolute_fault / duration * 100.0);

    return 0;
}