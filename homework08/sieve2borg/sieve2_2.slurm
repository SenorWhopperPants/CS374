#!/bin/bash
# Example with 1 nodes, 1 processes each = 1 processes
#
# Set the number of nodes to use (max 20)
#SBATCH -N 1
#SBATCH -n 1
#SBATCH -c 2

# Load the compiler and MPI library
module load openmpi-2.0/gcc

# Run the program
mpirun ./sieve2 1000000000 2
