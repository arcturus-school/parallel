#include <mutex>
#include <thread>
#include "timer.h"
#include "tool.h"
using namespace std;

TimerCounter tc;

// 普通方法
void gauss(Matrix<float> a, Array<float> b, Array<float>& x) {
    int __ROW_SIZE__ = a.size(), __COL_SIZE__ = a[0].size();

    // 消元
    float __TEMP__;
    for (int k = 0; k < __ROW_SIZE__; k++) {
        __TEMP__ = a[k][k];

        for (int i = k; i < __COL_SIZE__; i++) {
            a[k][i] /= __TEMP__;
        }

        b[k] /= __TEMP__;

        for (int i = k + 1; i < __ROW_SIZE__; i++) {
            __TEMP__ = a[i][k];

            for (int j = k; j < __COL_SIZE__; j++) {
                a[i][j] -= __TEMP__ * a[k][j];
            }

            b[i] -= __TEMP__ * b[k];
        }
    }

    // 回代
    for (int i = __ROW_SIZE__ - 1; i >= 0; i--) {
        x[i] = b[i];

        for (int j = i + 1; j < __COL_SIZE__; j++) {
            x[i] -= a[i][j] * x[j];
        }

        x[i] /= a[i][i];
    }
}

int __GLOBAL_NEXT__;
mutex __TASK_LOCK__;

// 消元
void eliminate(int k, Matrix<float>& a, Array<float>& b) {
    int __ROW_SIZE__ = a.size(),     // 行数
        __COL_SIZE__ = a[0].size();  // 列数

    while (true) {
        __TASK_LOCK__.lock();

        int __ROW__ = __GLOBAL_NEXT__++;  // 当前线程被分配到某一行

        __TASK_LOCK__.unlock();

        if (__ROW__ >= __ROW_SIZE__)
            break;

        float __TEMP__ = a[__ROW__][k];

        for (int i = k; i < __COL_SIZE__; i++) {
            a[__ROW__][i] -= __TEMP__ * a[k][i];
        }

        b[__ROW__] -= __TEMP__ * b[k];
    }
}

void gauss_thread(int tn, Matrix<float> a, Array<float> b, Array<float>& x) {
    int __ROW_SIZE__ = a.size(), __COL_SIZE__ = a[0].size();

    Array<thread> threads(tn);

    float __TEMP__;
    for (int k = 0; k < __ROW_SIZE__; k++) {
        // 先将第 k 行主元变成 1
        __TEMP__ = a[k][k];

        for (int i = k; i < __COL_SIZE__; i++) {
            a[k][i] /= __TEMP__;
        }

        b[k] /= __TEMP__;

        __GLOBAL_NEXT__ = k + 1;

        // 开 tn 个线程遍历剩余行
        for (int i = 0; i < tn; i++) {
            threads[i] = thread(eliminate, k, ref(a), ref(b));
        }

        // 必须等待本轮消元完成后才能进入下一轮消元
        for (int i = 0; i < tn; i++) {
            threads[i].join();
        }
    }

    for (int i = __ROW_SIZE__ - 1; i >= 0; i--) {
        x[i] = b[i];

        for (int j = i + 1; j < __COL_SIZE__; j++) {
            x[i] -= a[i][j] * x[j];
        }

        x[i] /= a[i][i];
    }
}

int main(int argc, char* argv[]) {
    int p[2] = {1000, 4};  // 矩阵大小, 线程数

    for (int i = 0; i < argc - 1; i++) {
        p[i] = atoi(argv[i + 1]);
    }

    int __MATRIX_SIZE__ = p[0], __THREAD_NUM__ = p[1];

    Matrix<float> a(__MATRIX_SIZE__, Array<float>(__MATRIX_SIZE__, 0.0));
    Array<float> b(__MATRIX_SIZE__, 0);

    Array<float> x(__MATRIX_SIZE__, 0);

    initLinearEquationGroup(a, b);

    tc.start();

    gauss(a, b, x);

    printf("The single thread run : %f ms\n", tc.now());

    Array<float> y(__MATRIX_SIZE__, 0);

    tc.start();

    gauss_thread(__THREAD_NUM__, a, b, y);

    printf("The multi thread run : %f ms\n", tc.now());

    return 0;
}