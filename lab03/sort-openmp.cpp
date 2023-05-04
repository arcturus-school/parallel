#include <omp.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include "random.h"
#include "timer.h"
using namespace std;

TimerCounter tc;

typedef vector<vector<int>> M;

// 初始化数组
void init(M& m) {
    for (int i = 0; i < m.size(); i++) {
        for (int j = 0; j < m[i].size(); j++) {
            m[i][j] = randint();
        }
    }
}

// 单线程
void my_sort(M m) {
    for (int i = 0; i < m.size(); i++) {
        sort(m[i].begin(), m[i].end());
    }
}

// 动态划分
void openmp_dynamic_sort(M m, int thread_nums, int size) {
    tc.start();
#pragma omp parallel for num_threads(thread_nums) schedule(dynamic, size)
    for (int i = 0; i < m.size(); i++) {
        sort(m[i].begin(), m[i].end());
    }
    printf("%f ", tc.now());
}

// 静态划分
void openmp_static_sort(M m, int thread_nums, int size) {
    tc.start();
#pragma omp parallel for num_threads(thread_nums) schedule(static, size)
    for (int i = 0; i < m.size(); i++) {
        sort(m[i].begin(), m[i].end());
    }
    printf("%f ", tc.now());
}

int main(int argc, char* argv[]) {
    int n = stoi(argv[1]);

    seed();

    M m(n, vector<int>(n, 0));
    init(m);

    printf("dynamic: ");

    // for (int num = 2; num <= 8; num++) {
    //     printf("thread: %d\n", num);
    //     for (int size = 50; size <= 1000; size += 50) {
    //         printf("size: %d\n", size);

    //         for (int i = 0; i < 3; i++) {
    //             openmp_dynamic_sort(m, num, size);
    //         }

    //         printf("\n");
    //     }
    // }

    openmp_dynamic_sort(m, 8, 500);

    printf("\n");

    printf("static: ");

    // for (int num = 2; num <= 8; num++) {
    //     printf("thread: %d\n", num);
    //     for (int size = 50; size <= 1000; size += 50) {
    //         printf("size: %d\n", size);

    //         for (int i = 0; i < 3; i++) {
    //             openmp_static_sort(m, num, size);
    //         }

    //         printf("\n");
    //     }
    // }

    openmp_static_sort(m, 8, 500);

    printf("\n");

    tc.start();

    my_sort(m);

    printf("single thread: %f\n", tc.now());

    return EXIT_SUCCESS;
}
