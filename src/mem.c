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

  header->size = mem_space_get_size();
  header->first = (mem_free_block_t *)(header + sizeof(mem_header_t));
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
  // TODO: implement
  assert(!"NOT IMPLEMENTED !");
  return NULL;
}

//-------------------------------------------------------------
// mem_get_size
//-------------------------------------------------------------
size_t mem_get_size(void *zone) {
  // TODO: implement
  assert(!"NOT IMPLEMENTED !");
  return 0;
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
  // TODO: implement
  assert(!"NOT IMPLEMENTED !");
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
  while(current_block->size < wanted_size && current_block != NULL)
    current_block = current_block->next;
  return current_block;
}
//-------------------------------------------------------------
mem_free_block_t *mem_best_fit(mem_free_block_t *first_free_block,
                               size_t wanted_size) {
  // TODO: implement
  assert(!"NOT IMPLEMENTED !");
  return NULL;
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
