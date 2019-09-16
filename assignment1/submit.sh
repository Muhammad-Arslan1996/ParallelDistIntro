#!/bin/bash
#
#SBATCH --cpus-per-task=4
#SBATCH --time=2:00
#SBATCH --mem=1G

srun python /scratch/assignment1/test_scripts/pi_calculation_tester.pyc --execPath=/home/marslan/sfuhome/CMPT431/assignments/assignment1/pi_calculation_parallel