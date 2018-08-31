#!/bin/bash
 
# Projekt: Nasobeni matic (Mesh multiplication)
# Autor: Petr Polansky
# Login: xpolan07
 
mat1=$(head -c 1 mat1)
mat2=$(head -c 1 mat2)

# echo $mat2
cpus=$((mat1 * mat2))

((cpus++))


mpic++ --prefix /usr/local/share/OpenMPI -o mm mm.cpp -std=c++0x
mpirun --prefix /usr/local/share/OpenMPI -np $cpus mm
rm -f mm
