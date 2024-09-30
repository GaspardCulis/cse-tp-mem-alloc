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

typedef struct mem_iter_s {
  void *next_block;
  mem_free_block_t *next_free_block;
} mem_iter_t;

typedef struct mem_iter_item_s {
  void *addr;
  size_t size;
  char free;
} mem_iter_item_t;

mem_iter_t mem_iterator_init() {
  mem_header_t *header = mem_space_get_addr();

  mem_iter_t iterator;
  iterator.next_block = (void *)header + sizeof(mem_header_t);
  iterator.next_free_block = header->first;

  return iterator;
}

// TODO: Handle cases where the last block is a busy_block
mem_iter_item_t mem_iter_next(mem_iter_t *iterator) {
  mem_iter_item_t item;

  item.addr = iterator->next_block;
  if(iterator->next_block == (void *)iterator->next_free_block) {
    item.size = iterator->next_free_block->size + sizeof(mem_free_block_t);
    item.free = 1;

    iterator->next_free_block = iterator->next_free_block->next;
  } else {
    item.size = ((mem_busy_block_t *)iterator->next_block)->size +
                sizeof(mem_busy_block_t);
    item.free = 0;
  }

  iterator->next_block = item.addr + item.size;

  return item;
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

  mem_iter_t iterator = mem_iterator_init();
  while(iterator.next_free_block != NULL) {
    mem_iter_item_t item = mem_iter_next(&iterator);
    print(item.addr, item.size, item.free);
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
