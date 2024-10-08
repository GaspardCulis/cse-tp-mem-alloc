//------------------------------------------------------------------------------
// Projet : TP CSE (malloc)
// Cours  : Conception des systèmes d'exploitation et programmation concurrente
// Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
//------------------------------------------------------------------------------

#ifndef MEM_OS_H
#define MEM_OS_H

// include stdlib pour definition du type size_t
#include <stdlib.h>

// Macross for mem_alloc gesture; defines the minimum free bytes in a free block
#define FREE_BYTES 0 

/// Initiallize the memory control structures.
/// Deafault allocation strategy is `mem_first_fit`.
void mem_init(void);

/// Free memory block control struct
typedef struct mem_free_block_s {
  /// The size of the block, including the size of its control struct (+24)
  size_t size;
  /// Pointer to the next free block of the memory
  struct mem_free_block_s *next;
  /// Pointer to the previous free block of the memory
  struct mem_free_block_s *prev;
} mem_free_block_t;

/// Type of an allocation fit function signature
typedef mem_free_block_t *(mem_fit_function_t)(mem_free_block_t *, size_t);

/// Occupied memory control struct
typedef struct mem_busy_block_s {
  /// The total size of the occupied block (including control struct and possible extra for control purposes).
  size_t size;  
#if defined(DEBUG)
#define BUSY_BLOCK_INTEGRITY_SIGNATURE 0x69042
  /// Debug field to control wether a pointer to a `mem_busy_block` is valid
  unsigned int integrity_signature;
#endif
} mem_busy_block_t;

/// Memory space header struct
typedef struct mem_header_s {
  /// Pointer to the fisrt free block
  mem_free_block_t *first;
  /// Fit function of allocations (first fit by deafault)
  mem_fit_function_t *fit_function;
} mem_header_t;

/// @brief set the function for allocations.
/// @param mff is `mem_first_fit` | `mem_best_fit` | `mem_worst_fit` 
void mem_set_fit_handler(mem_fit_function_t *);

/// @brief Finds the first free block of a sufficient size for an allocation.
/// @param first_free_block initial free block to iterate on. Must be the same as in the `header`.
/// @param wanted_size needed size for allocation.
/// @return Pointer to the found free block; `NULL` if such a block cannot be found.
mem_fit_function_t mem_first_fit;

/// @brief Finds the largest free block of a sufficient size for an allocation.
/// @param first_free_block initial free block to iterate on. Must be the same as in the `header`.
/// @param wanted_size needed size for allocation.
/// @return Pointer to the found free block; `NULL` if such a block cannot be found.
mem_fit_function_t mem_worst_fit;

/// @brief Finds the smallest free block of a sufficient size for an allocation.
/// @param first_free_block initial free block to iterate on. Must be the same as in the `header`.
/// @param wanted_size needed size for allocation.
/// @return Pointer to the found free block; `NULL` if such a block cannot be found.
mem_fit_function_t mem_best_fit;

#endif /* MEM_OS_H */
