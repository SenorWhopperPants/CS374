#!/bin/bash
# Example with 8 nodes, 8 processes each = 64 processes
#
# Set the number of nodes to use (max 20)
#SBATCH -N 8
#
# Set the number of processes per node (max 16)
#SBATCH --ntasks-per-node=8
#

# Load the compiler and MPI library
module load openmpi-2.0/gcc

# Run the program
mpirun ./sieve 1000000000
