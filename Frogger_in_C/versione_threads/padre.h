//created by Thomas Nieddu on 21/02/25.
#ifndef PROGETTO_SO_PADRE_H
#define PROGETTO_SO_PADRE_H
#include <string.h>
#include "regole_gioco.h"
#include <pthread.h>
#include <signal.h>
//variabili globali per la pausa e il nickname
extern bool pausa;
extern const char *nickname;
//il buffer circolare condiviso
extern buffer_circolare buf_principale;

//funzioni per disegnare vita, info e timer
void disegna_quadrato_vita(int riga, int colonna);
void disegna_info();
void disegna_timer();

//gestisce il loop di gioco
void funzione_padre(int spawn_colonna, int spawn_riga, pthread_t ranaThread);
//schermate finali
void Vittoria();
void GameOver();

//funzioni per killare coccodrilli e la rana
void kill_coccodrilli(int n_coccodrilli);
void kill_rana(pthread_t ranaThread);

//funzioni per killare tutti i proiettili e granate
void kill_all_proiettili(struct personaggio *proiettili, int n_coccodrilli);
void kill_all_granate(struct personaggio *granate, int n_granate);

//questa resetta completamente lo scenario
void reset_scenario(pthread_t *pRanaThread,
                    struct personaggio *rana,
                    struct personaggio *coccodrilli,
                    int n_coccodrilli,
                    struct personaggio *proiettili,
                    struct personaggio *granate,
                    int spawn_colonna, int spawn_riga);

#endif // PROGETTO_SO_PADRE_H
