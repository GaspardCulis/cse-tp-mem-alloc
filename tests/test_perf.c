// WTF wont compile if I don't define this shit
#define _XOPEN_SOURCE 500

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "mem.h"
#include "mem_os.h"
#include "mem_space.h"

#define MAX_ALLOC 100000
#define MAX_BLOC 200
#define FREQ_FREE 3
#define NUM_TESTS 10000  // Number of allocations and deallocations to measure

static void *allocs[MAX_ALLOC];

int main(int argc, char *argv[]) {
  mem_init();
  if(argc == 2) {
    if(strcmp(argv[1], "first") == 0) {
      mem_set_fit_handler(mem_first_fit);
      printf("Stratégie first fit\n");
    } else if(strcmp(argv[1], "best") == 0) {
      mem_set_fit_handler(mem_best_fit);
      printf("Stratégie best fit\n");
    } else if(strcmp(argv[1], "worst") == 0) {
      mem_set_fit_handler(mem_worst_fit);
      printf("Stratégie worst fit\n");
    }
  }
  srand(time(NULL));
  fprintf(stderr,
          "Test réalisant des series d'allocations / désallocations de manière "
          "aléatoire afin de benchmarker le temps d'exécution des functions "
          "d'allocation et de désallocation.\n");

  // Measure mean mem_alloc and mem_free call time
  struct timespec start, end;
  double alloc_time_ns = 0, free_time_ns = 0;
  int free;

  for(int i = 0; i < NUM_TESTS; ++i) {
    int size = (rand() % MAX_BLOC) + 1;

    // Measure mem_alloc time
    clock_gettime(CLOCK_MONOTONIC, &start);
    allocs[i] = mem_alloc(size);
    clock_gettime(CLOCK_MONOTONIC, &end);
    alloc_time_ns +=
        (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);

    if(allocs[i] == NULL) {
      printf(
          "Tentative d'allocation de  %d octets.\n"
          "Impossible car la mémoire est trop fragmentée.\n"
          "%i blocs ont été alloué (certains ont peut-être été libérés)\n",
          size, i);
      break;
    }

    if(rand() % FREQ_FREE == 0) {
      // Measure mem_free time
      free = ((rand() % (i + 1)) - 1);
      assert(allocs[free] <
             (void *)((char *)mem_space_get_addr() + mem_space_get_size()));
      clock_gettime(CLOCK_MONOTONIC, &start);
      mem_free(allocs[free]);
      clock_gettime(CLOCK_MONOTONIC, &end);
      free_time_ns +=
          (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
      allocs[free] = NULL;
    }
  }

  double mean_alloc_time_ns = alloc_time_ns / NUM_TESTS;
  double mean_free_time_ns = free_time_ns / NUM_TESTS;

  printf("Mean mem_alloc call time: %.2f nanoseconds\n", mean_alloc_time_ns);
  printf("Mean mem_free call time: %.2f nanoseconds\n", mean_free_time_ns);

  // Fin du test
  return 0;
}
