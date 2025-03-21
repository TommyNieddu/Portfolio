//created by Thomas Nieddu on 21/02/25.
#ifndef PROGETTO_SO_NPC_RANA_H
#define PROGETTO_SO_NPC_RANA_H

#include <stdbool.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "prato_tane.h"
#include "buffer.h"

//definisco costanti per dimensioni e velocità
#define DISTANZA_MINIMA 10
#define DISTANZA_MASSIMA 25
#define LUNGHEZZA_MINIMA 8
#define LUNGHEZZA_MASSIMA 18
#define LUNGHEZZA_RANA 5
#define INIZIOPRIOIETTILI -1
#define GRANATE_MASSIME 900
#define MAX_GRANATE_ATTIVE 6
#define SLEEP 5000
#define SLEEPGRANATA 60000
#define SLEEPPROJ 200000

//punteggio  globale
extern int punteggio;

//argomenti per coccodrillo e granata
typedef struct {
    struct personaggio coccodrillo;
} CoccodrilloArgs;

typedef struct {
    struct personaggio granata;
    int direction;
} GranataArgs;

typedef struct {
    struct personaggio coccodrillo;
    int direction;
} CoccodrilloProjArgs;

typedef struct {
    struct personaggio proiettile;
    int directionY;
} TanaProjArgs;

//argomenti per la rana
typedef struct {
    int spawn_riga;
    int spawn_colonna;
} RanaArgs;
//mi serve a creare un thread con i parametri della rana
void* wrapper_rana(void *arg);

//sprite della rana e ho due varianti per l’animazione
extern int rana_altezza;
extern const char *prima_linea_sprite;
extern const char *seconda_linea_sprite;
extern const char *prima_linea_sprite2;
extern const char *seconda_linea_sprite2;
void* processo_proiettile_tana(void* arg);
//funzioni per disegnare i vari personaggi
void disegna_sprite(struct personaggio p);
void disegna_coccodrillo(struct personaggio p);
void disegna_proiettile(struct personaggio p, int n_coccodrilli);
void disegna_granata(struct personaggio p);
//reset_proiettile
void reset_proiettile(struct personaggio p);
void disegna_proiettile_tana(struct personaggio p, int max_proiettili_tana);
//thread per rana, coccodrillo, proiettile, granata
void* processo_rana(int spawn_riga, int spawn_colonna);
void* processo_coccodrilli(void* arg);
void* processo_proiettile(void* arg);
void* processo_granata(void* arg);

#endif // PROGETTO_SO_NPC_RANA_H
