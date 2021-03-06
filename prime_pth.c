// ============================================================================
// File: prime.c
// Description: A simple prime number generator
//
// Odev 2
//
// Amac:
//    The goal of this assignment is to parallelize the prime number
//    generator using OpenMP and Pthreads.
//    (This project is adopted from a course project in CMU)
// ============================================================================


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>


typedef struct {				//Struct seemed the only way to carry local a and b's
	int start;
	int end;
	}parallelthreads;

pthread_mutex_t* mutex_for_prime_count;	//global mutex_t

// ============================================================================
// Serial version of the prime number generator
// ============================================================================

void Primes(unsigned N);

// ============================================================================
// Parallel version of the prime number generator
// ============================================================================

void ParallelPrimes(unsigned N, unsigned P);

// ============================================================================
// Pthread function parametered by void pointer 
// ============================================================================

void* parallelThreadsFunc(void* my_rank);

// ============================================================================
// Timer: returns time in seconds
// ============================================================================

double gettime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

// ============================================================================
// Global variables
// ============================================================================

// Number of primes found
unsigned count;
// Last prime number found
unsigned lastPrime;
// Array of flags. flag[i] denotes if 2*i+3 is prime or not
char *flags;


// ============================================================================
// Usage function
// ============================================================================

void Usage(char *program) {
  printf("Usage: %s [options]\n", program);
  printf("-n <num>\tSize of input. Maximum number to test\n");
  printf("-p <num>\tNumber of processors to use\n");
  printf("-o <file>\tOutput file name\n");
  printf("-d\t\tDisplay output\n");
  printf("-h \t\tDisplay this help and exit\n");

  exit(0);
}


// ============================================================================
// Main function
// ============================================================================

int main(int argc, char **argv) {

  int optchar;
  unsigned N = 100, P = 1;
  char outputfile[100] = "";
  char displayoutput = 0;

  // Read the command line options and obtain the input size and number of
  // processors.
  while ((optchar = getopt(argc, argv, "n:p:o:dh")) != -1) {

    switch (optchar) {

      case 'n':
        N = atoi(optarg);
        break;

      case 'p':
        P = atoi(optarg);
        break;

      case 'o':
        strcpy(outputfile, optarg);
        break;

      case 'd':
        displayoutput = 1;
        break;

      case 'h':
        Usage(argv[0]);
        break;

       default:
        Usage(argv[0]);
    }
  }

  // Create the flag array
  flags = malloc(sizeof(char) * ((N - 1)/2));
  if (!flags) {
    printf("Not enough memory.\n");
    exit(1);
  }

  printf("Testing for primes till: %u\n", N);
  printf("Number of processors: %u\n", P);

  // Depending on the number of processors, call the appropriate prime generator
  // function.
  if (P == 1) {
	double t;
	t = gettime();
    Primes(N);
	t= gettime()-t;
	printf("Serial Time: %.8f\n", t);
  }
  else {
	double t;
	t = gettime();
    ParallelPrimes(N, P);
	t= gettime()-t;
	printf("Parallel Time: %.8f\n" , t);
  }

  if (N >= 2) {
    count ++;
  }

  printf("Number of primes found = %u\n", count);
  printf("Last prime found = %u\n", lastPrime);


  // If we need to display the output, then open the output file
  // Open the output file
  if (displayoutput) {
    FILE *output;
    unsigned i;

    if (strlen(outputfile) > 0) {
      output = fopen(outputfile, "w");
      if (output == NULL) {
        printf("Cannot open specified output file `%s'. Falling back to stdout\n",
            outputfile);
        output = stdout;
      }
    }
    else {
      output = stdout;
    }

    fprintf(output, "List of prime numbers:\n");
    fprintf(output, "2\n");
    for (i = 0; i < (N-1)/2; i ++) {
      if (flags[i])
        fprintf(output, "%u\n", i*2+3);
    }
  }

  free(flags);
  return 0;
}

// ============================================================================
// Implementation of the serial version of the prime number generator
// ============================================================================

void Primes(unsigned N) {
  int i;
  int prime;
  int div1, div2, rem;

    count = 0;
    lastPrime = 0;

    for (i = 0; i < (N-1)/2; ++i) {    /* For every odd number */
      prime = 2*i + 3;

      /* Keep searching for divisor until rem == 0 (i.e. non prime),
         or we've reached the sqrt of prime (when div1 > div2) */

      div1 = 1;
      do {
        div1 += 2;            /* Divide by 3, 5, 7, ... */
        div2 = prime / div1;  /* Find the dividend */
        rem = prime % div1;   /* Find remainder */
      } while (rem != 0 && div1 <= div2);

      if (rem != 0 || div1 == prime) {
        /* prime is really a prime */
        flags[i] = 1;
        count++;
        lastPrime = prime;
      } else {
        /* prime is not a prime */
        flags[i] = 0;
      }
    }
  
}


// ============================================================================
// Parallel version of the prime number generator. You must use openmp to
// parallelize this function.
// ============================================================================

void ParallelPrimes(unsigned N, unsigned P) {
  int i;
  int prime;
  int div1, div2, rem;
    count = 0;
    lastPrime = 0;
	
	pthread_t** arr = (pthread_t**)malloc(sizeof(pthread_t*) * P);
	int scheduler = ((N-1)/2)/P;
	for(i=0; i<P; i++)
	{
	arr[i] = (pthread_t*)malloc(sizeof(pthread_t));
	int start = i * scheduler;
	int end = (i+1) * scheduler;
		if( i == P-1)end = (N-1)/2;		
	parallelthreads *start_end_pointer = (parallelthreads*)malloc(sizeof(parallelthreads));
	start_end_pointer->start = start;
	start_end_pointer->end = end;
  mutex_for_prime_count = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutex_for_prime_count, NULL);
	pthread_create(arr[i], NULL, parallelThreadsFunc, start_end_pointer);
    	}   
	
	for(i=0; i<P; i++)
	{
	pthread_join(*arr[i], NULL);
	}
 } /* parallelprimes */

	void* parallelThreadsFunc(void* my_rank)
	{
		int start = ((parallelthreads*)my_rank)->start;
		int end = ((parallelthreads*)my_rank)->end;
		int i;
  		int prime;
 	        int div1, div2, rem;
		unsigned private_counter = 0;
		unsigned private_last_prime = 0;

		for (i = start; i < end; ++i) {    /* For every odd number */
      		prime = 2*i + 3;
		
		/* Keep searching for divisor until rem == 0 (i.e. non prime),
         	or we've reached the sqrt of prime (when div1 > div2) */

      		div1 = 1;
      		do {
        		div1 += 2;            /* Divide by 3, 5, 7, ... */
        		div2 = prime / div1;  /* Find the dividend */
        		rem = prime % div1;   /* Find remainder */
      		} while (rem != 0 && div1 <= div2);

      		if (rem != 0 || div1 == prime) {
        		/* prime is really a prime */
       			flags[i] = 1;
			private_counter++;
			private_last_prime = prime;
			}else {
        		/* prime is not a prime */
        		flags[i] = 0;
      			} /* else */
   } /* for */
			pthread_mutex_lock(mutex_for_prime_count);
        		count+= private_counter;
			if(lastPrime < private_last_prime) lastPrime = private_last_prime;
			pthread_mutex_unlock(mutex_for_prime_count);
        
  } /* parallelThreadsFunc */  

