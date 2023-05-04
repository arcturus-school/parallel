#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ARR_NUM 900

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

void mpi_sort(int seg, int rank) {
    int buffer[seg][ARR_NUM];

    MPI_Recv(buffer, seg * ARR_NUM, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

    for (int i = 0; i < seg; i++) {
        qsort(&buffer[i][0], ARR_NUM, sizeof(int), cmpfunc);
    }

    MPI_Send(buffer, seg * ARR_NUM, MPI_INT, 0, status.MPI_TAG, MPI_COMM_WORLD);
}

int main(int argc, char* argv[]) {
    clock_t start, finish;

    start = clock();

    MPI_Init(&argc, &argv);

    int rank, process_num;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &process_num);

int seg = ARR_NUM / (process_num - 1);

if (rank == 0) {
    int source[ARR_NUM][ARR_NUM];
    int res[ARR_NUM][ARR_NUM];

    init(source);

    for (int i = 1; i < process_num; i++) {
        MPI_Send(&source[(i - 1) * seg][0], seg * ARR_NUM, MPI_INT, i, (i - 1) * seg, MPI_COMM_WORLD);
    }

    int buffer[seg][ARR_NUM];

    for (int i = 1; i < process_num; i++) {
        MPI_Recv(buffer, seg * ARR_NUM, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        memcpy(&res[status.MPI_TAG][0], buffer, sizeof(int) * ARR_NUM * seg);
    }

    finish = clock();
    double cost = (double)(finish - start) / CLOCKS_PER_SEC;

    printf("cost = %f\n", cost);
} else {
    mpi_sort(seg, rank);
}

    MPI_Finalize();

    return 0;
}
