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

#define NB_TESTS 10

static void *alloc_max(size_t estimate) {
  void *result;
  static size_t last = 0;

  while((result = mem_alloc(estimate)) == NULL) {
    estimate--;
  }
  debug("Alloced %zu bytes at %p\n", estimate, result);
  if(last) {
    // Idempotence test
    assert(estimate == last);
  } else
    last = estimate;
  return result;
}

int main(int argc, char *argv[]) {
  fprintf(stderr,
          "Test réalisant de multiples fois une initialisation "
          "suivie d'une alloc max.\n"
          "Définir DEBUG à la compilation pour avoir une sortie un "
          "peu plus verbeuse."
          "\n");
  for(int i = 0; i < NB_TESTS; i++) {
    mem_init();
    alloc_max(mem_space_get_size());
  }

  // TEST OK
  return 0;
}
