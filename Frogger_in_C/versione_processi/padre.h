//
// Created by Davide Balestrino on 15/01/25.
//

#ifndef PROGETTO_SO_PADRE_H
#define PROGETTO_SO_PADRE_H
#include <string.h>
#include "regole_gioco.h"
#include <sys/wait.h>
#include <signal.h>
extern int canale_a_figlio[2];  // Padre → Figlio
extern int canale_a_padre[2];

extern bool pausa;    //flag per vedere se sono in pausa
extern const char *nickname; //memorizzo il nome utente

//funzioni di disegno
void disegna_quadrato_vita(int riga, int colonna);
void disegna_info();
void disegna_timer();

//funzione del processo padre
void funzione_padre(int spawn_colonna, int spawn_riga, pid_t pid_rana);

//funzione di pausa
void toggle_pause_processes(bool pause_on, pid_t pid_rana, pid_t *coccodrilliPids, int n_coccodrilli, struct personaggio *proiettile, int n_proiettili, struct personaggio *granate);
void victory_screen();
void game_over_screen();
void check_granate(struct personaggio *granate, pid_t *oldgranataPids);
//funzione di terminazione dei processi coccodrillo
void kill_coccodrilli(int n_coccodrilli, pid_t *coccodrilliPids);
#endif //PROGETTO_SO_PADRE_H
