#include "mpi.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#define MAX(A,B) ((A > B) ? A : B)

//doing threads shares resources

// need INT_MIN to replace negative infinity
// INT_MAX	+2,147,483,647
// note : 2 billion, string length limited to this
// for longer string length benchmarks the need to replace with long
// LONG_MIN	-9223372036854775808	Defines the minimum value for a long int.
// LONG_MAX	+9223372036854775807	Defines the maximum value for a long int.
// in that event, the current plan is just to do a search replace command... o_o

// m by n times n by 1 gets m by 1
void multiply(int * matrix, int * vector)
{
	int i,j,k;
	// k i j loop iteration is ideal for locality
	// where a is i by j, b is k by j, and c multiplied becomes i by k
	// for matrix matrix multiplication
	// consider again for matrix vector
}

void pred_(int * LCS, int m, int n) {
	
}
// assumes the base cases already initialized
// 
void dp_seq(int * LCS, int m, int n, char * a, char * b) {
	int i,j;
	// iterate row by row, other way is negative diagonal
	for (i = 1; i < m; ++i) {
		for (j = 1; j < n; ++j) {
			if (a[i-1] == b[j-1]) // string is at len 1 for ind 0, matrix is at ind 0 len 0
				LCS[i*n + j] = LCS[(i-1)*n+j-1] + 1;
			else
				LCS[i*n + j] = MAX(LCS[(i-1)*n+j],LCS[i*n+j-1]);
		}
	}
}

bool is_parallel(int * a, int * b, int n) {
	int i,diff,prev_diff;
	prev_diff = a[0] - b[0]
	for (i = 1; i < n; ++i) {
		diff = a[i] - b[i];
		if (diff != prev_diff)
			return false;
		prev_diff = diff;
	}
	return true;
}

void print(int * LCS, int m, int n) {
	int i,j;
	for (i = 0; i < m; ++i) {
		for (j = 0; j < n; ++j) {
			printf(" %d ",LCS[i*n + j]);
		}
		printf("\n");
	}
}

void insert_rand_row(int * LCS, int cols) {
	int i;
	for (i = 1; i < cols; ++i) {
		LCS[i] = rand() % 10 + 1;
	}
}

char * rand_string(int n) {
	int i;
	char * ret = malloc(n*sizeof(int));
	for (i = 0; i < n; ++i) {
		ret[i] = 'a' + rand() % 4; // just doing abcd for now
	}
	return ret;
}

// need a minimum of 3 processors to parallelize
// fastest result is two iterations over array
int main (int argc, char *argv[])
{
	srand(13);
	int p,id;

	// could generate random strings of numbers for this too
	char * a = rand_string(
	char * b = "absdced";
	// strlen() + 1 is used to include length 0
	int m = strlen(a)+1;
	int n = strlen(b)+1;
	int * LCS = (int *)calloc(m*n,sizeof(int));

	MPI_Init (&argc, &argv);
	MPI_Comm_rank (MPI_COMM_WORLD, &id);
	MPI_Comm_size (MPI_COMM_WORLD, &p);
	MPI_Barrier(MPI_COMM_WORLD);

	double elapsed_time = -MPI_Wtime();

	// currently assume m % 2 == 0
	if (p > 1)
	{
		// parallel forward phase
		if (!id) {
			dp_seq(LCS,m/2,n,a,b);
		}
		else {
			insert_rand_row(LCS+(m/2)*n,n);
			dp_seq(LCS+(m/2)*n,m/2,n,a,b);
		}
		//fix up phase

	}
	else {
		dp_seq(LCS,m,n,a,b);
	}


	elapsed_time+=MPI_Wtime();
	if (!id) {
		printf("%s\n",a);
		printf("%s\n",b);
		if (p == 1)
			print(LCS,m,n);
		printf("LCS[m,n] = %d\n",LCS[(m-1)*n + n-1]);
		printf("\nelapsed_time:%f\n",elapsed_time);
	}else {
		print(LCS,m,n);
	}

	free(LCS);
	MPI_Finalize ();
	return 0;
}

