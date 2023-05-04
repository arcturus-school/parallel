#include <iostream>
#include "timer.h"
using namespace std;

// 二次函数
double f(double x) {
    return x * x;
}

double trap_signle_thread(double a, double b, int n) {
    double h = (b - a) / n;
    double approx = (f(a) + f(b)) / 2.0;

    for (int i = 1; i <= n - 1; i++) {
        approx += f(a + i * h);
    }

    return h * approx;
}

int main(int argc, char* argv[]) {
    double a = stod(argv[1]);
    double b = stod(argv[2]);
    int n = stoi(argv[3]);

    TimerCounter tc;

    tc.start();
    double result = trap_signle_thread(a, b, n);

    printf("The time of signle thread: %fms.\n", tc.now());

    printf("The result of signle thread: %f.\n", result);

    return EXIT_SUCCESS;
}