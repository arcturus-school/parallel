#include <windows.h>

class TimerCounter {
    long long freq, head, tail;

   public:
    void start() {
        QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
        QueryPerformanceCounter((LARGE_INTEGER*)&head);
    }

    double now() {
        QueryPerformanceCounter((LARGE_INTEGER*)&tail);
        return (tail - head) * 1000.0 / freq;
    }
};