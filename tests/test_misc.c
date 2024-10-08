//------------------------------------------------------------------------------
// Projet : TP CSE (malloc)
// Cours  : Conception des systèmes d'exploitation et programmation concurrente
// Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
//------------------------------------------------------------------------------

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "mem.h"
#include "mem_os.h"
#include "mem_space.h"

int main(int argc, char *argv[]) {
  mem_init();
  fprintf(
      stderr,
      "Test réalisant des tests basiques des cas particuliers d'utilisation "
      "de l'allocateur\n");

  void *alloc;
  size_t alloc_size;

  // Alloc zero size
  assert((alloc = mem_alloc(0)) != NULL);
#if defined(DEBUG)
  printf("mem_free warn message shouldn't appear:\n");
#endif
  mem_free(alloc);
#if defined(DEBUG)
  printf("mem_free warn message should appear:\n");
#endif
  mem_free(alloc);

  // Alloc full mem_size
  assert(mem_alloc(mem_space_get_size()) == NULL);

  // Alloc negative mem_size
  assert(mem_alloc(-69) == NULL);

  // Free unallocated memory
#if defined(DEBUG)
  printf("mem_free warn message should appear:\n");
#endif
  mem_free((void *)69);

  // Boundary tests + mem_get_size
  alloc_size = sizeof(mem_busy_block_t) - 1;
  alloc = mem_alloc(alloc_size);
  assert(mem_get_size(alloc) >= alloc_size);

  // Small size alloc
  alloc = mem_alloc(1);
  mem_free(alloc);

  // TEST OK
  return 0;
}
