#include <windows.h>
#include <iostream>
#include "myRand.h"
#include "myTime.h"
using namespace std;

void outputMatrix(int n, float** a, float b[]) {
    cout << endl;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n - 1; j++) {
            cout << a[i][j] << "\t";
        }

        cout << a[i][n - 1] << "\t"
             << "| " << b[i] << endl;
    }
    cout << endl;
}

void outputAnswer(int n, float x[]) {
    cout << "The answes is:";
    for (int i = 0; i < n; i++) {
        cout << x[i] << " ";
    }

    cout << endl;
}

// 构造随机可解的线性方程组
void initLinearEquationGroup(int n, float** a, float* b) {
    Rand r;

    // 随机的 b 矩阵
    for (int i = 0; i < n; i++) {
        b[i] = r.getRand(0, 5);
    }

    // 随机的对角矩阵
    for (int i = 0; i < n; i++) {
        // 下三角全为 0
        for (int j = 0; j < i; j++) {
            a[i][j] = 0;
        }

        // 对角线全为 1
        a[i][i] = 1;

        // 上三角为随机的
        for (int j = i + 1; j < n; j++) {
            a[i][j] = r.getRand(0, 5);
        }
    }

    for (int i = 1; i < n; i++) {
        float temp = r.getRand(0, 2);  // 随机倍数

        for (int j = 0; j < n; j++) {
            // 第 i 行的各个元素加上第 0 行的 temp 倍
            a[i][j] += a[0][j] * temp;
        }

        b[i] += b[0] * temp;
    }
}

// 创建二维数组
float** create2DMatrix(int n) {
    float** a = new float*[n];

    for (int i = 0; i < n; i++) {
        a[i] = new float[n];
    }

    return a;
}

// 释放二维数组
void destroy2DMatrix(int n, float** a) {
    for (int i = 0; i < n; i++) {
        delete[] a[i];
    }

    delete[] a;
}

void copyMatrix(int n, float** a1, float* b1, float** a2, float* b2) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            a2[i][j] = a1[i][j];
        }

        b2[i] = b1[i];
    }
}

double runTime(void (*func)(int, float**, float*, float*),
               int n,
               float** a,
               float* b,
               float* x) {
    TimerCounter tc;
    tc.StartCounter();
    func(n, a, b, x);
    return tc.GetCounter();
}

// 普通方法
void gauss(int n, float** a, float b[], float x[]) {
    float temp;

    // 消元计算
    for (int k = 0; k < n; k++) {
        // 第 k 行整体除以 a[k][k]
        temp = a[k][k];
        for (int i = k; i < n; i++) {
            a[k][i] = a[k][i] / temp;
        }

        b[k] /= temp;

        // 遍历剩余行
        for (int i = k + 1; i < n; i++) {
            temp = a[i][k];

            for (int j = k; j < n; j++) {
                // 第 k 行扩大 a[i][k] 倍
                // 第 i 行减去第 k 行
                a[i][j] -= temp * a[k][j];
            }

            // b 矩阵进行同等操作
            b[i] -= temp * b[k];
        }
    }

    // 回代求解
    for (int i = n - 1; i >= 0; i--) {
        x[i] = b[i];

        for (int j = i + 1; j < n; j++) {
            x[i] -= a[i][j] * x[j];
        }

        x[i] /= a[i][i];
    }
}

// 回代时使用向量编程
void gauss_sse_huidai(int n, float** a, float b[], float x[]) {
    float temp;

    // 消元计算
    for (int k = 0; k < n; k++) {
        // 第 k 行整体除以 a[k][k]
        temp = a[k][k];
        for (int i = k; i < n; i++) {
            a[k][i] = a[k][i] / temp;
        }

        b[k] /= temp;

        // 遍历剩余行
        for (int i = k + 1; i < n; i++) {
            temp = a[i][k];

            for (int j = k; j < n; j++) {
                // 第 k 行扩大 a[i][k] 倍
                // 第 i 行减去第 k 行
                a[i][j] -= temp * a[k][j];
            }

            // b 矩阵进行同等操作
            b[i] -= temp * b[k];
        }
    }

    // 回代求解
    __m128 t1, t2, t3, t4;

    for (int i = n - 1; i >= 0; i--) {
        t1 = _mm_setzero_ps();

        for (int j = n - 4; j > i; j -= 4) {
            t2 = _mm_loadu_ps(a[i] + j);
            t3 = _mm_loadu_ps(x + j);
            t4 = _mm_mul_ps(t2, t3);
            t1 = _mm_sub_ps(t1, t4);
        }

        t1 = _mm_hadd_ps(t1, t1);
        t1 = _mm_hadd_ps(t1, t1);

        _mm_store_ss(x + i, t1);

        for (int j = i + 1; j < (n - i - 1) % 4 + i + 1; j++) {
            x[i] -= a[i][j] * x[j];
        }

        x[i] += b[i];

        x[i] /= a[i][i];
    }
}

