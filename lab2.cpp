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
    for (double i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

double min(double a, double b) {
	return a < b ? a : b;
}

int main(void) {
    double e_x1 = 0.0;
    double e_x2 = 0.0;
    double e_x3 = 0.0;
    const double x = 1;
	const int N_TIMES = 1;
    const int N1 = 140000;
	const int N2 = 170000;
	const int N3 = 190000;
    double absolute_fault = 1000.0 / CLOCKS_PER_SEC;
	double min_duration1 = std::numeric_limits<double>::max();
	double min_duration2 = std::numeric_limits<double>::max();
	double min_duration3 = std::numeric_limits<double>::max();

	for (int i = 0; i < N_TIMES; ++i) {
		clock_t start = clock();
		for (int i = 0; i < N1; ++i) {
			e_x1 += pow(x, i) / factorial(i);
		}
		clock_t end = clock();
    	double duration = ((double)(end - start)) * absolute_fault;
		min_duration1 = min(min_duration1, duration);
	}
	for (int i = 0; i < N_TIMES; ++i) {
		clock_t start = clock();
		for (int i = 0; i < N2; ++i) {
			e_x2 += pow(x, i) / factorial(i);
		}
		clock_t end = clock();
    	double duration = ((double)(end - start)) * absolute_fault;
		min_duration2 = min(min_duration2, duration);
	}
	for (int i = 0; i < N_TIMES; ++i) {
		clock_t start = clock();
		for (int i = 0; i < N3; ++i) {
			e_x3 += pow(x, i) / factorial(i);
		}
		clock_t end = clock();
    	double duration = ((double)(end - start)) * absolute_fault;
		min_duration3 = min(min_duration3, duration);
	}

	printf("N1 = %d, absolute error = %.17f, time = %.0f ms\n", N1, abs(exp(x) - e_x1), min_duration1);
    printf("N2 = %d, absolute error = %.17f, time = %.0f ms\n", N2, abs(exp(x) - e_x2), min_duration2);
    printf("N3 = %d, absolute error = %.17f, time = %.0f ms\n", N3, abs(exp(x) - e_x3), min_duration3);

    return 0;
}