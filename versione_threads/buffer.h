//created by Thomas Nieddu on 21/02/25.
#ifndef BUFFER_H
#define BUFFER_H

#include <pthread.h>
#include <stdbool.h>
#include <signal.h>
//per tenere d’occhio il buffer uso un limite massimo
#define MESSAGGI_MASSIMI 50

//definisco i vari tipi di threads
typedef enum {
    RANA,
    COCCODRILLO,
    PROIETTILE,
    GRANATA,
    PROIETTILE_TANA
} tipo;

//struttura per la posizione
struct posizione {
    int x;
    int y;
};

//descrivo un personaggio con tutto ciò che mi serve
struct personaggio {
    struct posizione posizione;
    tipo  tipo;         //rappresenta la categoria (rana, coccodrillo ecccc)
    int   lunghezza;    //sprite orizzontale
    int   id;           //identificatore univoco
    bool  destra;       //mi dice se sta andando verso destra
    pthread_t thread_id;//thread

    bool  animation;
    int offset;
};

//ogni messaggio è un personaggio
typedef struct {
    struct personaggio p;
} messaggio_t;

//struttura per il buffer circolare
typedef struct {
    messaggio_t buffer[MESSAGGI_MASSIMI]; //array dei messaggi
    int head; //testa dove produco
    int tail; //coda dove consumo
    int count; //quanti messaggi ho dentro

    //mutex per coordinare gli accessi
    pthread_mutex_t mutex;

    //condizione che segnala che non è vuoto
    pthread_cond_t not_empty;

    //condizione che segnala che non è pieno
    pthread_cond_t not_full;
} buffer_circolare;

//inizializzo il buffer, mettendo coda e testa a zero e avvio il mutex
void init_buffer(buffer_circolare *buf);

//produco un messaggio nel buffer, se è pieno aspetto
void produce_messaggio(buffer_circolare *buf, struct personaggio p);

//estraggo un messaggio in modo non bloccante; se non c’è nulla torno false
bool consume_messaggio_non_bloccante(buffer_circolare *buf, struct personaggio *p);

#endif // BUFFER_H