// sse 优化
void gauss_sse(int n, float** a, float b[], float x[]) {
    __m128 t1, t2, t3, t4;
    float temp;

    // 消元计算
    for (int k = 0; k < n; k++) {
        temp = a[k][k];

        float t[4] = {temp, temp, temp, temp};
        t1 = _mm_loadu_ps(t);

        for (int i = n - 4; i >= k; i -= 4) {
            t2 = _mm_loadu_ps(a[k] + i);
            t3 = _mm_div_ps(t2, t1);
            _mm_storeu_ps(a[k] + i, t3);
        }

        for (int i = k; i < (n - k) % 4 + k; i++) {
            a[k][i] = a[k][i] / temp;
        }

        b[k] /= temp;

        // 遍历剩余行
        for (int i = k + 1; i < n; i++) {
            temp = a[i][k];

            float tt[4] = {temp, temp, temp, temp};
            t1 = _mm_loadu_ps(tt);

            for (int j = n - 4; j >= k; j -= 4) {
                t2 = _mm_loadu_ps(a[k] + j);
                t3 = _mm_loadu_ps(a[i] + j);
                t4 = _mm_sub_ps(t3, _mm_mul_ps(t2, t1));
                _mm_storeu_ps(a[i] + j, t4);
            }

            for (int j = k; j < (n - k) % 4 + k; j++) {
                a[i][j] -= temp * a[k][j];
            }

            b[i] -= temp * b[k];
        }
    }

    // 回代求解
    for (int i = n - 1; i >= 0; i--) {
        t1 = _mm_setzero_ps();

        for (int j = n - 4; j > i; j -= 4) {
            t2 = _mm_loadu_ps(a[i] + j);
            t3 = _mm_loadu_ps(x + j);
            t4 = _mm_mul_ps(t2, t3);
            t1 = _mm_sub_ps(t1, t4);
        }

        t1 = _mm_hadd_ps(t1, t1);
        t1 = _mm_hadd_ps(t1, t1);

        _mm_store_ss(x + i, t1);

        for (int j = i + 1; j < (n - i - 1) % 4 + i + 1; j++) {
            x[i] -= a[i][j] * x[j];
        }

        x[i] += b[i];

        x[i] /= a[i][i];
    }
}

// 编译: g++ -o gauss .\gauss.cpp -march=native
// avx 优化
void gauss_avx(int n, float** a, float b[], float x[]) {
    // cout << "avx:\n";

    __m256 t1, t2, t3, t4;
    __m128 s1, s2;
    float temp;

    // 消元计算
    for (int k = 0; k < n; k++) {
        temp = a[k][k];

        float t[8] = {temp, temp, temp, temp, temp, temp, temp, temp};
        t1 = _mm256_loadu_ps(t);

        for (int i = n - 8; i >= k; i -= 8) {
            t2 = _mm256_loadu_ps(a[k] + i);
            t3 = _mm256_div_ps(t2, t1);
            _mm256_storeu_ps(a[k] + i, t3);
        }

        for (int i = k; i < (n - k) % 8 + k; i++) {
            a[k][i] = a[k][i] / temp;
        }

        b[k] /= temp;

        // 遍历剩余行
        for (int i = k + 1; i < n; i++) {
            temp = a[i][k];

            float tt[8] = {temp, temp, temp, temp, temp, temp, temp, temp};
            t1 = _mm256_loadu_ps(tt);

            for (int j = n - 8; j >= k; j -= 8) {
                t2 = _mm256_loadu_ps(a[k] + j);
                t3 = _mm256_loadu_ps(a[i] + j);
                t4 = _mm256_sub_ps(t3, _mm256_mul_ps(t2, t1));
                _mm256_storeu_ps(a[i] + j, t4);
            }

            for (int j = k; j < (n - k) % 8 + k; j++) {
                a[i][j] -= temp * a[k][j];
            }

            // b 矩阵进行同等操作
            b[i] -= temp * b[k];
        }
    }

    // 回代求解
    for (int i = n - 1; i >= 0; i--) {
        t1 = _mm256_setzero_ps();
        for (int j = n - 8; j > i; j -= 4) {
            t2 = _mm256_loadu_ps(a[i] + j);
            t3 = _mm256_loadu_ps(x + j);
            t4 = _mm256_mul_ps(t2, t3);
            t1 = _mm256_sub_ps(t1, t4);
        }

        s1 = _mm256_extractf128_ps(t1, 0);
        s2 = _mm256_extractf128_ps(t1, 1);

        s1 = _mm_hadd_ps(s1, s2);
        s1 = _mm_hadd_ps(s1, s1);
        s1 = _mm_hadd_ps(s1, s1);

        _mm_store_ss(x + i, s1);

        for (int j = i + 1; j < (n - i - 1) % 8 + i + 1; j++) {
            x[i] -= a[i][j] * x[j];
        }

        x[i] += b[i];

        x[i] /= a[i][i];
    }

    // outputAnswer(n, x);
}

int main() {
    // 步长为 5 矩阵规模逐渐增加
    for (int i = 5; i <= 400; i += 5) {
        float** a = create2DMatrix(i);
        float* b = new float[i];
        float* x = new float[i];
        float** a1 = create2DMatrix(i);
        float* b1 = new float[i];

        cout << "n=" << i << " ";

        for (int j = 0; j < 3; j++) {
            // 重复三次
            initLinearEquationGroup(i, a, b);

            // 高斯消元法
            copyMatrix(i, a, b, a1, b1);
            double t1 = runTime(&gauss, i, a1, b1, x);

            // 回代时向量化
            copyMatrix(i, a, b, a1, b1);
            double t2 = runTime(&gauss_sse_huidai, i, a1, b1, x);

            // SSE 优化
            // copyMatrix(i, a, b, a1, b1);
            // double t2 = runTime(&gauss_sse, i, a1, b1, x);

            // AVX 优化
            // copyMatrix(i, a, b, a1, b1);
            // double t3 = runTime(&gauss_avx, i, a1, b1, x);

            cout << t1 << " " << t2 << " ";
            // cout << t1 << " " << t2 << " " << t3 << " ";
        }

        cout << endl;

        destroy2DMatrix(i, a);
        destroy2DMatrix(i, a1);
        delete[] b;
        delete[] b1;
        delete[] x;
    }

    return 0;
}