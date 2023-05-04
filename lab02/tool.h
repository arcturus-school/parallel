#include <iostream>
#include "random.h"
#include "type.h"

// 输出矩阵
void outputMatrix(Matrix<float>& a, Array<float>& b) {
    for (int i = 0; i < a.size(); i++) {
        for (int j = 0; j < a[i].size(); j++) {
            std::cout << a[i][j] << "\t";
        }

        std::cout << "| " << b[i] << std::endl;
    }

    std::cout << std::endl;
}

// 输出答案
void outputAnswer(Array<float>& x) {
    std::cout << "\nThe answes is:";

    for (int i = 0; i < x.size(); i++) {
        std::cout << x[i] << " ";
    }

    std::cout << std::endl;
}

// 构造随机可解的线性方程组
void initLinearEquationGroup(Matrix<float>& a, Array<float>& b) {
    seed();

    // 随机的 b 矩阵
    for (int i = 0; i < b.size(); i++) {
        b[i] = randint(0, 5);
    }

    // 随机的对角矩阵
    for (int i = 0; i < a.size(); i++) {
        // 下三角全为 0
        for (int j = 0; j < i; j++) {
            a[i][j] = 0;
        }

        // 对角线全为 1
        a[i][i] = 1;

        // 上三角为随机的
        for (int j = i + 1; j < a[i].size(); j++) {
            a[i][j] = randint(0, 5);
        }
    }

    for (int i = 1; i < a.size(); i++) {
        float temp = randint(0, 2);  // 随机倍数

        for (int j = 0; j < a[i].size(); j++) {
            // 第 i 行的各个元素加上第 0 行的 temp 倍
            a[i][j] += a[0][j] * temp;
        }

        b[i] += b[0] * temp;
    }
}