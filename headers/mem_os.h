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
typedef struct mem_block_s {
  size_t size;     // taille du bloc courant,
                   // ne comprend pas la taille de la structture de contrôle
  char allocated;  // 1 si le block est alloué, 0 sinon
                   // Utile uniquement pour le debug
  struct mem_block_s *next;       // pointeur vers le bloc memoire suivant
  struct mem_block_s *prev;       // pointeur vers le bloc memoire précédent
  struct mem_block_s *next_free;  // pointeur vers le bloc memoire libre suivant
} mem_block_t;

// Définition du type mem_fit_function_t
// type des fonctions d'allocation
typedef mem_block_t *(mem_fit_function_t)(mem_block_t *, size_t);

// Definition la structure du header de la zone memoire
typedef struct mem_header_s {
  size_t size;              // taille complete de la memoire
  mem_block_t *first;       // pointeur vers le premier bloc memoire
  mem_block_t *first_free;  // pointeur vers le premier bloc memoire libre
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
