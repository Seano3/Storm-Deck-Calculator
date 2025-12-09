#ifndef STORM_DECK_CARDS_H
#define STORM_DECK_CARDS_H

#include "vars.h"

/* Number of templates in the library; keep in sync with cards.c */
#define LIBRARY_SIZE 31

/* Library of card templates (defined in cards.c) */
extern Card library[LIBRARY_SIZE];

/* Initialize the library; must be called before using `library` */
void init_cards(void);

#endif /* STORM_DECK_CARDS_H */