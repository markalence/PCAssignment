#pragma clang diagnostic push
#pragma ide diagnostic ignored "openmp-use-default-none"

#include <stdio.h>
#include <math.h>
#include <omp.h>
#include <time.h>

#define START_POINT 1.5
#define END_POINT 20
#define N_THREADS 4
#define PAD 8
#define STEP 500000
#define N_PARTITIONS 20000000
#define N_REPEATS 10

void writeToFile(double serial[],double parallel[]){

    FILE* file = fopen("serial_times_4_threads.csv","w");
    for(int i = 0; i < N_PARTITIONS/STEP; ++i){
        fprintf(file,"%d,%f\n",(i+1)*STEP,serial[i]);
    }
    fclose(file);

    file = fopen("parallel_times_4_threads.csv","w");
    for(int i = 0; i < N_PARTITIONS/STEP; ++i){
        fprintf(file,"%d,%f\n",(i+1)*STEP,parallel[i]);
    }

    fclose(file);


}

double doParallel(int nPartitions) {
    double  time = omp_get_wtime();
    omp_set_num_threads(N_THREADS);
    double area[N_THREADS][PAD];
    double k = (END_POINT - START_POINT) / (double) nPartitions;
#pragma omp parallel for
    for (int i = 0; i < (int) nPartitions; ++i) {
        double x = START_POINT + k * i;
        area[omp_get_thread_num()][0] += fabs(k * x * exp(-x));
    }
    double sum = 0;
    for (int i = 0; i < N_THREADS; ++i) {
        sum += area[i][0];
        area[i][0] = 0;
    }

    return omp_get_wtime()-time;
}

double doSerial(int nPartitions) {
    double time = clock();
    double area = 0;
    double k = (END_POINT - START_POINT) / (double) nPartitions;
    for (int i = 0; i < (int) nPartitions; ++i) {
        double x = START_POINT + k * i;
        area += fabs(k * x *exp(-x));
    }

    return (clock()-time)/CLOCKS_PER_SEC;

}

double getVar(double arr[],double avg){
    double var = 0;
    for(int i = 0; i < 10; ++i){
        var += (arr[i]-avg)*(arr[i]-avg);
    }
    return var;
}

int main() {
    double avg = 0;
    double pavgs[N_PARTITIONS/STEP];
    int ptimes[N_REPEATS];
    for (int i = STEP; i <= N_PARTITIONS; i += STEP) {
        avg = 0;
        for (int j = 0; j < N_REPEATS; ++j) {
            avg += doParallel(i);
//            ptimes[j] = avg;
        }
        pavgs[(i/STEP)-1] = avg/N_REPEATS;
//        printf("%d,%f,%f\n",i,avg/10,getVar(ptimes,avg/10));
    }

    printf("\n\n\n");

    int stimes[N_REPEATS];
    double savgs[N_PARTITIONS/STEP];
    for (int i = STEP; i <= STEP; i += STEP) {
        avg = 0;
        for (int j = 0; j < N_REPEATS; ++j) {
            avg += doSerial(i);
//            stimes[j] = avg;
        }
        savgs[(i/STEP)-1] = avg/N_REPEATS;
//        printf("%d,%f,%f\n",i,avg/10,getVar(stimes,avg/10));
    }

    writeToFile(savgs,pavgs);



    return 0;
}

#pragma clang diagnostic pop