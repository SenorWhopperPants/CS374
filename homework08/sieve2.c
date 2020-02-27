/* sieve.c finds prime numbers using a parallel/MPI version
 *  of Eratosthenes Sieve.
 * Based on an implementation by Quinn
 * Modified by Ryan Holt to correctly handle '-np 1', Fall 2007
 * Modified by Nathan Dykhuis to handle larger ranges, Fall 2009
 *
 * Usage: mpirun -np N ./sieve <upperBound> <numThreads>
 *
 * Extended by Josh Bussis to include OpenMP multithreading
 * on 11/22/2019 at Calvin University for CS 374, project 8
 */

#include "mpi.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main (int argc, char ** argv) {
  int i;
  int n;
  int index;
  int size;
  int prime;
  int count;
  int global_count;
  int first;
  long int high_value;
  long int low_value;
  int comm_rank;
  int comm_size;
  char * marked;
  double runtime;
  
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  
  MPI_Barrier(MPI_COMM_WORLD);
  runtime = -MPI_Wtime();
  
  // Check for the command line argument.
  if (argc != 3) {
    if (comm_rank == 0) printf("\nUsage: ./sieve2 <upperBound> <numThreads>\n\n");
    MPI_Finalize();
    exit(1);
  }
  
  n = atoi(argv[1]);

  // get thread number from command line and initialize that number to omp
  omp_set_num_threads( atoi(argv[2]) );

  // Bail out if all the primes used for sieving are not all held by
  // process zero.
  if ((2 + (n - 1 / comm_size)) < (int) sqrt((double) n)) {
    if (comm_rank == 0) printf("Too many processes.\n");
    MPI_Finalize();
    exit(1);
  }
  
  // Figure out this process's share of the array, as well as the integers
  // represented by the first and last array elements.
  low_value  = 2 + (long int)(comm_rank) * (long int)(n - 1) / (long int)comm_size;
  high_value = 1 + (long int)(comm_rank + 1) * (long int)(n - 1) / (long int)comm_size;
  size = high_value - low_value + 1;
  
  marked = (char *) calloc(size, sizeof(char));
  
  if (marked == NULL) {
   printf("Cannot allocate enough memory.\n");
   MPI_Finalize();
   exit(1);
  }
  
  if (comm_rank == 0) index = 0;
  prime = 2;
  
  do {
    if (prime * prime > low_value) {
      first = prime * prime - low_value;
    } else {
      if ((low_value % prime) == 0) first = 0;
      else first = prime - (low_value % prime);
    }
    
    // use omp here
    #pragma omp parallel for
    for (i = first; i < size; i += prime) marked[i] = 1;
    
    if (comm_rank == 0) {
      while (marked[++index]);
      prime = index + 2;
    }
    
    if (comm_size > 1) MPI_Bcast(&prime,  1, MPI_INT, 0, MPI_COMM_WORLD);
  } while (prime * prime <= n);
  
  count = 0;
  // use omp here
  #pragma omp parallel for reduction(+: count)
  for (i = 0; i < size; i++) {
      if (marked[i] == 0) {
          count++;
      }
  }
  
  if (comm_size > 1) {
    MPI_Reduce(&count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
  } else {
    global_count = count;
  }
  
  runtime += MPI_Wtime();
  
  if (comm_rank == 0) {
    printf("In %f seconds we found %d primes less than or equal to %d.\n",
		runtime, global_count, n);
  }
  
  MPI_Finalize();
  free(marked);
  return 0;
}


