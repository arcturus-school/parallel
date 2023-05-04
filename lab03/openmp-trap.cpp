#include <omp.h>
#include <iostream>
#include <vector>
#include "timer.h"
using namespace std;

// 二次函数
double f(double x) {
    return x * x;
}

double trap_openmp(double a, double b, int n) {
    int rank = omp_get_thread_num();
    int thread_num = omp_get_num_threads();

    double h = (b - a) / n;
    double local_n = n / thread_num;
    double local_a = a + rank * local_n * h;
    double local_b = local_a + local_n * h;

    double my_result = (f(local_a) + f(local_b)) / 2.0;

    for (int i = 1; i <= local_n - 1; i++) {
        my_result += f(local_a + i * h);
    }

    return my_result * h;
}

int main(int argc, char* argv[]) {
    double a = stod(argv[1]);
    double b = stod(argv[2]);

    double result = 0.0;

    TimerCounter tc;

    for (int num = 2; num <= 8; num++) {
        printf("The num of thread is: %d\n", num);

        for (int d = 24; d <= 24000000; d *= 10) {
            printf("Delta is %d ", d);

            for (int i = 0; i < 3; i++) {
                tc.start();

                result = 0.0;

#pragma omp parallel num_threads(num) reduction(+ : result)
                result += trap_openmp(a, b, d);

                printf("%f ", tc.now());
            }

            printf("\n");
        }
    }

    return EXIT_SUCCESS;
}
