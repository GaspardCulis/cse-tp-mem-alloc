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

// Definion la structure du bloc libre
typedef struct mem_free_block_s {
  size_t size;                    // taille du bloc courant
  struct mem_free_block_s *next;  // pointeur vers le bloc memoire libre suivant
  struct mem_free_block_s *prev;  // poinet to the previous free block
} mem_free_block_t;

// Définition du type mem_fit_function_t
// type des fonctions d'allocation
typedef mem_free_block_t *(mem_fit_function_t)(mem_free_block_t *, size_t);

// Definition la structure du bloc occupé
typedef struct mem_busy_block_s {
  size_t size;  // taille du bloc courant
#if defined (DEBUG)
#define BUSY_BLOCK_INTEGRITY_SIGNATURE 0x69042
  unsigned int integrity_signature; 
#endif
} mem_busy_block_t;

// Definition la structure du header de la zone memoire
typedef struct mem_header_s {
  size_t size;              // taille complete de la memoire
  mem_free_block_t *first;  // pointeur vers le premier bloc memoire libre
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
