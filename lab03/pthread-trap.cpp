#include <pthread.h>
#include <iostream>
#include <vector>
#include "timer.h"
using namespace std;

// 二次函数
double f(double x) {
    return x * x;
}

pthread_mutex_t mutex;

typedef struct {
    double a;        // 区间下限
    double b;        // 区间上限
    int n;           // 粒度
    int thread_num;  // 线程数
} params;

double result = 0.0;

void* trap_pthread(void* argv) {
    params p = *((params*)argv);

    double h = (p.b - p.a) / p.n;
    int rank = pthread_self() - 2;

    int local_n = p.n / p.thread_num;
    double local_a = p.a + rank * local_n * h;
    double local_b = local_a + local_n * h;

    double my_result = (f(local_a) + f(local_b)) / 2.0;

    for (int i = 1; i <= local_n - 1; i++) {
        my_result += f(local_a + i * h);
    }

    my_result = my_result * h;

    pthread_mutex_lock(&mutex);

    result += my_result;

    pthread_mutex_unlock(&mutex);

    return nullptr;
}

int main(int argc, char* argv[]) {
    double a = stod(argv[1]);
    double b = stod(argv[2]);

    pthread_mutex_init(&mutex, nullptr);

    TimerCounter tc;

    for (int num = 2; num <= 8; num++) {
        printf("The num of thread is: %d\n", num);

        for (int d = 24; d <= 24000000; d *= 10) {
            printf("Delta is %d ", d);

            for (int i = 0; i < 3; i++) {
                // 重复实验三次
                vector<pthread_t> threads(num);

                params p = {a, b, d, num};

                tc.start();

                result = 0;

                for (int i = 0; i < threads.size(); i++) {
                    pthread_create(&threads[i], nullptr, &trap_pthread, &p);
                }

                for (int i = 0; i < threads.size(); i++) {
                    pthread_join(threads[i], nullptr);
                }

                printf("%f ", tc.now());
            }

            printf("\n");
        }
    }

    pthread_mutex_destroy(&mutex);

    return EXIT_SUCCESS;
}
