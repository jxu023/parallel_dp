a: main.c
	mpicc -o main main.c
run:
	mpiexec -np 8 ./main
clean:
	rm *.o
