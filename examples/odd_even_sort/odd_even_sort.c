void Odd_even_iter(int local_A[], int temp_B[], int temp_C[],
        int local_n, int phase, int even_partner, int odd_partner,
        int my_rank, int p, MPI_Comm comm) {
   MPI_Status status;

   if (phase % 2 == 0) { /* even phase */
      if (even_partner >= 0) { /* check for even partner */
         MPI_Sendrecv(local_A, local_n, MPI_INT, even_partner, 0,
            temp_B, local_n, MPI_INT, even_partner, 0, comm,
            &status);
         if (my_rank % 2 != 0) /* odd rank */
            // local_A have largest local_n ints from local_A and even_partner
            Merge_high(local_A, temp_B, temp_C, local_n);
         else /* even rank */
            // local_A have smallest local_n ints from local_A and even_partner
            Merge_low(local_A, temp_B, temp_C, local_n);
      }
   } else { /* odd phase */
      if (odd_partner >= 0) {  /* check for odd partner */
         MPI_Sendrecv(local_A, local_n, MPI_INT, odd_partner, 0,
            temp_B, local_n, MPI_INT, odd_partner, 0, comm,
            &status);
         if (my_rank % 2 != 0) /* odd rank */
            Merge_low(local_A, temp_B, temp_C, local_n);
         else /* even rank */
            Merge_high(local_A, temp_B, temp_C, local_n);
      }
   }
}  /* Odd_even_iter */





void Sort(int local_A[], int local_n, int my_rank,
         int p, MPI_Comm comm) {
   int phase;
   int *temp_B, *temp_C;
   int even_partner;  /* phase is even or left-looking */
   int odd_partner;   /* phase is odd or right-looking */

   /* Temporary storage used in merge-split */
   temp_B = (int*) malloc(local_n*sizeof(int));
   temp_C = (int*) malloc(local_n*sizeof(int));

   /* Find partners:  negative rank => do nothing during phase */
   if (my_rank % 2 != 0) {   /* odd rank */
      even_partner = my_rank - 1;
      odd_partner = my_rank + 1;
      if (odd_partner == p) odd_partner = MPI_PROC_NULL;  // Idle during odd phase
   } else {                   /* even rank */
      even_partner = my_rank + 1;
      if (even_partner == p) even_partner = MPI_PROC_NULL;  // Idle during even phase
      odd_partner = my_rank-1;
   }

   /* Sort local list using built-in quick sort */
   qsort(local_A, local_n, sizeof(int), Compare);

#  ifdef DEBUG
   printf("Proc %d > before loop in sort\n", my_rank);
   fflush(stdout);
#  endif

   for (phase = 0; phase < p; phase++)
      Odd_even_iter(local_A, temp_B, temp_C, local_n, phase,
             even_partner, odd_partner, my_rank, p, comm);

   // deallocate memory
   free(temp_B);
   free(temp_C);
}  /* Sort */



int main(int argc, char* argv[]) {
   int my_rank, p;   // rank, number processes
   char g_i;         // holds either g or i depending on user input
   int *local_A;     // local list: size of local number of elements * size of int
   int global_n;     // number of elements in global list
   int local_n;      // number of elements in local list (process list)
   MPI_Comm comm;
   double start, finish, loc_elapsed, elapsed;

   MPI_Init(&argc, &argv);
   comm = MPI_COMM_WORLD;
   MPI_Comm_size(comm, &p);
   MPI_Comm_rank(comm, &my_rank);

   Get_args(argc, argv, &global_n, &local_n, &g_i, my_rank, p, comm);
   local_A = (int*) malloc(local_n*sizeof(int));

   // generate random list based on user input
   if (g_i == 'g') {
      Generate_list(local_A, local_n, my_rank);
      Print_local_lists(local_A, local_n, my_rank, p, comm);
   }
   // read in user defined list from command line
   else {
      Read_list(local_A, local_n, my_rank, p, comm);
//#     ifdef DEBUG
      Print_local_lists(local_A, local_n, my_rank, p, comm);
//#     endif
   }

#  ifdef DEBUG
   printf("Proc %d > Before Sort\n", my_rank);
   fflush(stdout);
#  endif

   MPI_Barrier(comm);
   start = MPI_Wtime();
   Sort(local_A, local_n, my_rank, p, comm);
   finish = MPI_Wtime();
   loc_elapsed = finish-start;
   MPI_Reduce(&loc_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, comm);

#  ifdef DEBUG
   Print_local_lists(local_A, local_n, my_rank, p, comm);
   fflush(stdout);
#  endif

   Print_global_list(local_A, local_n, my_rank, p, comm);

   free(local_A);  // deallocate memory

   if (my_rank == 0) printf("Sorting took %f milliseconds \n", loc_elapsed*1000);

   MPI_Finalize();

   return 0;
}  /* main */
