#!/bin/bash
# Example with 2 nodes, 1 processes each = 2 processes and 100000 traps
#
# Set the number of nodes to use (max 20)
#SBATCH -N 2
#
# Set the number of processes per node (max 16)
#SBATCH --ntasks-per-node=1
#

# Load the compiler and MPI library
module load openmpi-2.0/gcc

# Run the program
mpirun ../calcPI 100000