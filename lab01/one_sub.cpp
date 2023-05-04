#include <nmmintrin.h>
#include <windows.h>
#include <iostream>
#include "myRand.h"
#include "myTime.h"
using namespace std;

#define SIZE 256  // 矩阵大小
Rand r;

// 输出矩阵
void output(float m[][SIZE]) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE - 1; j++) {
            cout << m[i][j] << " ";
        }

        cout << m[i][SIZE - 1] << endl;
    }
}

void runTime(void (*func)(float[][SIZE], float[][SIZE], float[][SIZE], int),
             float a[][SIZE],
             float b[][SIZE],
             float c[][SIZE],
             int T) {
    TimerCounter tc;
    tc.StartCounter();
    func(a, b, c, T);
    cout << "run time is: " << tc.GetCounter() << "ms\n";
}

// 初始化二维矩阵
void initMatrix(float m[][SIZE]) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            m[i][j] = r.getRand();
        }
    }
}

// 转置
void trans(float t[][SIZE]) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < i; j++) {
            swap(t[i][j], t[j][i]);
        }
    }
}

// 分片策略
void sse_tile(float a[][SIZE], float b[][SIZE], float c[][SIZE], int T) {
    __m128 t1, t2, t3, sum;
    float t;

    trans(b);

    for (int r = 0; r < SIZE / T; ++r) {
        for (int q = 0; q < SIZE / T; ++q) {
            for (int i = 0; i < T; ++i) {
                for (int j = 0; j < T; ++j) {
                    c[r * T + i][q * T + j] = 0.0;
                }
            }

            for (int p = 0; p < SIZE / T; ++p) {
                for (int i = 0; i < T; ++i) {
                    for (int j = 0; j < T; ++j) {
                        sum = _mm_setzero_ps();

                        for (int k = 0; k < T; k += 4) {
                            t1 = _mm_loadu_ps(a[r * T + i] + p * T + k);
                            t2 = _mm_loadu_ps(b[q * T + j] + p * T + k);
                            t3 = _mm_mul_ps(t1, t2);
                            sum = _mm_add_ps(sum, t3);
                        }

                        sum = _mm_hadd_ps(sum, sum);
                        sum = _mm_hadd_ps(sum, sum);
                        _mm_store_ss(&t, sum);

                        c[r * T + i][q * T + j] += t;
                    }
                }
            }
        }
    }

    trans(b);
}

// 这里必须声明为全局变量, 否则数组长度超过 400 多后会出现栈溢出
float a[SIZE][SIZE], b[SIZE][SIZE], c[SIZE][SIZE];

int main() {
    TimerCounter tc;

    initMatrix(a);
    initMatrix(b);

    cout << "======= SSE tile ======\n";
    for (int i = 64; i <= 16384; i *= 2) {
        cout << "======= " << i << " ======\n";
        runTime(&sse_tile, a, b, c, i);
    }

    return 0;
}