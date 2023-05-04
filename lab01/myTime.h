#include <windows.h>

class TimerCounter {
   public:
    long long freq, head, tail;

    void StartCounter() {
        QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
        QueryPerformanceCounter((LARGE_INTEGER*)&head);
    }

    double GetCounter() {
        QueryPerformanceCounter((LARGE_INTEGER*)&tail);
        return (tail - head) * 1000.0 / freq;
    }
};