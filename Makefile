# params
COMP = g++
OMP = -fopenmp
MATHEVAL = -lmatheval
WALL = -Wall


integral_omp: integral_omp.cpp
	$(COMP) integral_omp.cpp $(WALL) $(OMP) $(MATHEVAL) -o integral_omp
