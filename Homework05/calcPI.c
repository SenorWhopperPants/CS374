/* calcPi.c calculates PI using the integral of the unit circle.
 * Since the area of the unit circle is PI, 
 *  PI = 4 * the area of a quarter of the unit circle 
 *    (i.e., its integral from 0.0 to 1.0)
 *
 * Joel Adams, Fall 2013 for CS 374 at Calvin College.
 * Edited by Josh Bussis: added parallelization via the chunks method for 
 *                        calculating PI
 * @date  10/21/2019
 * @where Calvin University
 * @why   Project 05 
 * @class CS 374  
 */

#include "integral.h"   // integrate()
#include <stdio.h>      // printf(), etc.
#include <stdlib.h>     // exit()
#include <math.h>       // sqrt() 
#include <mpi.h>


/* function for unit circle (x^2 + y^2 = 1)
 * parameter: x, a double
 * return: sqrt(1 - x^2)
 */
double f(double x) {
   return sqrt(1.0 - x*x);
}

/* retrieve desired number of trapezoids from commandline arguments
 * parameters: argc: the argument count
 *             argv: the argument vector
 * return: the number of trapezoids to be used.
 */            
unsigned long long processCommandLine(int argc, char** argv) {
   if (argc == 1) {
       return 1;
   } else if (argc == 2) {
//       return atoi( argv[1] );
       return strtoull( argv[1], 0, 10 );
   } else {
       fprintf(stderr, "Usage: ./calcPI [numTrapezoids]");
       exit(1);
   }
}
 

int main(int argc, char** argv) {

   // variables for Homework5
   double startTime, totalTime = 0;
   int id, numProcesses;
   unsigned long long start, stop = -1;
   unsigned long long chunkSize1, chunkSize2, remainder;

   long double localApproximatePI = 0;
   long double totalApproximatePI = 0;
   const long double REFERENCE_PI = 3.141592653589793238462643383279L;

   // initialize MPI
   MPI_Init(&argc, &argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &id);
   MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);

   unsigned long long numTrapezoids = processCommandLine(argc, argv); 
   
   // calculate the chunk sizes
   chunkSize1 = (unsigned long long)ceil(numTrapezoids / numProcesses);
   remainder = numTrapezoids % numProcesses;

   if (remainder == 0 || (remainder != 0 && id < remainder)) {
      start = id * chunkSize1;
      stop = start + chunkSize1;
   }
   else {
      chunkSize2 = chunkSize1 - 1;

      start = (remainder * chunkSize1) + (chunkSize2 * (id - remainder));
      stop = start + chunkSize2;
   }

   // start timer
   startTime = MPI_Wtime();
   // send start and stop indicies into integrateTrap()
   localApproximatePI = integrateTrap(0.0, 1.0, numTrapezoids, start, stop, id);
   // get the results
   MPI_Reduce(&localApproximatePI, &totalApproximatePI, 1, MPI_LONG_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
   totalApproximatePI *= 4;

   // finish timing
   totalTime = MPI_Wtime() - startTime;

   if (id == 0) {
      // print results from process 0
      printf("Using %llu trapezoids, the approximate vs actual values of PI are:\n%.30Lf\n%.30Lf\n",
         numTrapezoids, totalApproximatePI, REFERENCE_PI);
      printf("\nRuntime for integrate function: %lf\n\n", totalTime);

   }
   MPI_Finalize();
   return 0;
}

