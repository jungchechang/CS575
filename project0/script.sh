#!/bin/bash
for t in 1 4
do
    g++   prog.cpp  -DNUMT=$t -o prog  -lm  -fopenmp
  ./prog
done
