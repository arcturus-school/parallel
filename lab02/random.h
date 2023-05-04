#include <cstdlib>
#include <ctime>

// 设置随机数种子
void seed(unsigned int s = time(0)) {
    srand(s);
}

// 获取 [a, b] 间随机数
int randint(int a = 0, int b = 100) {
    return rand() % (b - a + 1) + a;
}

// 获取 [0, RAND_MAX] 间随机数
int radom() {
    return rand();
}