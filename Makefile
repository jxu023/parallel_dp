a: main.c
	mpicc -o main main.c
run:
	mpiexec -np 2 ./main
clean:
	rm *.o
