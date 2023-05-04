#include <mpi.h>
#include <stdio.h>
#include <time.h>

// 二次函数
double f(double x) {
    return x * x;
}

double Trap(double local_a, double local_b, int local_n, double h) {
    double result, x;

    result = (f(local_a) + f(local_b)) / 2.0;

    for (int i = 1; i <= local_n - 1; i++) {
        result += f(local_a + i * h);
    }

    result = result * h;

    return result;
}

int main(int argc, char* argv[]) {
    clock_t start, finish;

    start = clock();  // 开始计时

    MPI_Init(&argc, &argv);  // 初始化

    int n = 24000000;  // 粒度
    double a = 0.0, b = 50000;

    int rank, thread_num;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);        // 进程编号
    MPI_Comm_size(MPI_COMM_WORLD, &thread_num);  // 进程数

    double h = (b - a) / n;
    int local_n = n / thread_num;

    double local_a = a + rank * local_n * h;
    double local_b = local_a + local_n * h;

    double local_result = Trap(local_a, local_b, local_n, h);

    if (rank != 0) {
        // 从进程发送自己的计算结果
        MPI_Send(&local_result, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    } else {
        // 主进程接收从进程的结果并进行求和
        double result = local_result;

        for (int source = 1; source < thread_num; source++) {
            MPI_Recv(&local_result, 1, MPI_DOUBLE, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            result += local_result;
        }

        finish = clock();                                         // 结束计时
        double cost = (double)(finish - start) / CLOCKS_PER_SEC;  // 获取时间消耗

        printf("n = %d \n", n);
        printf("from %f to %f = %f, cost = %f\n", a, b, result, cost);
    }

    MPI_Finalize();

    return 0;
}
