/**
 * Copyright 1993-2013 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 * Extended for use in CS 374 at Calvin College by Joel C. Adams.
 * Extended by Josh Bussis for CS 374 at Calvin University on 12/6/2019 to compute the length of the hypotenuse of 
 * two orthogonal vectors
 */

/**
 * Vector hypotenuse: C = sqrt(A^2 + B^2)
 *
 * This sample is a very basic sample that implements finding the hypotenuse of two orthogonal vectors.
 */

#include <stdio.h>
#include <omp.h>
// For the CUDA runtime routines (prefixed with "cuda_")
#include <cuda_runtime.h>

/**
 * CUDA Kernel Device code
 *
 * Computes the hypotenuse of A and B
 * The 3 vectors have the same number of elements numElements.
 */
__global__
void vectorHypot(const float *A, const float *B, float *C, unsigned long numElements)
{
    int i = blockDim.x * blockIdx.x + threadIdx.x;

    if (i < numElements)
    {
        C[i] = sqrt( (A[i]*A[i]) + (B[i]*B[i]) );
    }
}

void checkErr(cudaError_t err, const char* msg) 
{
    if (err != cudaSuccess)
    {
        fprintf(stderr, "%s (error code %d: '%s')!\n", msg, err, cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
}

/**
 * Host main routine
 */
int main(int argc, char** argv)
{
    // timing variables
    double startCuda, stopCuda;
    double startSeq, stopSeq;
    double startHostDev, stopHostDev;
    double startComp, stopComp;
    double startDevHost, stopDevHost;

    // Error code to check return values for CUDA calls
    cudaError_t err = cudaSuccess;

    // Print the vector length to be used, and compute its size
    unsigned long numElements = 50000;
    if (argc == 2) {
      numElements = strtoul( argv[1] , 0, 10 );
    }
    size_t size = numElements * sizeof(float);
    printf("[Hypotenuse of %lu elements]\n", numElements);

    // Allocate the host input vectors A & B
    float * h_A = (float *)malloc(size);
    float * h_B = (float *)malloc(size);

    // Allocate the host output vector C
    float * h_C = (float *)malloc(size);

    // Verify that allocations succeeded
    if (h_A == NULL || h_B == NULL || h_C == NULL)
    {
        fprintf(stderr, "Failed to allocate host vectors!\n");
        exit(EXIT_FAILURE);
    }

    // Initialize the host input vectors
    for (int i = 0; i < numElements; ++i)
    {
        h_A[i] = rand()/(float)RAND_MAX;
        h_B[i] = rand()/(float)RAND_MAX;
    }

    // 1a. Allocate the device input vectors A & B
    float * d_A = NULL;
    err = cudaMalloc((void **)&d_A, size);
    checkErr(err, "Failed to allocate device vector A");
    float * d_B = NULL;
    err = cudaMalloc((void **)&d_B, size);
    checkErr(err, "Failed to allocate device vector B");

    // 1.b. Allocate the device output vector C
    float * d_C = NULL;
    err = cudaMalloc((void **)&d_C, size);
    checkErr(err, "Failed to allocate device vector C");

    // 2. Copy the host input vectors A and B in host memory 
    //     to the device input vectors in device memory
    startCuda = omp_get_wtime();
    startHostDev = omp_get_wtime();
    //printf("Copy input data from the host memory to the CUDA device\n");
    err = cudaMemcpy(d_A, h_A, size, cudaMemcpyHostToDevice);
    checkErr(err, "Failed to copy device vector A from host to device");


    err = cudaMemcpy(d_B, h_B, size, cudaMemcpyHostToDevice);
    checkErr(err, "Failed to copy device vector B from host to device");
    stopHostDev = omp_get_wtime();

    startComp = omp_get_wtime();
    // 3. Launch the Vector Add CUDA Kernel
    int threadsPerBlock = 256;
    int blocksPerGrid =(numElements + threadsPerBlock - 1) / threadsPerBlock;
    //printf("CUDA kernel launch with %d blocks of %d threads\n", blocksPerGrid, threadsPerBlock);
    vectorHypot<<<blocksPerGrid, threadsPerBlock>>>(d_A, d_B, d_C, numElements);
    err = cudaGetLastError();
    checkErr(err, "Failed to launch vectorHypot kernel");
    stopComp = omp_get_wtime();

    startDevHost = omp_get_wtime();
    // 4. Copy the device result vector in device memory
    //     to the host result vector in host memory.
    //printf("Copy output data from the CUDA device to the host memory\n");
    err = cudaMemcpy(h_C, d_C, size, cudaMemcpyDeviceToHost);
    checkErr(err, "Failed to copy vector C from device to host");
    stopDevHost = omp_get_wtime();
    stopCuda = omp_get_wtime();
    // Verify that the result vector is correct
    for (int i = 0; i < numElements; ++i)
    {
        if (fabs(sqrt( (h_A[i]*h_A[i]) + (h_B[i]*h_B[i]) ) - h_C[i]) > 1e-5)
        {
            fprintf(stderr, "Result verification failed at element %d!\n", i);
            exit(EXIT_FAILURE);
        }
    }
    printf("CUDA test PASSED\n");
    printf("CUDA host-to-dev time: %lf\nCUDA comp time: %lf\nCUDA dev-to-host time: %lf\n", stopHostDev - startHostDev, 
                                                                                            stopComp - startComp,
                                                                                            stopDevHost - startDevHost);
    printf("CUDA total time: %lf\n", stopCuda-startCuda); 

    // Free device global memory
    err = cudaFree(d_A);
    checkErr(err, "Failed to free device vector A");

    err = cudaFree(d_B);
    checkErr(err, "Failed to free device vector B");

    err = cudaFree(d_C);
    checkErr(err, "Failed to free device vector C");

    startSeq = omp_get_wtime();
    // repeat the computation sequentially
    for (int i = 0; i < numElements; ++i)
    {
       h_C[i] = sqrt( (h_A[i]*h_A[i]) + (h_B[i]*h_B[i]) );
    }
    stopSeq = omp_get_wtime();

    // verify again
    for (int i = 0; i < numElements; ++i)
    {
        if (fabs(sqrt( (h_A[i]*h_A[i]) + (h_B[i]*h_B[i]) ) - h_C[i]) > 1e-5)
        {
            fprintf(stderr, "Result verification failed at element %d!\n", i);
            exit(EXIT_FAILURE);
        }
    }
    printf("\nNormal test PASSED\n");
    printf("Normal time: %lf\n", stopSeq-startSeq); 
    
    // Free host memory
    free(h_A);
    free(h_B);
    free(h_C);

    // Reset the device and exit
    err = cudaDeviceReset();
    checkErr(err, "Unable to reset device");

    printf("Done\n");
    return 0;
}

