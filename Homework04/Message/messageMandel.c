/* messageMandel.c
 * Compute/draw mandelbrot set using MPI/MPE commands
 *
 * Written Winter, 1998, W. David Laverell.
 *
 * Refactored Winter 2002, Joel Adams. 
 * 
 * Paralellised by   Josh Bussis -> Parallelization was done through using the message passing paradigm
 * Date              10/12/2019
 * Where             Calvin University
 * Why               Homework 04            
 */

#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <mpi.h>
#include <mpe.h>
#include "display.h"
#include <stdlib.h>


/* compute the Mandelbrot-set function for a given
 *  point in the complex plane.
 *
 * Receive: doubles x and y,
 *          complex c.
 * Modify: doubles ans_x and ans_y.
 * POST: ans_x and ans_y contain the results of the mandelbrot-set
 *        function for x, y, and c.
 */
void compute(double x, double y, double c_real, double c_imag,
              double *ans_x, double *ans_y)
{
        *ans_x = x*x - y*y + c_real;
        *ans_y = 2*x*y + c_imag;
}

/* compute the 'distance' between x and y.
 *
 * Receive: doubles x and y.
 * Return: x^2 + y^2.
 */
double distance(double x, double y)
{
        return x*x + y*y;
}


int main(int argc, char* argv[])
{
   const int  WINDOW_SIZE = 1024;

   int        n        = 0,
            ix       = 0,
            iy       = 0,
            button   = 0,
            id       = 0;
   double     spacing  = 0.005,
            x        = 0.0,
            y        = 0.0,
            c_real   = 0.0,
            c_imag   = 0.0,
            x_center = 1.0,
            y_center = 0.0;

   MPE_XGraph graph;

   // variables for homework 04
   double startTime, totalTime = -1;
   int numProcesses = -1;
   int x_coord_to_draw;
   char firstTimeInLoop;
   int count;
   int rowToCompute;
   int rowsDrawn = 0;
   int numRowsReceived = 0;
   char terminateSent = 0;
   int numRowsSent = 0;

   MPI_Init(&argc,&argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &id);
   MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);
