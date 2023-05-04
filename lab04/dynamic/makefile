EXECS=mpi_dynamic_sort

MPICC?=mpicc

all: ${EXECS}

mpi_dynamic_sort: mpi_dynamic_sort.c
	${MPICC} -o mpi_dynamic_sort mpi_dynamic_sort.c

clean:
	rm -f ${EXECS}

