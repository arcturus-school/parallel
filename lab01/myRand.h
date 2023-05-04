#include <cstdlib>
#include <ctime>

// 获取随机浮点数
class Rand {
   public:
    Rand() { srand(time(0)); };
    // 获取 [a, b] 随机数
    float getRand(int a = 0, int b = 100) { return rand() % (b - a + 1) + a; }
};