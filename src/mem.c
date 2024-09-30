//------------------------------------------------------------------------------
// Projet : TP CSE (malloc)
// Cours  : Conception des systèmes d'exploitation et programmation concurrente
// Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
//------------------------------------------------------------------------------

#include "mem.h"

#include <assert.h>

#include "mem_os.h"
#include "mem_space.h"

//-------------------------------------------------------------
// mem_init
//-------------------------------------------------------------
/**
 * Initialize the memory allocator.
 * If already init it will re-init.
 **/
void mem_init() {
  assert(sizeof(mem_header_t) < mem_space_get_size());

  mem_header_t *header = mem_space_get_addr();

  header->size = mem_space_get_size() - sizeof(mem_header_t);
  header->first = (void *)header + sizeof(mem_header_t);
  header->first->next = header->first->prev = NULL;
  header->first->size =
      mem_space_get_size() - sizeof(mem_header_t) - sizeof(mem_free_block_t);
  header->fit_function =
      mem_first_fit;  // TODO: Proper default definition in mem_os.h
}

//-------------------------------------------------------------
// mem_alloc
//-------------------------------------------------------------
/**
 * Allocate a bloc of the given size.
 **/
void *mem_alloc(size_t size) {
  // get header
  mem_header_t *header = mem_space_get_addr();
  size_t alloc_size = size + sizeof(mem_busy_block_t);

  // get the free block
  mem_free_block_t *alloc_block =
      header->fit_function(header->first, alloc_size);

  if(alloc_block != NULL) {
    // case to split the free block in busy and free
    if(alloc_block->size - alloc_size >
       sizeof(mem_free_block_t) + sizeof(mem_busy_block_t)) {
      mem_free_block_t *split_block = (void *)alloc_block + alloc_size;
      // put the free split block to the right
      if(alloc_block->prev != NULL)
        split_block->prev = alloc_block->prev;
      else
        // update header
        header->first = alloc_block->prev;

      split_block->next = alloc_block->next;
      split_block->size =
          alloc_block->size - alloc_size - sizeof(mem_free_block_t);

      if(alloc_block->prev != NULL)
        (alloc_block->prev)->next = split_block;  // relink memory
      else {
        // update header
        header->first = split_block;
      }
    } else {
      // case entire allocation
      alloc_block->prev->next = alloc_block->next;  // relink memory
    }
    mem_busy_block_t *new_busy_block =
        (mem_busy_block_t *)alloc_block;  // recast free to busy
    new_busy_block->size = size;

    return (void *)new_busy_block +
           sizeof(mem_busy_block_t);  // give user memory
  }
  return NULL;
}

//-------------------------------------------------------------
// mem_get_size
//-------------------------------------------------------------
size_t mem_get_size(void *zone) {
  mem_busy_block_t *busy_block = zone - sizeof(mem_busy_block_t);

#if defined(DEBUG)
  // Check if the `mem_busy_block_s` we got from the `zone` pointer is valid
  assert(busy_block->integrity_signature == BUSY_BLOCK_INTEGRITY_SIGNATURE);
#endif

  return busy_block->size;
}

//-------------------------------------------------------------
// mem_free
//-------------------------------------------------------------
/**
 * Free an allocaetd bloc.
 **/
void mem_free(void *zone) {
  // TODO: implement
  assert(!"NOT IMPLEMENTED !");
}

//-------------------------------------------------------------
// Itérateur(parcours) sur le contenu de l'allocateur
// mem_show
//-------------------------------------------------------------
void mem_show(void (*print)(void *, size_t, int free)) {
  mem_header_t *header = mem_space_get_addr();

  // We consider the header as an occupied block
  print(header, sizeof(mem_header_t), 0);

  void *next_block = (void *)header + sizeof(mem_header_t);
  mem_free_block_t *next_free = header->first;
  while(next_free != NULL) {
    if(next_free != next_block) {
      // There is a gap between the two blocks addresses, should be a busy_block
      mem_busy_block_t *busy_block = next_block;
      // Tant que l'on a pas atteint le prochain block de mémoire libre
      while((void *)busy_block != (void *)next_free) {
        size_t total_block_size = busy_block->size + sizeof(mem_busy_block_t);
        // Affichage du block
        print(busy_block, total_block_size, 0);
        // Get next busy block
        busy_block = (void *)busy_block + total_block_size;
      }
    }

    size_t total_block_size = next_free->size + sizeof(mem_free_block_t);
    // Print current free block
    print(next_free, total_block_size, 1);

    next_block = (void *)next_free + total_block_size;
    next_free = next_free->next;
  }
}

//-------------------------------------------------------------
// mem_fit
//-------------------------------------------------------------
void mem_set_fit_handler(mem_fit_function_t *mff) {
  mem_header_t *header = mem_space_get_addr();
  header->fit_function = mff;
}

//-------------------------------------------------------------
// Stratégies d'allocation
//-------------------------------------------------------------
mem_free_block_t *mem_first_fit(mem_free_block_t *first_free_block,
                                size_t wanted_size) {
  mem_free_block_t *current_block = first_free_block;
  while(current_block != NULL && current_block->size < wanted_size)
    current_block = current_block->next;
  return current_block;
}
//-------------------------------------------------------------
mem_free_block_t *mem_best_fit(mem_free_block_t *first_free_block,
                               size_t wanted_size) {
  mem_free_block_t *best_fit = NULL;
  mem_free_block_t *current_fit = first_free_block;
  // TODO: Think about overflows
  long best_fit_score = current_fit->size - wanted_size;
  while(current_fit != NULL && best_fit_score == 0) {
    long current_fit_score = current_fit->size - wanted_size;

    // If fit_score is negative, then the current free block is too small
    // Else the greater it is, the more space is left unused
    if(current_fit_score > 0 && current_fit_score < best_fit_score) {
      best_fit = current_fit;
      best_fit_score = current_fit_score;
    }

    current_fit = current_fit->next;
  }
  return best_fit;
}

//-------------------------------------------------------------
mem_free_block_t *mem_worst_fit(mem_free_block_t *first_free_block,
                                size_t wanted_size) {
  mem_free_block_t *current_block = first_free_block;  // iterate through memory
  mem_free_block_t *worst_fit = NULL;
  size_t worst_size = wanted_size;  // at least must have this size
  while(current_block != NULL) {
    if(current_block->size >= worst_size) {
      worst_fit = current_block;
      worst_size = current_block->size;
    }
    current_block = current_block->next;
  }
  return worst_fit;
}
