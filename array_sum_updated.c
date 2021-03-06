#include<stdio.h>
#include<stdlib.h>
#include <mpi.h>

#define size 50

int data[size];
int update_sum(int offset, int chunk, int id);

int main(int argc, char *argv[]){
    int np, id, i, j, chunk, offset, sum, total_sum = 0;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &np);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    chunk = size / np;
    if(id == 0) {
        sum = 0;
        for(i = 0; i < size; i++) {
            data[i] = 1;
            sum = sum + data[i];
        }
        printf("\nInitial Sum = %d", sum);
        sum = 0;
        offset = chunk;
        for(i = 1; i < np; i++) {
            MPI_Send(&offset, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
            MPI_Send(&data[offset], chunk, MPI_INT, i, 1, MPI_COMM_WORLD);
            offset = offset + chunk;
        }
        sum = update_sum(0, chunk, 0);
        MPI_Reduce(&sum, &total_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        printf("\nFinal Sum = %d", total_sum);
    }
    else {
        MPI_Recv(&offset, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
        MPI_Recv(&data[offset], chunk, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
        sum = update_sum(offset, chunk, id);
        MPI_Reduce(&sum, &total_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    }
    MPI_Finalize();
    return 0;
}

int update_sum(int offset, int chunk, int id) {
    int i, sum = 0;
    for(i = offset; i < offset + chunk; i++) {
        sum = sum + data[i];
    }
    printf("\nProcess - %d\tOffset - %d to %d\tSum - %d", id, offset, offset + chunk, sum);
    return(sum);
}
