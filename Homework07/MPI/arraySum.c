/* arraySum.c uses an array to sum the values in an input file,
 *  whose name is specified on the command-line.
 * Joel Adams, Fall 2005
 * for CS 374 (HPC) at Calvin College.
 * 
 * Extended by Josh Bussis for CS 374, project 7
 * This was done by adding MPI to parallelize the summing of the numbers through
 *    MPI_Scatter and MPI_Reduce
 * Date: 11/13/2019
 * Where: Calvin University
 */

#include <stdio.h>      /* I/O stuff */
#include <stdlib.h>     /* calloc, etc. */
#include <mpi.h>
#include <math.h>

void readArray(char * fileName, double ** a, int * n);
double sumArray(double * a, int numValues) ;

int main(int argc, char * argv[])
{
  // mpi variables
  int id, numProcesses, remainder;
  unsigned int localChunk;
  double localSum;

  //timing variables
  double totalTime, ioTime, scatterTime, sumTime;
  double startTime, startIoTime, startScatterTime, startSumTime;

  int  howMany;
  double sum;
  double * a;
  double * localA;

  if (argc != 2) {
    fprintf(stderr, "\n*** Usage: arraySum <inputFile>\n\n");
    exit(1);
  }

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &id);
  MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);
  startTime = MPI_Wtime();
  
  // only the master process should read the file
  if (id == 0) {
    // read the 
    startIoTime = MPI_Wtime();
    readArray(argv[1], &a, &howMany);
    ioTime = MPI_Wtime() - startIoTime;
  }
  // make sure all PEs have the correct number for allocating their arrays
  MPI_Bcast(&howMany, 1, MPI_INT, 0, MPI_COMM_WORLD);
  localChunk = (unsigned int)(howMany / numProcesses);
  // make the local array
  localA = malloc(sizeof(double) * localChunk);
  // send out the work to the worker processes
  startScatterTime = MPI_Wtime();
  MPI_Scatter(a, localChunk, MPI_DOUBLE, localA, localChunk, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  scatterTime = MPI_Wtime() - startScatterTime;
  // calculate the localSum
  startSumTime = MPI_Wtime();
  localSum = sumArray(localA, localChunk);
  // Reduction from all the localSums to sum
  MPI_Reduce(&localSum, &sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

  // have the master process take care of the final chunks and printing results
  if (id == 0) {
    remainder = howMany % numProcesses;
    // take care of the last bits if there was some remainder that wasn't calulated before
    if (remainder != 0) {
      for (int i = 0; i < remainder; i++) {
        sum += a[localChunk * numProcesses + i];
      }
    }
    sumTime = MPI_Wtime() - startSumTime;
    totalTime = MPI_Wtime() - startTime;
    printf("\nThe sum of the values in the input file '%s' is %g\n\n",
           argv[1], sum);
    printf("Total time of program:   %lf\n", totalTime);
    printf("I/O time of program:     %lf\n", ioTime);
    printf("Scatter time of program: %lf\n", scatterTime);
    printf("Summing time of program: %lf\n", sumTime);
  }

  if (id == 0) {
    // id 0 has to free dynamic array "a" because only id 0 allocates memory for it
    free(a);
  }
  // clean up and finalize
  free(localA);
  MPI_Finalize();
  return 0;
}

/* readArray fills an array with values from a file.
 * Receive: fileName, a char*,
 *          a, the address of a pointer to an array,
 *          n, the address of an int.
 * PRE: fileName contains N, followed by N double values.
 * POST: a points to a dynamically allocated array
 *        containing the N values from fileName
 *        and n == N.
 */

void readArray(char * fileName, double ** a, int * n) {
  int count, howMany;
  double * tempA;
  FILE * fin;

  fin = fopen(fileName, "r");
  if (fin == NULL) {
    fprintf(stderr, "\n*** Unable to open input file '%s'\n\n",
                     fileName);
    exit(1);
  }

  fscanf(fin, "%d", &howMany);
  tempA = calloc(howMany, sizeof(double));
  if (tempA == NULL) {
    fprintf(stderr, "\n*** Unable to allocate %d-length array",
                     howMany);
    exit(1);
  }

  for (count = 0; count < howMany; count++)
   fscanf(fin, "%lf", &tempA[count]);

  fclose(fin);

  *n = howMany;
  *a = tempA;
}

/* sumArray sums the values in an array of doubles.
 * Receive: a, a pointer to the head of an array;
 *          numValues, the number of values in the array.
 * Return: the sum of the values in the array.
 */

double sumArray(double * a, int numValues) {
  int i;
  double result = 0.0;

  for (i = 0; i < numValues; i++) {
    result += *a;
    a++;
  }

  return result;
}

