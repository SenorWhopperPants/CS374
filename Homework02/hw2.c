/* hw2.c
 * @brief   This is the homework02 file that sends a message around a loop of processes
 * 
 * @author  Josh Bussis
 * @date    09/20/2019
 * @class   CS 374
 */

#include <stdio.h>
#include <mpi.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    int id = -1, numProcesses = -1; 
    char sendMessage[3];
    double startTime = 0.0, totalTime = 0.0;
    int arraySize = 1000 * sizeof(char);
    MPI_Status status;

    // start MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);
    // make the message to be passed around
    char* receivedMessage = NULL;
    receivedMessage = (char*)malloc(arraySize);
    // make the temp buffer for each individual process to add to total message
    char* buffer = NULL;
    // set array size to number of digits of the id and add 2 (for the space and \0)
    buffer = (char*)malloc((id/10)+2);

    // this is the master process
    if (id == 0) {
        sprintf(sendMessage, "%d ", id);
        // start timer
        startTime = MPI_Wtime();
        // send the first message containing the id: 0
        MPI_Send(sendMessage, 3, MPI_CHAR, id+1, 0, MPI_COMM_WORLD);
        // receive the total message
        MPI_Recv(receivedMessage, arraySize, MPI_CHAR, numProcesses-1, MPI_ANY_TAG, 
                       MPI_COMM_WORLD, &status);
        // calculate total time the loop took
        totalTime = MPI_Wtime() - startTime;
        // print results
        printf("%s\n", receivedMessage);
        printf("Time: %f\n", totalTime);
    }
    // these are the worker processes
    else {
        MPI_Recv(receivedMessage, arraySize, MPI_CHAR, id-1, MPI_ANY_TAG, 
                       MPI_COMM_WORLD, &status);
        // make a new message with current id
        sprintf(buffer, "%d ", id);
        // add that to the total message
        strcat(receivedMessage, buffer);
        // send out the new message down the line
        MPI_Send(receivedMessage, arraySize, MPI_CHAR, (id+1)%numProcesses, 2,
                        MPI_COMM_WORLD);
    }
    // finalize MPI
    MPI_Finalize();
    return 0;
}