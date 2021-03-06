/* slicesMandel.c
 * Compute/draw mandelbrot set using MPI/MPE commands
 *
 * Written Winter, 1998, W. David Laverell.
 *
 * Refactored Winter 2002, Joel Adams. 
 * 
 * Paralellised by   Josh Bussis -> Parallelization done through the slices paradigm
 * Date              10/09/2019
 * Where             Calvin University
 * Why               Homework 04     
 * Class             CS 374       
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
   int totalX, totalY;
   short totalMatrix[WINDOW_SIZE][WINDOW_SIZE];
   double startTime, totalTime = -1;
   int numProcesses = -1;
   int currentX;
   int x_coord_to_draw;

   // allocate arrays
   //totalMatrix = allocate_matrix(WINDOW_SIZE, WINDOW_SIZE);
   //localMatrix = allocate_matrix(WINDOW_SIZE);

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
   short localMatrix[WINDOW_SIZE / numProcesses][WINDOW_SIZE];

   // moved opening graphics into process 0 to handle error where DISPLAY variable is not set
   if (id == 0) {
      MPE_Open_graphics( &graph, MPI_COMM_WORLD, 
                     getDisplay(),
                     -1, -1,
                     WINDOW_SIZE, WINDOW_SIZE, 0 );
   }

   startTime = MPI_Wtime();
   for (ix = id; ix < WINDOW_SIZE; ix += numProcesses)
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
            localMatrix[((ix - id) / numProcesses)][iy] = 1;
         } else {
            //MPE_Draw_point(graph, ix, iy, MPE_BLACK);
            // store a 0 for BLACK
            // account for the offset in the x component of the matrix by subtracting the starting location
            localMatrix[((ix - id) / numProcesses)][iy] = 0;
         }
      }
   }
   // gather all of the local matrices to the total matrix
   MPI_Gather(localMatrix, WINDOW_SIZE * (WINDOW_SIZE / numProcesses), MPI_SHORT, totalMatrix, WINDOW_SIZE * (WINDOW_SIZE / numProcesses), MPI_SHORT, 0, MPI_COMM_WORLD);

   // draw the picture with id = 0
   if (id == 0) {
      currentX = 0;
      for (totalY = 0; totalY < WINDOW_SIZE; totalY++) {
         for (int i = 0; i < numProcesses; i++) {
            for (totalX = 0; totalX < (WINDOW_SIZE / numProcesses); totalX++) {
               // map the current reading to the correct coord.
               x_coord_to_draw = i + (totalX * numProcesses);
               if (totalMatrix[currentX][totalY] == 1) {
                  // draw RED 
                  MPE_Draw_point(graph, x_coord_to_draw, totalY, MPE_RED);
               } 
               else {
                  // draw BLACK 
                  MPE_Draw_point(graph, x_coord_to_draw, totalY, MPE_BLACK);
               }
               currentX++;
            }
         }
         currentX = 0;
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

   MPI_Finalize();
   return 0;
}