/*
   // Uncomment this block for interactive use
   printf("\nEnter spacing (.005): "); fflush(stdout);
   scanf("%lf",&spacing);
   printf("\nEnter coordinates of center point (0,0): "); fflush(stdout);
   scanf("%lf %lf", &x_center, &y_center);
   printf("\nSpacing=%lf, center=(%lf,%lf)\n",
         spacing, x_center, y_center);
*/

   // make the local matrix the exact dimentions of the chunks that were calculated
   //short localMatrix[WINDOW_SIZE];
   short totalMatrix[WINDOW_SIZE][WINDOW_SIZE];
   short * localMatrix;
   localMatrix = (short *) malloc(WINDOW_SIZE * sizeof(short));

   // moved opening graphics into process 0 to handle error where DISPLAY variable is not set
   if (id == 0) {
      MPE_Open_graphics( &graph, MPI_COMM_WORLD, 
                     getDisplay(),
                     -1, -1,
                     WINDOW_SIZE, WINDOW_SIZE, 0 );
   }

   firstTimeInLoop = 1;
   count = 1;

   if (numProcesses == 1) {
      startTime = MPI_Wtime();
      for (ix = 0; ix < WINDOW_SIZE; ix++)
      {
         // compute whole column with each process
         for (iy  = 0; iy < WINDOW_SIZE; iy++)
         {
            c_real = (ix - 400) * spacing - x_center;
            c_imag = (iy - 400) * spacing - y_center;
            x = y = 0.0;
            n = 0;

            while (n < 50 && distance(x, y) < 4.0)
            {
               compute(x, y, c_real, c_imag, &x, &y);
               n++;
            }

            if (n < 50) {
               //MPE_Draw_point(graph, ix, iy, MPE_RED);
               // store a 1 for RED
               // account for the offset in the x component of the matrix by subtracting the starting location
               totalMatrix[ix][iy] = 1;
            } else {
               //MPE_Draw_point(graph, ix, iy, MPE_BLACK);
               // store a 0 for BLACK
               // account for the offset in the x component of the matrix by subtracting the starting location
               totalMatrix[ix][iy] = 0;
            }
         }
      }

      // draw the picture with id = 0
      if (id == 0) {
         for (int totalX = 0; totalX < WINDOW_SIZE; totalX++) {
            for (int totalY = 0; totalY < WINDOW_SIZE; totalY++) {
               if (totalMatrix[totalX][totalY] == 1) {
                  // draw RED 
                  MPE_Draw_point(graph, totalX, totalY, MPE_RED);
               }
               else {
                  // draw BLACK
                  MPE_Draw_point(graph, totalX, totalY, MPE_BLACK);
               }
            }
         }
      }
      totalTime = MPI_Wtime() - startTime;

      // pause until mouse-click so the program doesn't terminate
      if (id == 0) {
         printf("\nTotal time: %lf\n\n", totalTime);
         printf("\nClick in the window to continue...\n");
         MPE_Get_mouse_press( graph, &ix, &iy, &button );

         MPE_Close_graphics( &graph );
      }

      // de-allocate localMatrix
      free(localMatrix);
      MPI_Finalize();
      return 0;
   }
   startTime = MPI_Wtime();
   while (1) {
      
      if (firstTimeInLoop) {
         // compute the first row for both the master and the workers
         ix = id;
         for (iy  = 0; iy < WINDOW_SIZE; iy++) {
            c_real = (ix - 400) * spacing - x_center;
            c_imag = (iy - 400) * spacing - y_center;
            x = y = 0.0;
            n = 0;

            while (n < 50 && distance(x, y) < 4.0)
            {
               compute(x, y, c_real, c_imag, &x, &y);
               n++;
            }

            if (n < 50) {
               //MPE_Draw_point(graph, ix, iy, MPE_RED);
               // store a 1 for RED
               // account for the offset in the x component of the matrix by subtracting the starting location
               localMatrix[iy] = 1;
            } else {
               //MPE_Draw_point(graph, ix, iy, MPE_BLACK);
               // store a 0 for BLACK
               // account for the offset in the x component of the matrix by subtracting the starting location
               localMatrix[iy] = 0;
            }
         }
         // master process
         if (id == 0) {
            // receive a process's computation and let it know which row to compute next
            MPI_Recv(localMatrix, WINDOW_SIZE, MPI_SHORT, MPI_ANY_SOURCE, count, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            numRowsReceived++;
            rowToCompute = count + numProcesses - 1;
            MPI_Send(&rowToCompute, 1, MPI_INT, count, 0, MPI_COMM_WORLD);
            numRowsSent++;

            x_coord_to_draw = 0;
            // DRAW THE ROW THAT PROCESSES ZERO COMPUTED
            for (int y = 0; y < WINDOW_SIZE; y++) {
               if (localMatrix[y] == 1) {
                  // draw RED
                  MPE_Draw_point(graph, x_coord_to_draw, y, MPE_RED);
               }
               else {
                  // draw BLACK
                  MPE_Draw_point(graph, x_coord_to_draw, y, MPE_BLACK);
               }
            }
            firstTimeInLoop = 0;
            rowsDrawn++;
         }
         // workers
         else {
            // send the computed row to master process
            MPI_Send(localMatrix, WINDOW_SIZE, MPI_SHORT, 0, id, MPI_COMM_WORLD);
            // receive instrcution for next row computation
            MPI_Recv(&rowToCompute, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            firstTimeInLoop = 0;
         }
      }
      // otherwise run this for the rest of the rows
      else {
         // master process

         if (id == 0) {

            // draw the previous row received
            x_coord_to_draw = count - 1;
            // draw the received row
            for (int y = 0; y < WINDOW_SIZE; y++) {
               if (localMatrix[y] == 1) {
                  // draw RED
                  MPE_Draw_point(graph, x_coord_to_draw, y, MPE_RED);
               }
               else {
                  // draw BLACK
                  MPE_Draw_point(graph, x_coord_to_draw, y, MPE_BLACK);
               }
            }
            rowsDrawn++;
            // receive a row from any source
            if (numRowsReceived < WINDOW_SIZE - 1) {
               MPI_Recv(localMatrix, WINDOW_SIZE, MPI_SHORT, MPI_ANY_SOURCE, count, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
               numRowsReceived++;
            }
            // if there's still rows to compute
            if (numRowsSent < WINDOW_SIZE - numProcesses) {
               rowToCompute = count + numProcesses - 1;
               // tell a process which row to compute next
               MPI_Send(&rowToCompute, 1, MPI_INT, (count % (numProcesses - 1)) + 1, 0, MPI_COMM_WORLD);
               numRowsSent++;
            }
            // otherwise, tell the workers that nothing else has to be computed
            else {
               rowToCompute = -1;
               for (int i = 1; i < numProcesses; i++) {
                  MPI_Send(&rowToCompute, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
               }
               terminateSent = 1;
            }

            // check that all the rows are drawn, all rows are received, and all workers have terminated
            if (rowsDrawn >= WINDOW_SIZE && numRowsReceived >= WINDOW_SIZE - 2 && terminateSent) {
               // finish up loop
               break;
            }
         }
         // worker processes
         else {
            // send row to the master process
            MPI_Send(localMatrix, WINDOW_SIZE, MPI_SHORT, 0, rowToCompute, MPI_COMM_WORLD);
            // recieve the next row that needs to be computed 
            MPI_Recv(&rowToCompute, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // if the master processes says that there's nothing else to compute, break out of the loop
            ix = rowToCompute;
            if (rowToCompute == -1) {
               break;
            }
            // compute the row that the master process assigned
            for (iy  = 0; iy < WINDOW_SIZE; iy++) {
               c_real = (ix - 400) * spacing - x_center;
               c_imag = (iy - 400) * spacing - y_center;
               x = y = 0.0;
               n = 0;

               while (n < 50 && distance(x, y) < 4.0) {
                  compute(x, y, c_real, c_imag, &x, &y);
                  n++;
               }

               if (n < 50) {
                  //MPE_Draw_point(graph, ix, iy, MPE_RED);
                  // store a 1 for RED
                  // account for the offset in the x component of the matrix by subtracting the starting location
                  localMatrix[iy] = 1;
               } else {
                  //MPE_Draw_point(graph, ix, iy, MPE_BLACK);
                  // store a 0 for BLACK
                  // account for the offset in the x component of the matrix by subtracting the starting location
                  localMatrix[iy] = 0;
               }
            }
         }
      }
      count++;
   }
   totalTime = MPI_Wtime() - startTime;

   // pause until mouse-click so the program doesn't terminate
   if (id == 0) {
      printf("\nTotal time: %lf\n\n", totalTime);
      printf("\nClick in the window to continue...\n");
      MPE_Get_mouse_press( graph, &ix, &iy, &button );

      MPE_Close_graphics( &graph );
   }

   // de-allocate localMatrix
   free(localMatrix);
   MPI_Finalize();
   return 0;
}

