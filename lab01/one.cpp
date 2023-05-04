#include <nmmintrin.h>
#include <windows.h>
#include <iostream>
#include "myRand.h"
#include "myTime.h"
using namespace std;

#define SIZE 10  // 矩阵大小
#define T 20     // 分片大小

Rand r;

void runTime(void (*func)(float[][SIZE], float[][SIZE], float[][SIZE]),
             float a[][SIZE],
             float b[][SIZE],
             float c[][SIZE]) {
    TimerCounter tc;
    tc.StartCounter();
    func(a, b, c);
    cout << "run time is: " << tc.GetCounter() << "ms\n";
}

// 输出矩阵
void output(float m[][SIZE]) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE - 1; j++) {
            cout << m[i][j] << " ";
        }

        cout << m[i][SIZE - 1] << endl;
    }
}

// 初始化二维矩阵
void initMatrix(float m[][SIZE]) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            m[i][j] = r.getRand();
        }
    }
}

// 串行计算
void mul(float a[][SIZE], float b[][SIZE], float c[][SIZE]) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            c[i][j] = 0;
            for (int k = 0; k < SIZE; k++) {
                c[i][j] += a[i][k] * b[k][j];
            }
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

// cache 优化
void trans_mul(float a[][SIZE], float b[][SIZE], float c[][SIZE]) {
    trans(b);

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            c[i][j] = 0;

            for (int k = 0; k < SIZE; k++) {
                c[i][j] += a[i][k] * b[j][k];
            }
        }
    }

    trans(b);
}

// SSE 优化
void sse_mul(float a[][SIZE], float b[][SIZE], float c[][SIZE]) {
    __m128 t1, t2, t3, sum;

    trans(b);

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            c[i][j] = 0;

            // 清零操作, r0 = r1 = r2 = r3 = 0.0
            sum = _mm_setzero_ps();

            // 4 组数同时运算
            for (int k = SIZE - 4; k >= 0; k -= 4) {
                // 将四个单精度浮点数加载到寄存器
                // r0 = p[0], r1 = p[1], r2 = p[2], r3 = p[3]
                t1 = _mm_loadu_ps(a[i] + k);
                t2 = _mm_loadu_ps(b[j] + k);

                // t3[i : i + 3] = a[i : i + 3] * b[i : i + 3]
                t3 = _mm_mul_ps(t1, t2);

                // 类似于 sum += t1
                sum = _mm_add_ps(sum, t3);
            }

            // (A1, A2, A3, A4)
            // (A1, A2, A3, A4)
            // 进行 _mm_hadd_ps 后得 (A3 + A4, A1 + A2, A3 + A4, A1 + A2)
            // 再进行一次操作变成 (A1 + A2 + A2 + A4, ...)
            sum = _mm_hadd_ps(sum, sum);
            sum = _mm_hadd_ps(sum, sum);

            // 把求和结果放入 c[i][j]
            _mm_store_ss(c[i] + j, sum);

            // 处理剩下的部分
            // 例如矩阵列数不能被 4 整除, 上面的操作还会留下前面一小部分未求和
            for (int k = (SIZE % 4) - 1; k >= 0; k--) {
                c[i][j] += a[i][k] * b[j][k];
            }
        }
    }

    trans(b);
}

// 分片策略
void sse_tile(float a[][SIZE], float b[][SIZE], float c[][SIZE]) {
    __m128 t1, t2, t3, sum;
    float t;

    trans(b);

    for (int r = 0; r < SIZE / T; ++r) {
        for (int q = 0; q < SIZE / T; ++q) {
            // 相当于前面 c[i][j] = 0 操作, 把子矩阵所有元素置 0
            for (int i = 0; i < T; ++i) {
                for (int j = 0; j < T; ++j) {
                    c[r * T + i][q * T + j] = 0.0;
                }
            }

            // 子矩阵进行 sse 操作
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
    initMatrix(a);
    initMatrix(b);

    cout << "========= mul =========\n";
    runTime(&mul, a, b, c);
    output(c);

    cout << "====== trans_mul ======\n";
    runTime(&trans_mul, a, b, c);

    cout << "========= SSE =========\n";
    runTime(&sse_mul, a, b, c);

    cout << "======= SSE tile ======\n";
    runTime(&sse_tile, a, b, c);
    output(c);

    return 0;
}