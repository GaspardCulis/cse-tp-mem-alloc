//------------------------------------------------------------------------------
// Projet : TP CSE (malloc)
// Cours  : Conception des systèmes d'exploitation et programmation concurrente
// Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
//------------------------------------------------------------------------------

#ifndef MEM_OS_H
#define MEM_OS_H

// include stdlib pour definition du type size_t
#include <stdlib.h>

/* -----------------------------------------------*/
/* Interface de gestion de votre allocateur       */
/* -----------------------------------------------*/
// Initialisation
void mem_init(void);

/// Free memory block control struct
typedef struct mem_free_block_s {
  /// The size of the block, including the size of its control struct (+24)
  size_t size;
  /// Pointeur vers la structure de controle du prochain bloc memoire libre
  struct mem_free_block_s *next;
  /// Pointeur vers la structure de controle du bloc de memoire libre précédent
  struct mem_free_block_s *prev;
} mem_free_block_t;

/// Type of an allocation fit function signature
typedef mem_free_block_t *(mem_fit_function_t)(mem_free_block_t *, size_t);

// Definition la structure du bloc occupé
typedef struct mem_busy_block_s {
  /// The size of the occupied block, including the size of its control struct
  size_t size;  
#if defined(DEBUG)
#define BUSY_BLOCK_INTEGRITY_SIGNATURE 0x69042
  /// Debug field to control wether a pointer to a `mem_busy_block` is valid
  unsigned int integrity_signature;
#endif
} mem_busy_block_t;

/// Memory space header struct
typedef struct mem_header_s {
  /// The total of the memory, minus the size occupied by the header (-24)
  size_t size;
  /// Pointeur vers la structure de controle du premier bloc memoire libre
  mem_free_block_t *first;
  mem_fit_function_t *fit_function;
} mem_header_t;

// Choix de la fonction d'allocation
// = choix de la stratégie de l'allocation
void mem_set_fit_handler(mem_fit_function_t *);

// Stratégies de base (fonctions) d'allocation
mem_fit_function_t mem_first_fit;
mem_fit_function_t mem_worst_fit;
mem_fit_function_t mem_best_fit;

#endif /* MEM_OS_H */
