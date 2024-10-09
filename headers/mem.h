//------------------------------------------------------------------------------
// Projet : TP CSE (malloc)
// Cours  : Conception des systèmes d'exploitation et programmation concurrente
// Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
//------------------------------------------------------------------------------

#ifndef MEM_H
#define MEM_H

#include <stddef.h>

/* -----------------------------------------------*/
/*                 User Interface                 */
/* -----------------------------------------------*/

/// @brief Allocate a block of memory of a given size.
/// @param size number of octets to allocate.
/// @return Address to the allocated zone; `NULL` if the allocation is impossible.
void *mem_alloc(size_t);

/// @brief Free an occupied block.
/// Prints a warning if the passed zone is already free.
/// @param zone address of a block to liberate.
void mem_free(void *);

/// @brief Get the maximum size of an allocated zone.
/// @param zone address of the allocated zone.
/// @return Size of allocated block in octets.
size_t mem_get_size(void *);

/// @brief Iterator on the contents of allocater.
void mem_show(void (*print)(void *, size_t, int free));

/* Enable logging for debugging */
void mem_set_logging(int enabled);

#endif  // MEM_H
