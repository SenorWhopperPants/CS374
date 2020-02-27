#!/bin/bash
# Example with 4 nodes, 2 processes each = 8 processes and 100000 traps
#
# Set the number of nodes to use (max 20)
#SBATCH -N 4
#
# Set the number of processes per node (max 16)
#SBATCH --ntasks-per-node=2
#

# Load the compiler and MPI library
module load openmpi-2.0/gcc

# Run the program
mpirun ../calcPI 10000000