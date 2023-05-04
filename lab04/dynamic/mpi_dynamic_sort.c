#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ARR_NUM 12
#define SEG 50
// 每个线程至少被分配到任务, 因此 seg 不能大于 ARR_NUM / process_num
// 5 个线程时, 大小为 100 的数组 seg 最大不能超过 20
// 并且要求能完成排序, seg 还要能被 ARR_NUM 整除

MPI_Status status;

int cmpfunc(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

void seed(unsigned int s) {
    srand(s);
}

int randint(int a, int b) {
    return rand() % (b - a + 1) + a;
}

void init(int source[][ARR_NUM]) {
    seed(time(0));

    for (int i = 0; i < ARR_NUM; i++) {
        for (int j = 0; j < ARR_NUM; j++) {
            source[i][j] = randint(0, 1000);
        }
    }
}

void show(int source[][ARR_NUM]) {
    for (int i = 0; i < ARR_NUM; i++) {
        for (int j = 0; j < ARR_NUM; j++) {
            printf("%d ", source[i][j]);
        }

        printf("\n");
    }

    printf("\n");
}

void mpi_sort() {
    int buffer[SEG][ARR_NUM];

    while (1) {
        MPI_Recv(buffer, SEG * ARR_NUM, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if (status.MPI_TAG >= ARR_NUM)
            break;

        for (int i = 0; i < SEG; i++) {
            qsort(&buffer[i][0], ARR_NUM, sizeof(int), cmpfunc);
        }

        MPI_Send(buffer, SEG * ARR_NUM, MPI_INT, 0, status.MPI_TAG, MPI_COMM_WORLD);
    }
}

int main(int argc, char* argv[]) {
    clock_t start, finish;

    start = clock();

    MPI_Init(&argc, &argv);

    int rank, process_num;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &process_num);

    if (rank == 0) {
        int source[ARR_NUM + 1][ARR_NUM];
        int res[ARR_NUM + 1][ARR_NUM];
        int buffer[SEG][ARR_NUM];

        init(source);

        int i, finished = 1;

        for (i = 1; i < process_num; i++) {
            MPI_Send(&source[(i - 1) * SEG][0], SEG * ARR_NUM, MPI_INT, i, (i - 1) * SEG, MPI_COMM_WORLD);
        }

        while (finished < process_num) {
            MPI_Recv(buffer, SEG * ARR_NUM, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            memcpy(&res[status.MPI_TAG][0], buffer, sizeof(int) * ARR_NUM * SEG);

            if ((i - 1) * SEG < ARR_NUM) {
                MPI_Send(&source[(i - 1) * SEG][0], SEG * ARR_NUM, MPI_INT, status.MPI_SOURCE, (i - 1) * SEG, MPI_COMM_WORLD);
                i++;
            } else {
                MPI_Send(&source[ARR_NUM][0], SEG * ARR_NUM, MPI_INT, status.MPI_SOURCE, ARR_NUM, MPI_COMM_WORLD);
                finished++;
            }
        }

        finish = clock();
        double cost = (double)(finish - start) / CLOCKS_PER_SEC;
        printf("array size = %d, cost = %f\n", ARR_NUM, cost);

        show(res);
    } else {
        mpi_sort();
    }

    MPI_Finalize();

    return 0;
}
