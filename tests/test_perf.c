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
#define NUM_CYCLES 1000  // Number of allocations and deallocations to measure
static void *allocs[MAX_ALLOC];

int main(int argc, char *argv[]) {
  mem_init();
  mem_fit_function_t* fit = mem_first_fit;
  if(argc == 2) {
    if(strcmp(argv[1], "first") == 0) {
      fit = mem_first_fit;
      printf("\nCalled on first fit strategy.\n");
    } else if(strcmp(argv[1], "best") == 0) {
      fit = mem_best_fit;
      printf("\nCalled on best fit strategy.\n");
    } else if(strcmp(argv[1], "worst") == 0) {
      fit = mem_worst_fit;
      printf("\nCalled on worst fit strategy.\n");
    }
  }

  fprintf(stderr,
          "Test making allocs and frees on random "
          "to benchmark average execution time of "
          "mem_alloc and free functions.\n");
  fprintf(stderr, "Doing %d test cycles for a given fit function (first_fit by default).\nAlloctions per cycle: <= %d\n", NUM_CYCLES, NUM_TESTS);
  srand(time(NULL));
  // test units
  srand(time(NULL));
  struct timespec start, end;
  double alloc_time_ns = 0, free_time_ns = 0;
  int alloc_cnt = 0;
  int free_cnt = 0;
  // big evaluating cycle
  for(int cycle = 0; cycle<NUM_CYCLES; cycle++){
    mem_init();
    mem_set_fit_handler(fit);
    int free;

    // random memory fragmentation cycle
    for(int i = 0; i < NUM_TESTS; ++i) {
      int size = (rand() % MAX_BLOC) + 1;
      // Measure mem_alloc time
      clock_gettime(CLOCK_MONOTONIC, &start);
      allocs[i] = mem_alloc(size);
      clock_gettime(CLOCK_MONOTONIC, &end);
      alloc_cnt++;
      alloc_time_ns +=
          (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
      if(allocs[i] == NULL) {
        break;
      }
      if(rand() % FREQ_FREE == 0) {
        // Measure mem_free time
        free = ((rand() % (i + 1)) - 1);
        assert(allocs[free] <
              (void *)((char *)mem_space_get_addr() + mem_space_get_size()));
        if(allocs[free] != NULL){
          clock_gettime(CLOCK_MONOTONIC, &start);
          mem_free(allocs[free]);
          clock_gettime(CLOCK_MONOTONIC, &end);
          free_cnt++;
          free_time_ns +=
              (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
          allocs[free] = NULL;
        }
      }
    }
  }
  
  double mean_alloc_time_ns = alloc_time_ns / alloc_cnt;
  double mean_free_time_ns = free_time_ns / free_cnt;
  printf("\n------------------------------- STATS --------------------------------\n");
  printf("-- Average number of allocations: %d\n", alloc_cnt/NUM_CYCLES);
  printf("-- Mean mem_alloc call time: %.2f nanoseconds\n", mean_alloc_time_ns);
  printf("-- Mean mem_free call time: %.2f nanoseconds\n", mean_free_time_ns);

  // End
  return 0;
}
