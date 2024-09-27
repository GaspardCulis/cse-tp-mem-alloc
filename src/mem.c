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
  header->first = header->first_free = (void *)header + sizeof(mem_header_t);
  header->fit_function =
      mem_first_fit;  // TODO: Proper default definition in mem_os.h

  header->first->size =
      mem_space_get_size() - sizeof(mem_header_t) - sizeof(mem_block_t);
  header->first->allocated = 0;
  header->first->next = header->first->prev = header->first->next_free = NULL;
}

//-------------------------------------------------------------
// mem_alloc
//-------------------------------------------------------------
/**
 * Allocate a bloc of the given size.
 **/
void *mem_alloc(size_t size) {
  // assert(!"NOT IMPLEMENTED !");
  size_t alloc_size = size + sizeof(mem_block_t);
  mem_header_t *header = mem_space_get_addr();

  // get the free block
  mem_block_t *alloc_block =
      header->fit_function(header->first_free, alloc_size);

  if(alloc_block != NULL) {
    // case to split the free block in busy and free
    if(alloc_block->size - alloc_size > sizeof(mem_block_t)) {
      mem_block_t *split_block = (void *)alloc_block + alloc_size;

      // put the split block to the right
      if(alloc_block->prev != NULL) {
        alloc_block->prev->next_free = split_block;  // relink memory
      } else {
        // update header
        header->first_free = split_block;
      }

      split_block->next = alloc_block->next;
      split_block->size = alloc_block->size - size;
      split_block->allocated = 0;

      alloc_block->next = alloc_block->next_free = split_block;
      alloc_block->size = size;
      alloc_block->allocated = 1;

    } else {
      // case entire allocation
      if(alloc_block->prev != NULL)
        alloc_block->prev->next_free = alloc_block->next;  // relink memory
      alloc_block->allocated = 1;
    }

    return alloc_block + sizeof(mem_block_t);  // give user memory
  }
  return NULL;
}

//-------------------------------------------------------------
// mem_get_size
//-------------------------------------------------------------
size_t mem_get_size(void *zone) {
  mem_block_t *block = zone - sizeof(mem_block_t);

  // Check if the `mem_block_s` we got from the `zone` pointer is valid
  assert(block->allocated == 1);

  return block->size;
}

//-------------------------------------------------------------
// mem_free
//-------------------------------------------------------------
/**
 * Free an allocaetd bloc.
 **/
void mem_free(void *zone) {
  mem_block_t *block = zone - sizeof(mem_block_t);

  // Check if the `mem_block_s` we got from the `zone` pointer is valid
  assert(block->allocated == 1);

  block->allocated = 0;
}

//-------------------------------------------------------------
// Itérateur(parcours) sur le contenu de l'allocateur
// mem_show
//-------------------------------------------------------------
void mem_show(void (*print)(void *, size_t, int free)) {
  mem_header_t *header = mem_space_get_addr();

  // We consider the header as an occupied block
  print(header, sizeof(mem_header_t), 0);

  mem_block_t *current = header->first;
  while(current != NULL) {
    // Print current free block
    print(current, current->size, 1 - current->allocated);

    current = current->next;
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
mem_block_t *mem_first_fit(mem_block_t *first_free_block, size_t wanted_size) {
  mem_block_t *current_block = first_free_block;
  while(current_block->size < wanted_size && current_block != NULL)
    current_block = current_block->next_free;
  return current_block;
}
//-------------------------------------------------------------
mem_block_t *mem_best_fit(mem_block_t *first_free_block, size_t wanted_size) {
  mem_block_t *best_fit = NULL;
  mem_block_t *current_fit = first_free_block;
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

    current_fit = current_fit->next_free;
  }
  return best_fit;
}

//-------------------------------------------------------------
mem_block_t *mem_worst_fit(mem_block_t *first_free_block, size_t wanted_size) {
  mem_block_t *current_block = first_free_block;  // iterate through memory
  mem_block_t *worst_fit = NULL;
  size_t worst_size = wanted_size;  // at least must have this size
  while(current_block != NULL) {
    if(current_block->size >= worst_size) {
      worst_fit = current_block;
      worst_size = current_block->size;
    }
    current_block = current_block->next_free;
  }
  return worst_fit;
}
