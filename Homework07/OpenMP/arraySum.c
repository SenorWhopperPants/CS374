/* arraySum.c uses an array to sum the values in an input file,
 *  whose name is specified on the command-line.
 * Joel Adams, Fall 2005
 * for CS 374 (HPC) at Calvin College.
 * 
 * Extended by Josh Bussis for CS 374, project 7.
 * This was done my adding OpenMP to multithread the summation of the values in the file.
 * Date: 11/13/2019
 * Where: Calvin University
 */

#include <stdio.h>      /* I/O stuff */
#include <stdlib.h>     /* calloc, etc. */
#include <omp.h>

void readArray(char * fileName, double ** a, int * n);
double sumArray(double * a, int numValues) ;

int main(int argc, char * argv[])
{
  int id;
  int  howMany;
  double sum;
  double * a;

  // timing variables
  double totalTime, ioTime, sumTime;
  double startTime, startIoTime, startSumTime;

  startTime = omp_get_wtime();
  if (argc != 3) {
    fprintf(stderr, "\n*** Usage: arraySum <inputFile> <numThreads>\n\n");
    exit(1);
  }

  // set the number of threads
  omp_set_num_threads( atoi(argv[2]) );
  // make sure we're only using thread 0 (the master)
  #pragma omp parallel private(id)
  {
    id = omp_get_thread_num();
    // check if thread 0
    if (id == 0) {
      // read file
      startIoTime = omp_get_wtime();
      readArray(argv[1], &a, &howMany);
      ioTime = omp_get_wtime() - startIoTime;
    }
  }
  // sum the array
  startSumTime = omp_get_wtime();
  sum = sumArray(a, howMany);
  sumTime = omp_get_wtime() - startSumTime;
  totalTime = omp_get_wtime() - startTime;
  // print out results
  printf("\nThe sum of the values in the input file '%s' is %g\n\n",
           argv[1], sum);
  printf("Total time:   %lf\n", totalTime);
  printf("I/O time:     %lf\n", ioTime);
  printf("Summing time: %lf\n", sumTime);

  free(a);

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
  // parallelize the loop with omp threads
  #pragma omp parallel for
  for (i = 0; i < numValues; i++) {
    // make sure adding to the result is protected
    #pragma omp atomic
    result += a[i];
  }

  return result;
}

