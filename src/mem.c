//------------------------------------------------------------------------------
// Projet : TP CSE (malloc)
// Cours  : Conception des systèmes d'exploitation et programmation concurrente
// Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
//------------------------------------------------------------------------------

#include "mem.h"

#include <assert.h>

#include "mem_os.h"
#include "mem_space.h"

void mem_init() {
  // Basic assertions, else nothing can be done
  assert(mem_space_get_addr() != NULL);
  assert(sizeof(mem_header_t) < mem_space_get_size());
  // Place header
  mem_header_t *header = mem_space_get_addr();
  // Create the first free block of the memory
  header->first = (void *)header + sizeof(mem_header_t);
  header->first->next = header->first->prev = NULL;
  header->first->size = mem_space_get_size() - sizeof(mem_header_t);
  // Fit strategy
  header->fit_function = mem_first_fit;
}

void *mem_alloc(size_t size) {
  // get header
  mem_header_t *header = mem_space_get_addr();

  // total size of allocation
  size_t alloc_size = size + sizeof(mem_busy_block_t);
  // size control, no loss of busy_block during future free
  if(alloc_size < sizeof(mem_free_block_t))
    alloc_size = sizeof(mem_free_block_t);

  // get the free block
  mem_free_block_t *alloc_block =
      header->fit_function(header->first, alloc_size);

  // if size is suitable or the header isn't empty
  if(alloc_block != NULL) {
    mem_busy_block_t *new_busy_block;
    /* case to split the free block in busy and free
        -- split size at least = free_block
    */
    if(alloc_block->size - alloc_size >= sizeof(mem_free_block_t) + 1) {
      // update the size of allocated free_block
      alloc_block->size -= alloc_size;
      //  put the busy block to the right
      new_busy_block = (alloc_block->size + (void *)alloc_block);
      new_busy_block->size = alloc_size;
      new_busy_block->prev = alloc_block;  // link the busy to the found block
    }
    // case entire allocation
    else {
      // if prev isn't header
      if(alloc_block->prev != NULL)
        alloc_block->prev->next = alloc_block->next;  // relink memory
      else
        // update header elsewise
        header->first = alloc_block->next;
      // relink next block
      if(alloc_block->next != NULL)
        alloc_block->next->prev = alloc_block->prev;
      new_busy_block = (mem_busy_block_t *)alloc_block;  // recast free to busy
      new_busy_block->size = alloc_block->size;
      new_busy_block->prev =
          alloc_block->prev;  // link the busy to the previous of the free block
    }

#if defined(DEBUG)
    new_busy_block->integrity_signature = BUSY_BLOCK_INTEGRITY_SIGNATURE;
#endif

    return (void *)new_busy_block +
           sizeof(mem_busy_block_t);  // give user memory
  }
  return NULL;
}

size_t mem_get_size(void *zone) {
  mem_busy_block_t *busy_block = zone - sizeof(mem_busy_block_t);

#if defined(DEBUG)
  // Check if the `mem_busy_block_s` we got from the `zone` pointer is valid
  assert(busy_block->integrity_signature == BUSY_BLOCK_INTEGRITY_SIGNATURE);
#endif

  return busy_block->size - sizeof(mem_busy_block_t);
}

/// Iterator control struct over the content of the memory.
/// Constructed using `mem_iterator_init`.
typedef struct mem_iter_s {
  /// The block directly next to the previous one, could be free or busy
  void *next_block;
  /// The next free block
  mem_free_block_t *next_free_block;
  /// Wether to iteration is complete (1 if over, 0 if can continue)
  char finished;
} mem_iter_t;

/// The item type returned by the `mem_iter_next` function.
/// Represents a block of memory.
typedef struct mem_iter_item_s {
  /// The address of the block, including its control struct
  void *addr;
  /// The size of the block, including the size of its control struct
  size_t size;
  /// Wether the block is free or not, 0 if allocated, 1 if free
  char free;
} mem_iter_item_t;

/// Returns a new `mem_iter_t` initialized at the beginning of the memory
mem_iter_t mem_iterator_init() {
  mem_header_t *header = mem_space_get_addr();

  mem_iter_t iterator;
  iterator.next_block = (void *)header + sizeof(mem_header_t);
  iterator.next_free_block = header->first;
  /// Will never have 0 iterations so we can initialize to 0
  iterator.finished = 0;

  return iterator;
}

