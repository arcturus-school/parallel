#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <mutex>
#include <thread>
#include <vector>
#include "timer.h"
using namespace std;

pthread_barrier_t barrier;
sem_t sem_count, barrier_sem;
int counter = 0;  // 有多少线程抵达路障

mutex c_lock, b_lock;  // 计数器锁

TimerCounter tc;

void default_barrier(int tid) {
    printf("Thread %d has entered step 1.\n", tid);
    pthread_barrier_wait(&barrier);  // 等待路障
    printf("Thread %d has entered step 2.\n", tid);
}

void sem_barrier_wait(sem_t* c, sem_t* b, int tnum) {
    sem_wait(c);  // 减至 0, 防止其他线程进来

    if (counter == tnum - 1) {
        counter = 0;
        sem_post(c);
        for (int i = 0; i < tnum - 1; i++) {
            // 唤醒 tnum - 1 个线程
            // 因为唤醒一个线程 b 就减一, 因此要加 tnum - 1 次才能全部唤醒
            sem_post(b);
        }
    } else {
        counter++;
        sem_post(c);  // 重新增加至 1, 允许其他线程进来
        sem_wait(b);  // 整个线程阻塞, 等待 b 变成非 0
    }
}

void mutex_barrier_wait(int tid, int tnum) {
    c_lock.lock();

    if (counter == tnum - 1) {
        counter = 0;
        c_lock.unlock();
        b_lock.unlock();
    } else {
        counter++;
        c_lock.unlock();
        b_lock.lock();
        b_lock.unlock();
    }
}

void sem_thread_barrier(int tid, int NUM_THREADS) {
    printf("Thread %d has entered step 1.\n", tid);
    sem_barrier_wait(&sem_count, &barrier_sem, NUM_THREADS);
    printf("Thread %d has entered step 2.\n", tid);
}

void mutex_thread_barrier(int tid, int NUM_THREADS) {
    printf("Thread %d has entered step 1.\n", tid);
    mutex_barrier_wait(tid, NUM_THREADS);
    printf("Thread %d has entered step 2.\n", tid);
}

int main(int argc, char* argv[]) {
    int NUM_THREADS = 4;  // 默认线程数
    if (argc == 2) {
        NUM_THREADS = atoi(argv[1]);
    }

    // pthread_barrier_init(&barrier, NULL, NUM_THREADS);  // 初始化
    // sem_init(&sem_count, 0, 1);
    // sem_init(&barrier_sem, 0, 0);

    vector<thread> threads(NUM_THREADS);

    tc.start();

    b_lock.lock();
    for (int i = 0; i < NUM_THREADS; i++) {
        // threads[i] = thread(default_barrier, i);
        // threads[i] = thread(sem_thread_barrier, i, NUM_THREADS);
        threads[i] = thread(mutex_thread_barrier, i, NUM_THREADS);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i].join();
    }

    printf("The run time is %f ms", tc.now());

    // pthread_barrier_destroy(&barrier);
    // sem_destroy(&sem_count);
    // sem_destroy(&barrier_sem);

    return 0;
}