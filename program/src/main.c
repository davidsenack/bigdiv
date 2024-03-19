#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <time.h>
#include <mpi.h>

#define DIGITS 100000000

typedef struct {
    mpz_t dividend;
    mpz_t max_divisor;
    mpz_t start;
    mpz_t end;
} ThreadArgs;

void* checkDivisibility(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;

    mpz_t counter, divisor, remainder;
    mpz_init_set(counter, args->start);
    mpz_init(divisor);
    mpz_init(remainder);

    while (mpz_cmp(counter, args->end) <= 0 && mpz_cmp(counter, args->dividend) < 0) {
        mpz_add_ui(divisor, counter, 1);

        mpz_mod(remainder, args->dividend, divisor);

        if (mpz_cmp_ui(remainder, 0) == 0) {
            mpz_set(args->max_divisor, divisor);
        }

        mpz_add_ui(counter, counter, 1);
    }

    mpz_clear(counter);
    mpz_clear(divisor);
    mpz_clear(remainder);

    return NULL;
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int world_size, world_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if (argc != 2) {
        if (world_rank == 0) {
            printf("Usage: %s <num_threads>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    int num_threads = atoi(argv[1]);

    // Initialize variables
    mpz_t dividend;
    mpz_init(dividend);

    // Initialize GMP integers
    mpz_t max_divisor;
    mpz_init(max_divisor);

    // Set random seed (optional)
    gmp_randstate_t state;
    gmp_randinit_default(state);
    gmp_randseed_ui(state, time(NULL));

    // Generate random 100 million digit integer for dividend
    if (world_rank == 0) {
        mpz_urandomb(dividend, state, DIGITS);
    }

    // Broadcast dividend to all processes
    MPI_Bcast(mpz_limbs_read(dividend), DIGITS, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD);

    // Initialize threads and thread arguments
    ThreadArgs thread_args[num_threads];
    pthread_t threads[num_threads];

    mpz_t start, end;
    mpz_init(start);
    mpz_init(end);
    mpz_t step;
    mpz_init(step);
    mpz_fdiv_q_ui(step, dividend, num_threads);
    mpz_set(start, step);
    mpz_set_ui(end, 0);

    #pragma omp parallel for num_threads(num_threads)
    for (int i = 0; i < num_threads; i++) {
        mpz_set_ui(thread_args[i].start, 0);
        mpz_set_ui(thread_args[i].end, 0);
        mpz_set_ui(thread_args[i].max_divisor, 0);
        mpz_set(thread_args[i].dividend, dividend);

        mpz_addmul_ui(end, step, 1);

        if (i == num_threads - 1) {
            mpz_add_ui(end, end, 1);
        }

        mpz_set(thread_args[i].start, start);
        mpz_set(thread_args[i].end, end);

        pthread_create(&threads[i], NULL, checkDivisibility, (void*)&thread_args[i]);

        mpz_set(start, end);
    }

    // Join threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Find maximum divisor among all threads
    mpz_set_ui(max_divisor, 0);
    for (int i = 0; i < num_threads; i++) {
        if (mpz_cmp(thread_args[i].max_divisor, max_divisor) > 0) {
            mpz_set(max_divisor, thread_args[i].max_divisor);
        }
    }

    // Gather all maximum divisors to process 0
    mpz_t all_max_divisors[world_size];
    MPI_Gather(mpz_limbs_read(max_divisor), DIGITS, MPI_UNSIGNED_LONG, all_max_divisors, DIGITS, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD);

    if (world_rank == 0) {
        // Find the overall maximum divisor
        mpz_set_ui(max_divisor, 0);
        for (int i = 0; i < world_size; i++) {
            if (mpz_cmp(all_max_divisors[i], max_divisor) > 0) {
                mpz_set(max_divisor, all_max_divisors[i]);
            }
        }

        // Output result
        gmp_printf("The largest divisor less than the dividend is: %Zd\n", max_divisor);
    }

    // Clear allocated memory
    mpz_clear(dividend);
    mpz_clear(max_divisor);
    mpz_clear(start);
    mpz_clear(end);
    mpz_clear(step);

    MPI_Finalize();

    return 0;
}

