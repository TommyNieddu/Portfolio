//created by Thomas Nieddu on 21/02/25.
#ifndef PROGETTO_SO_REGOLE_GIOCO_H
#define PROGETTO_SO_REGOLE_GIOCO_H

#include "npc_rana.h"
#include <pthread.h>
#include <signal.h>
//tempo massimo e vite iniziali
#define TEMPO_MASSIMO 180
#define VITEINIZIALI 5
#define COCCODRILLIXRIGA 5

//variabili di gioco
extern int vite;
extern int tempo_rimasto;

//array di thread coccodrilli
extern pthread_t *coccodrilliThreads;
extern bool movimentoManuale;

//dichiaro le funzioni principali
int  conta_coccodrilli();
//creo i coccodrilli
void spawn_coccodrilli(struct personaggio *coccodrilli, int n_coccodrilli);

//controllo se la rana sta cavalcando un coccodrillo
void controllo_coccodrilli(struct personaggio *rana,
                           struct personaggio coccodrilli[],
                           int n_coccodrilli);

//reset_rana: la riporto a x,y
void reset_rana(struct personaggio *rana, int x, int y);

//se ogni tana è chiusa, return true
bool tutte_tane_chiuse();

//se la rana è entrata in una tana
bool check_tane(struct personaggio rana);

//decremento vita e punteggio
bool perdivita();

//vedo se la rana collide con proiettili
bool controllo_proiettili(struct personaggio rana,
                          struct personaggio proiettili[],
                          int n_coccodrilli);

//fiume e bordi
bool controllo_fiume(struct personaggio rana,
                     struct personaggio coccodrilli[],
                     int n_coccodrilli);
bool controllo_bordi(struct personaggio rana);

#endif //PROGETTO_SO_REGOLE_GIOCO_H
