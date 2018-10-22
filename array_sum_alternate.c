#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

float *create_rand_nums(int num_elements) {
    float *rand_nums = (float *)malloc(sizeof(float) * num_elements);
    int i;
    for (i = 0; i < num_elements; i++) {
        rand_nums[i] = (rand() / (float)RAND_MAX);
    }
    return rand_nums;
}

float compute_sum(float *array, int num_elements) {
    float sum = 0.f;
    int i;
    for (i = 0; i < num_elements; i++) {
        sum += array[i];
    }
    return sum;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: avg num_elements_per_proc\n");
        exit(0);
    }
    int num_elements_per_proc = atoi(argv[1]);
    srand(time(NULL));
    MPI_Init(NULL, NULL);
    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    float *rand_nums = NULL;
    if (world_rank == 0) {
        rand_nums = create_rand_nums(num_elements_per_proc * world_size);
    }
    float *sub_rand_nums = (float *)malloc(sizeof(float) * num_elements_per_proc);
    MPI_Scatter(rand_nums, num_elements_per_proc, MPI_FLOAT, sub_rand_nums, num_elements_per_proc, MPI_FLOAT, 0, MPI_COMM_WORLD);
    float sub_sum = compute_sum(sub_rand_nums, num_elements_per_proc);
    float *sub_sums = NULL;
    if (world_rank == 0) {
        sub_sums = (float *)malloc(sizeof(float) * world_size);
    }
    MPI_Gather(&sub_sum, 1, MPI_FLOAT, sub_sums, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
    if (world_rank == 0) {
        float sum = compute_sum(sub_sums, world_size);
        printf("Sum of all elements is %f\n", sum);
    }
    if (world_rank == 0) {
        free(rand_nums);
        free(sub_sums);
    }
    free(sub_rand_nums);
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return 0;
}
