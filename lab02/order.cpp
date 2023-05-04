#include <algorithm>
#include <cmath>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include "random.h"
#include "timer.h"
using namespace std;

TimerCounter tc;  // 计时器
mutex g_lock;     // 锁

// 二维数组初始化
void init(vector<vector<int>>& arr) {
    for (int i = 0; i < arr.size(); i++) {
        for (int j = 0; j < arr[i].size(); j++)
            arr[i][j] = randint();
    }
}

void init_2(int tnum, vector<vector<int>>& arr) {
    int row = arr.size();

    int ratio, seg = row / tnum, m = (int)pow(2, tnum + 3);

    for (int i = 0; i < row; i++) {
        ratio = i < seg ? 0 : (int)pow(2, tnum + i / seg);

        // 获取 [0, 2^(tnum + 3)) 间的数
        if ((radom() & m - 1) < ratio) {
            for (int j = 0; j < arr[i].size(); j++) {
                arr[i][j] = arr[i].size() - j - 1;  // 逆序
            }
        } else {
            for (int j = 0; j < arr[i].size(); j++) {
                arr[i][j] = j;  // 升序
            }
        }
    }
}

// 数组排序
void arrSort(int tid, int seg, vector<vector<int>>& arr) {
    for (int i = tid * seg; i < (tid + 1) * seg; i++) {
        sort(arr[i].begin(), arr[i].end());
    }

    g_lock.lock();  // 加锁

    double t = tc.now();

    printf("Thread %d run %f ms\n", tid, t);

    g_lock.unlock();  // 解锁
}

int next_arr = 0;
mutex task_lock;
// 动态任务划分
void arrSortFine(int tid, vector<vector<int>>& arr) {
    int task = 0;

    while (1) {
        task_lock.lock();

        task = next_arr++;

        task_lock.unlock();

        if (task >= arr.size())
            break;

        sort(arr[task].begin(), arr[task].end());
    }

    g_lock.lock();  // 加锁

    double t = tc.now();

    printf("Thread %d run %f ms\n", tid, t);

    g_lock.unlock();  // 解锁
}

// 验证排序正确性
void output(vector<vector<int>>& arr) {
    for (int i = 0; i < arr.size(); i++) {
        for (int j = 0; j < arr[i].size(); j++) {
            cout << arr[i][j] << " ";
        }
        cout << endl;
    }
}

int main(int argc, char* argv[]) {
    vector<int> p(3);

    // 默认值
    p[0] = 10000;
    p[1] = 10000;
    p[2] = 4;

    for (int i = 0; i < argc - 1; i++) {
        p[i] = atoi(argv[i + 1]);
    }

    int ARR_NUM = p[0],     // 数组个数
        ARR_LEN = p[1],     // 数组长度
        THREAD_NUM = p[2];  // 线程数(必须能被 ARR_NUM 整除)

    if (ARR_NUM % THREAD_NUM != 0)
        return 0;

    const int seg = ARR_NUM / THREAD_NUM;  // 每个线程需要排序的数组数

    vector<vector<int>> arr(ARR_NUM, vector<int>(ARR_LEN, 0));

    // init(arr);

    init_2(THREAD_NUM, arr);

    vector<thread> threads(THREAD_NUM);

    tc.start();  // 开始计时

    for (int i = 0; i < THREAD_NUM; i++) {
        threads[i] = thread(arrSort, i, seg, ref(arr));
        // threads[i] = thread(arrSortFine, i, ref(arr));
    }

    for (int i = 0; i < THREAD_NUM; i++) {
        threads[i].join();  // 等待进程结束
    }
}