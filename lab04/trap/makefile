EXECS=mpi_trap

MPICC?=mpicc

all: ${EXECS}

mpi_trap: mpi_trap.c
	${MPICC} -o mpi_trap mpi_trap.c

clean:
	rm -f ${EXECS}

