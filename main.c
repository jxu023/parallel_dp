#include "mpi.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#define MIN(A,B) ((A < B) ? A : B)
#define MAX(A,B) ((A > B) ? A : B)

//doing threads shares resources
	// avoids communication overhead

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

void * reverse (char * s,int n) {
	int i;
	char temp;
	for (i = 0; i < n/2; ++i) {
		temp = s[i];
		s[i] = s[n-i-1];
		s[n-i-1] = temp;
	}
}
char * backtrack(int * LCS, int n, int start_stage, char * a, char * b, int i, int * jj) {
	int j = *jj;
	char * ret = malloc(MIN(i,j)*sizeof(char));
	int pos = 0;
	while(i > 0) {
		if (a[start_stage + i-1] == b[j-1]) {
			ret[pos++] = a[start_stage + i-1];
			--i; --j;
		} else {
			if (LCS[(i-1)*n + j] > LCS[i*n + j-1])
				--i;
			else
				--j;
		}
	}
	ret[pos] = '\0';
	reverse(ret,pos);
	*jj = j;
	return ret;
}
// might want to make this inline
void dp_stage(int * stage, int n, char a, char * b) {
	int j;
	for (j = 1; j < n; ++j) {
		if (a == b[j-1]) // string is at len 1 for ind 0, matrix is at ind 0 len 0
			stage[j] = stage[j-n-1] + 1;
		else
			stage[j] = MAX(stage[j-n],stage[j-1]);
	}
}
void dp_seq(int * LCS, int m, int n, char * a, char * b) {
	int i;
	// iterate row by row, other way is negative diagonal
	for (i = 1; i < m; ++i) {
		dp_stage(LCS + i*n,n,a[i-1],b);
	}
}

int is_parallel(int * a, int * b, int n) {
	int i,diff,prev_diff;
	prev_diff = a[0] - b[0];
	for (i = 1; i < n; ++i) {
		diff = a[i] - b[i];
		if (diff != prev_diff)
			return 0;
		prev_diff = diff;
	}
	return 1;
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

void print_id(int * LCS, int m, int n,int id) {
	int i,j;
	for (i = 0; i < m; ++i) {
		for (j = 0; j < n; ++j) {
			printf(" %d ",LCS[i*n + j]);
		}
		printf("id: %d\n",id);
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

void * copy_array(int * a, int * b,int n) {
	int i;
	for (i = 0; i < n; ++i) {
		a[i] = b[i];
	}
}

// need a minimum of 3 processors to parallelize
// fastest result is two iterations over array
int main (int argc, char *argv[])
{
	srand(13);
	int p,id;

	int size = 1000;
	char * a = rand_string(size);
	char * b = rand_string(size);
	// strlen() + 1 is used to include length 0
	int m = strlen(a)+1;
	int n = strlen(b)+1;

	MPI_Init (&argc, &argv);
	MPI_Comm_rank (MPI_COMM_WORLD, &id);
	MPI_Comm_size (MPI_COMM_WORLD, &p);

	int first_stage = id*m/p;
	int last_stage = (id+1)*m/p-1;
	int stages = last_stage-first_stage+1;
	int * LCS = (int *)calloc(stages*n,sizeof(int));

	if (!id)
		printf("p:%d\n",p);
	printf("p:%d first:%d last:%d\n",id,first_stage,last_stage);

	double elapsed_time = -MPI_Wtime();

	int i,j;
	// currently assume m % 2 == 0
	if (p > 1)
	{
		// parallel forward phase
		if (id)
			insert_rand_row(LCS,n);
		for (i = 1; i < stages; ++i)
			dp_stage(LCS + i*n,n,a[first_stage + i-1],b);


		//fix up phase
		if (!id) {
			// send the last stage
			int converged = 1;
			int all_converged = 0;
			int iterations = 0;
			while (!all_converged) {
				MPI_Send(LCS+(stages-1)*n,n,MPI_INT,id+1,0,MPI_COMM_WORLD);
				MPI_Allreduce(&converged,&all_converged,1,MPI_INT,MPI_LAND,MPI_COMM_WORLD);
				++iterations;
			}
			printf("iterations:%d\n",iterations);
		} else {
			int * stage = (int *)calloc(2*n,sizeof(int));
			int all_converged = 0;
			while (!all_converged) {
				int converged = 0;
				// send then receive doesn't block since goes into buffer, otherwise does recv then send here
				if (id != p-1)
					MPI_Send(LCS+(stages-1)*n,n,MPI_INT,id+1,0,MPI_COMM_WORLD);
				MPI_Recv(stage,n,MPI_INT,id-1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
				for (i = 0; i < stages; ++i) {
					dp_stage(stage + n,n,a[first_stage + i-1],b);
					if (is_parallel(stage + n, LCS + i*n,n)) {
						converged = 1;
						break;
					}
					copy_array(LCS+ i*n,stage + n,n);
					copy_array(stage,stage + n,n);
				}
				MPI_Allreduce(&converged,&all_converged,1,MPI_INT,MPI_LAND,MPI_COMM_WORLD);
			}
			free(stage);
		}
		int predi = stages;
		int predj = n;
		if (id == p-1) {
			char * sequence = backtrack(LCS,n,first_stage,a,b,stages-1,&predj);
			printf("id: %d backtrack:%s\n",id,sequence);
			//print_id(LCS,stages,n,id);
			MPI_Send(&predj,1,MPI_INT,id-1,0,MPI_COMM_WORLD);
		}
		else if (id) {
			MPI_Recv(&predj,1,MPI_INT,id+1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			char * sequence = backtrack(LCS,n,first_stage,a,b,stages-1,&predj);
			printf("id: %d backtrack:%s\n",id,sequence);
			//print_id(LCS,stages,n,id);
			MPI_Send(&predj,1,MPI_INT,id-1,0,MPI_COMM_WORLD);
		}
		else {
			MPI_Recv(&predj,1,MPI_INT,id+1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			char * sequence = backtrack(LCS,n,0,a,b,stages-1,&predj);
			printf("id: %d backtrack:%s\n",id,sequence);
			//print_id(LCS,stages,n,id);
		}
	}
	else {
		dp_seq(LCS,m,n,a,b);
	}


	elapsed_time+=MPI_Wtime();
	if (!id) {
		printf("%s\n",a);
		printf("%s\n",b);
		if (p == 1) {
			//print(LCS,m,n);
			printf("LCS[m,n] = %d\n",LCS[(m-1)*n + n-1]);
			char * back = backtrack(LCS,n,first_stage,a,b,m,&n);
			printf("lcs:%s\n",back);
			free(back);
		}
		printf("\nelapsed_time:%f\n",elapsed_time);
	}else {
		//print(LCS,m,n);
	}
	printf("Process %d has finished\n",id);

	free(LCS);
	free(a);
	free(b);
	MPI_Finalize ();
	return 0;
}

