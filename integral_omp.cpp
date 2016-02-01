#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "matheval.h"
#include <omp.h>

bool validateArgs(int argc, char *argv[]);
inline double calculateValueOfFunction(void *f, char ** varname, double x1);

/**
 * a - beginning
 * b - end
 * n - number of trapezoids
 * m - number of threads
 */
int main(int argc, char *argv[])
{
    char **names;
    void * fun;
    int functionParamCount, n, m, i;
    double begin_time, end_time, time_spent;
    double sumOfPeaks, a, b, h;

    // validation
    if (!validateArgs(argc, argv))
    {
        return -1;
    }

    //init parameters and evaluators
    m = atoi(argv[1]);
    a = atof(argv[2]);
    b = atof(argv[3]);
    n = atoi(argv[4]);
    char *function = argv[5];

    void **function_evaluators = (void **) malloc(sizeof(void *) * m);
    for (i = 0; i < m; ++i) {
        function_evaluators[i] = evaluator_create (function);
    }

    assert (function_evaluators[0]);
    evaluator_get_variables (function_evaluators[0], &names, &functionParamCount);
    if (functionParamCount != 1) {
        printf("Incorrect number of parameters in function");
        return -1;
    }

    h = (b - a) / n;
    sumOfPeaks = 0;

    // parallel counting values of function.
    begin_time = omp_get_wtime();
    #pragma omp parallel num_threads(m) default(none) shared(function_evaluators, a, h, n) private(i, fun, names) \
    reduction(+: sumOfPeaks)
    {
        int thread_num= omp_get_thread_num();
        fun = function_evaluators[thread_num];
        int functionParamCount = 0;
        evaluator_get_variables (fun, &names, &functionParamCount);

        #pragma omp for nowait
        for (i = 1; i < n; i++) {
            double x = a + i * h;
            sumOfPeaks += calculateValueOfFunction(fun, names, x);
        }
    }
    end_time = omp_get_wtime();

    double beginValue = calculateValueOfFunction(function_evaluators[0], names, a);
    double endValue = calculateValueOfFunction(function_evaluators[0], names, b);
    sumOfPeaks +=  beginValue/2 + endValue/2;
    double area = h * sumOfPeaks;


    time_spent = (end_time - begin_time) * 1000;

    printf("Time: %gms\n", time_spent);
    printf("Integral value: %g\n", area);


    // destroy evaluators and free memory
    for (i = 0; i < m; ++i) {
        evaluator_destroy(function_evaluators[i]);
    }
    free (function_evaluators);

    return 0;
}

bool validateArgs(int argc, char *argv[]) {
    if (argc != 6) {
        fprintf(stderr, "Invalid number of parameters.\n");
        return false;
    }

    double a = atof(argv[2]);
    double b = atof(argv[3]);

    if(a > b) {
        fprintf(stderr, "End value is NOT greater then start value.\n");
        return false;
    }

    int numOfThreads = atoi(argv[1]);
    if (numOfThreads < 1) {
        fprintf(stderr, "Program should be run on one or more threads\n");
        return false;
    }

    return true;
}

inline double calculateValueOfFunction(void *f, char ** varname, double x1) {
    double x1Array [] = {x1};
    return evaluator_evaluate(f, 1, varname, x1Array);
}