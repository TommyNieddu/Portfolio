//
// Created by Davide Balestrino on 06/01/25.
//

#ifndef PROGETTO_SO_REGOLE_GIOCO_H
#define PROGETTO_SO_REGOLE_GIOCO_H

#include "npc_rana.h"
#include <signal.h>
#include <sys/wait.h>
#define TEMPO_MASSIMO 180 // questo è il tempo massimo del timer
#define VITEINIZIALI 5
#define COCCODRILLIXRIGA 5

//variabili di gioco
extern int vite;  //numero di vite del giocatore
extern int tempo_rimasto;  //quanto tempo mi rimane

int conta_coccodrilli(); //conto di quanti coccodrilli ho bisogno
void spawn_coccodrilli(struct personaggio *coccodrilli, int n_coccodrilli, pid_t *coccodrilliPids); //genero i coccodrilli
void controllo_coccodrilli(struct personaggio *rana, struct personaggio coccodrilli[], int n_coccodrilli); //sposto la rana con i coccodrilli

void reset_rana(struct personaggio *rana, int x, int y, pid_t *old, struct personaggio *granate); //per riportare la rana alla posizione iniziale

//funzioni di controllo per le tane
bool tutte_tane_chiuse();
bool check_tane(struct personaggio rana);

bool perdivita();

//funzioni di controllo delle collisioni
bool controllo_proiettili(struct personaggio rana, struct personaggio proiettili[], int n_coccodrilli); //controllo se la rana è su un proiettile
bool  controllo_fiume(struct personaggio rana, struct personaggio coccodrilli[], int n_coccodrilli); //controllo se la rana è sul fiume
bool controllo_bordi(struct personaggio rana); //controllo se la rana è fuori dai margini di gioco

void collisioni_proiettili(struct personaggio *proiettili, struct personaggio *granate, int n_proiettili);


#endif //PROGETTO_SO_REGOLE_GIOCO_H
