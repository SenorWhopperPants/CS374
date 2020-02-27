/**
 * pthreadReduction.h contains the function pthreadReductionSum() which 
 *      implements a sum reduction for use with POSIX threads.
 * 
 * @author  Josh Bussis
 * @date    11/2/2019
 * @class   CS 374, Project 06
 * @Where   Calvin University
 */
#include <stdlib.h>
#include "pthreadBarrier.h"
#include <stdio.h>

/**
 * @brief   pthreadReductionSum() is a function to perform a sum reduction on an array with multithreading.
 *                                This function performs the sum in O(ln(N)) time
 * 
 * @author  Josh Bussis
 * @date    11/2/2019
 * @param   totalSum (volatile long double *)
 * @param   localSumArray (long double *)
 * @param   threadID (unsigned long)
 * @param   numThreads (unsigned long)
 * @return  void
*/
void pthreadReductionSum(volatile long double * totalSum, long double * localSumArray, unsigned long threadID, unsigned long numThreads) {
    
    // synchronize all of the threads
    pthreadBarrier(numThreads);

    // go through the array and sum the results via the tree method
    // do the reduction as long as there's values to add
    // the variable arrayOffset holds the offset value for the respective index to add itself and itself+arrayOffset's value
    // arrayOffset>>=1 bit-shifts arrayOffset 1 to the right, which divides by two, but faster than divide function
    // slide deck from nvidia helped with the syntax slightly, but I would have figured it out on my own anyway
    for (unsigned long arrayOffset = (unsigned long)(numThreads / 2); arrayOffset > 0; arrayOffset>>=1) {
        // let only the desired threads to do the work
        if (threadID < arrayOffset) {
            localSumArray[threadID] += localSumArray[(int)(threadID + arrayOffset)];
        }
        // synchronize the threads before moving on
        pthreadBarrier(numThreads);
    }
    // clean up the barrier
    barrierCleanup();
    // store the first item in the array since this will be the sum of the whole array
    // only let thread 0 do this so there's no issues
    if (threadID == 0) {
        *totalSum = localSumArray[0];
    }
}
