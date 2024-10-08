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
#define NUM_ALLOCS 10000  // Number of allocations and deallocations to measure
#define NUM_CYCLES 1000  // Number of allocations and deallocations to measure

static void *allocs[MAX_ALLOC];

/* utility functions to do some stats */
//check if memory splits upon an alloc
int will_split(size_t size) {
  // get header
  mem_header_t *header = mem_space_get_addr();
  // total size of allocation
  size_t alloc_size = size + sizeof(mem_busy_block_t);
  // size control, no loss of busy_block during future free
  if(alloc_size < sizeof(mem_free_block_t))
    alloc_size = sizeof(mem_free_block_t);

  mem_free_block_t *alloc_block =
      header->fit_function(header->first, alloc_size);
  if(alloc_block != NULL)
    if(alloc_block->size - alloc_size >= sizeof(mem_free_block_t) + 1)
      return 1;
  return 0;
}
// external frag computation:
long calc_ext(){
  long total_external = 0;
  mem_header_t* header = mem_space_get_addr();
  mem_free_block_t* current = header->first;
  while(current != NULL){
    if(current->size < MAX_BLOC)
      total_external+= (long) current->size;
    current = current->next;
  }
  return total_external;
}

int main(int argc, char *argv[]) {
  fprintf(stderr,
          "Test making allocs and frees on random"
          "to benchmark the fragmentation of memory"
          "with different fit functions.\n");
  fprintf(stderr, "\nDoing %d test cycles for a given fit function (first_fit by default).\nAlloctions per cycle: <= %d\n", NUM_CYCLES, NUM_ALLOCS);
  srand(time(NULL));
  mem_fit_function_t* fit = mem_first_fit;
  if(argc == 2) {
    if(strcmp(argv[1], "first") == 0) {
      fit = mem_first_fit;
      printf("Called on first fit strategy\n");
    } else if(strcmp(argv[1], "best") == 0) {
      fit = mem_best_fit;
      printf("Called on best fit strategy\n");
    } else if(strcmp(argv[1], "worst") == 0) {
      fit = mem_worst_fit;
      printf("Called on worst fit strategy\n");
    }
  }
  // units to make stats
  double user_proportion = 0;
  double sys_proportion = 0;
  double ext_frag_proportion = 0;
  double int_frag_proportion = 0;
  long avg_usr_bytes = 0;
  long avg_sys_bytes = 0;
  long avg_ext_bytes = 0;
  long avg_int_bytes = 0;

  double mem_size = (double) mem_space_get_size();

  for(int cycle = 0; cycle<NUM_CYCLES; cycle++){
    mem_init();
    mem_set_fit_handler(fit);
    // measure memory consommation
    long total_sys = (long)(sizeof(mem_header_t) + sizeof(mem_free_block_t));
    int free;
    //trace internal fragmentation
    size_t internal_frag_tab[MAX_ALLOC];
    for(int i = 0; i<MAX_ALLOC; i++)
      internal_frag_tab[i] = 0;
    //trace user allocs
    size_t mem_user_tab[MAX_ALLOC];
    for(int i = 0; i<MAX_ALLOC; i++)
      mem_user_tab[i] = 0;

    for(int i = 0; i < NUM_ALLOCS; ++i) {
      int size = (rand() % MAX_BLOC) + 1;

      if(will_split(size))
        total_sys += sizeof(mem_busy_block_t);
      else
        total_sys-= sizeof(mem_free_block_t) - sizeof(mem_busy_block_t);

      allocs[i] = mem_alloc(size);
      if(allocs[i] == NULL) {
        break;
      }
      mem_user_tab[i] = size;
      internal_frag_tab[i] = mem_get_size(allocs[i]) - size; // trace internal leaks

      if(rand() % FREQ_FREE == 0) {
        free = ((rand() % (i + 1)) - 1);
        assert(allocs[free] <
              (void *)((char *)mem_space_get_addr() + mem_space_get_size()));
        // avoid uselles frees
        if(allocs[free] != NULL){
          mem_free(allocs[free]);
          allocs[free] = NULL;
          total_sys += (long)(sizeof(mem_free_block_t) - sizeof(mem_busy_block_t));
          internal_frag_tab[i] = 0; // memory freed, leaks cleaned
          mem_user_tab[free] = 0;
        }
      }
    }
    long total_user = 0;
    long total_int = 0;
    for(int i = 0; i < NUM_ALLOCS; ++i) {
      total_user += (long) mem_user_tab[i];
      total_int += (long) internal_frag_tab[i];
    }
    long total_ext = calc_ext();

    avg_usr_bytes+=total_user;
    avg_sys_bytes+=total_sys;
    avg_int_bytes+=total_int;
    avg_ext_bytes+=total_ext;

    user_proportion += ((double)total_user)/((double)(total_int + total_sys + total_user));
    //user_proportion += ((double)total_user)/mem_size;
    sys_proportion += ((double)total_sys)/((double)(total_int + total_sys + total_user));
    //sys_proportion += ((double)total_sys)/mem_size;
    ext_frag_proportion += ((double)total_ext)/mem_size;
    int_frag_proportion += ((double)total_int)/((double)total_user);
  }
  printf("\n------------------------------- STATS --------------------------------\n");
  printf("-- User memory consomation on average: %.4lf %% ~ %ld bytes\n", user_proportion*100/NUM_CYCLES, 
            avg_usr_bytes/(long)NUM_CYCLES);
  printf("-- System memory consomation on average: %.4lf %% ~ %ld bytes\n", sys_proportion*100/NUM_CYCLES, 
            avg_sys_bytes/(long)NUM_CYCLES);
  printf("-- External memory fragmentation on average (in relation to the entire memory): %.4lf %% ~ %ld bytes\n", ext_frag_proportion*100/NUM_CYCLES, 
            avg_ext_bytes/(long)NUM_CYCLES);
  printf("-- Internal memory fragmentation on average (in relation to user memory): %.4lf %% ~ %ld bytes\n", int_frag_proportion*100/NUM_CYCLES, 
            avg_int_bytes/(long)NUM_CYCLES);
  // End
  return 0;
}
