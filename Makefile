a: main.c
	mpicc -o main main.c
run:
	mpiexec -np 4 ./main
clean:
	rm *.o