/// Returns the next block in memory as a `mem_iter_item_s`.
/// Mutates `iterator` parameter.
mem_iter_item_t mem_iter_next(mem_iter_t *iterator) {
  mem_iter_item_t item;

  item.addr = iterator->next_block;
  if(iterator->next_block == (void *)iterator->next_free_block) {
    item.size = iterator->next_free_block->size;
    item.free = 1;

    iterator->next_free_block = iterator->next_free_block->next;
  } else {
    item.size = ((mem_busy_block_t *)iterator->next_block)->size;
    item.free = 0;
  }

  iterator->next_block = item.addr + item.size;
  iterator->finished =
      iterator->next_block == mem_space_get_addr() + mem_space_get_size();

  return item;
}

void mem_free(void *zone) {
  mem_header_t *header = mem_space_get_addr();
  mem_busy_block_t *busy_block = zone - sizeof(mem_busy_block_t);

#if defined(DEBUG)
  // Check if the `mem_busy_block_s` we got from the `zone` pointer is valid
  if(busy_block->integrity_signature != BUSY_BLOCK_INTEGRITY_SIGNATURE) {
    printf("Tried to free an invalid/already-freed/0-sized pointer\n");
    return;
  }
#endif

  void *next_block = (void *)busy_block + busy_block->size;
  mem_free_block_t *prev_free_block = busy_block->prev;
  mem_free_block_t *next_free_block =
      prev_free_block != NULL ? prev_free_block->next : header->first;

  mem_free_block_t *new_free = (mem_free_block_t *)busy_block;
  new_free->size = busy_block->size;

  // Link right
  if(next_free_block != NULL) {
    if(next_block == next_free_block) {
      // The next block is free, merge them by extending `new_free`
      new_free->size += next_free_block->size;
      new_free->next = next_free_block->next;
      if(next_free_block->next != NULL) next_free_block->next->prev = new_free;
      // We leave the old `next_free` as-is in memory
    } else {
      new_free->next = next_free_block;
      next_free_block->prev = new_free;
    }
  } else {
    new_free->next = NULL;
  }

  // Link left
  if(prev_free_block != NULL) {
    void *prev_free_next_block =
        (void *)prev_free_block + prev_free_block->size;
    if(prev_free_next_block == new_free) {
      // The next block is free, merge them by extending `prev_free_block`
      prev_free_block->next = new_free->next;
      prev_free_block->size += new_free->size;
      if(new_free->next != NULL) new_free->next->prev = prev_free_block;
      // We leave the `new_free` as is in memory
      new_free = prev_free_block;
    } else {
      new_free->prev = prev_free_block;
      prev_free_block->next = new_free;
    }
  } else {
    new_free->prev = NULL;
    header->first = new_free;
  }

  // Maybe link possible next busy_block->prev
  void *new_next_block = (void *)new_free + new_free->size;
  void *mem_end = mem_space_get_addr() + mem_space_get_size();
  if((new_free->next != new_next_block) ||
     (new_free->next == NULL && next_block != mem_end)) {
    ((mem_busy_block_t *)new_next_block)->prev = new_free;
  }
}

void mem_show(void (*print)(void *, size_t, int free)) {
  mem_header_t *header = mem_space_get_addr();

  // We consider the header as an occupied block
  print(header, sizeof(mem_header_t), 0);

  mem_iter_t iterator = mem_iterator_init();
  while(iterator.finished == 0) {
    mem_iter_item_t item = mem_iter_next(&iterator);
    print(item.addr, item.size, item.free);
  }
}

void mem_set_fit_handler(mem_fit_function_t *mff) {
  mem_header_t *header = mem_space_get_addr();
  header->fit_function = mff;
}

mem_free_block_t *mem_first_fit(mem_free_block_t *first_free_block,
                                size_t wanted_size) {
  mem_free_block_t *current_block = first_free_block;
  while(current_block != NULL && current_block->size < wanted_size)
    current_block = current_block->next;
  return current_block;
}

mem_free_block_t *mem_best_fit(mem_free_block_t *first_free_block,
                               size_t wanted_size) {
  mem_free_block_t *current_fit = first_free_block;
  mem_free_block_t *best_fit = NULL;
  // memory can be full
  if(current_fit == NULL)
    return NULL;
  long long best_fit_score =
      current_fit->size - wanted_size +
      1;  // + 1 for best_fit init on the `first_free_block` if score is valid
  while(current_fit != NULL && best_fit_score != 0) {
    long long current_fit_score = current_fit->size - wanted_size;

    // If fit_score is negative, then the current free block is too small
    // Else the greater it is, the more space is left unused
    if(current_fit_score >= 0 && current_fit_score < best_fit_score) {
      best_fit = current_fit;
      best_fit_score = current_fit_score;
    }

    current_fit = current_fit->next;
  }
  return best_fit;
}

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
