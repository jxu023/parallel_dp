a: main.c
	mpicc -o main main.c
run:
	mpiexec -np 1 ./main
clean:
	rm *.o
