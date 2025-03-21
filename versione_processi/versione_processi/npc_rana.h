//
// Created by Davide Balestrino on 06/01/25.
//

#ifndef PROGETTO_SO_NPC_RANA_H
#define PROGETTO_SO_NPC_RANA_H

#include <ncurses.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include "prato_tane.h"

#define MINDISTANZA 10
#define MAXDISTANZA 25
#define MINLUNGHEZZA 8
#define MAXLUNGHEZZA 18

#define LUNGHEZZARANA 5

#define MAX_GR_SCHERMO 6

#define INIZIOPRIOIETTILI -1

#define SLEEP 1000
#define SLEEPGR 20000
#define SLEEPROJ 250000
#define SLEEPCROC1 450000
#define SLEEPCROC2 400000
#define SLEEPCROC3 350000

typedef enum {RANA, COCCODRILLO, PROIETTILE, GRANATA} tipo;

struct posizione {
    int x;
    int y;
};

struct personaggio {
    struct posizione posizione;
    tipo  tipo;
    int lunghezza;
    int id;
    bool destra;
    pid_t pid_figlio;
    bool animation;
}; //struttura personaggio con le informazioni di coccodrillo, rana, granata e proiettile

extern int canale_a_figlio[2]; //pipe
extern int canale_a_padre[2];

// lo sprite è 2x3 in ascii
extern int rana_altezza; //altezza rana
extern const char *prima_linea_sprite;
extern const char *seconda_linea_sprite;
extern const char *prima_linea_sprite2;
extern const char *seconda_linea_sprite2;

extern int punteggio;  //punteggio

//funz di disegno e gestione della rana e coccodrilli
void disegna_sprite(struct personaggio p);
void processo_rana(int spawn_riga, int spawn_colonna);

void disegna_coccodrillo(struct personaggio p);
void processo_coccodrilli(struct personaggio coccodrillo);

//funzione di disegno, reset e processo dei proiettili
void disegna_proiettile(struct personaggio p, int n_coccodrilli);
void reset_proiettile(struct personaggio p);
void processo_proiettile(struct personaggio coccodrillo);

// funzione di disegno, reset e processo della granata
void disegna_granata(struct personaggio p);
void processo_granata(struct personaggio granata, int direction);

#endif //PROGETTO_SO_NPC_RANA_H
