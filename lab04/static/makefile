EXECS=mpi_static_sort

MPICC?=mpicc

all: ${EXECS}

mpi_static_sort: mpi_static_sort.c
	${MPICC} -o mpi_static_sort mpi_static_sort.c

clean:
	rm -f ${EXECS}

