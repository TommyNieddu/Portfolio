//created by Thomas Nieddu on 21/02/25.
#include "buffer.h"
#include <stdio.h>
#include <pthread.h>
extern buffer_circolare buf_principale;

//inizializzo il buffer circolare
void init_buffer(buffer_circolare *buf) {
    //imposto la testa, la coda e il count a 0 così parto pulito
    buf->head = 0;
    buf->tail = 0;
    buf->count= 0;
    //inizializzo il mutex per proteggere l’accesso
    pthread_mutex_init(&buf->mutex, NULL);
    //inizializzo le variabili di condizione per gestire spazi e messaggi
    pthread_cond_init(&buf->not_empty, NULL);
    pthread_cond_init(&buf->not_full, NULL);
}

//produco un messaggio e aspetto se il buffer è pieno
void produce_messaggio(buffer_circolare *buf, struct personaggio p) {
    //prendo il mutex
    pthread_mutex_lock(&buf->mutex);
    //finché il buffer è pieno, mi metto in attesa
    while (buf->count == MESSAGGI_MASSIMI) {
        pthread_cond_wait(&buf->not_full, &buf->mutex);
    }
    //posiziono il messaggio in testa
    buf->buffer[buf->head].p = p;
    //sposto la head in modo circolare
    buf->head = (buf->head + 1) % MESSAGGI_MASSIMI;
    //aumento il count di uno
    buf->count++;
    //avviso che c’è un messaggio disponibile
    pthread_cond_signal(&buf->not_empty);
    //lascio il mutex
    pthread_mutex_unlock(&buf->mutex);
}

//cerco di consumare un messaggio in modo non bloccante, se lo trovo ritorno true
bool consume_messaggio_non_bloccante(buffer_circolare *buf, struct personaggio *p) {
    bool trovato = false;
    //prendo il mutex
    pthread_mutex_lock(&buf->mutex);
    //se c’è almeno un messaggio, lo estraggo
    if (buf->count > 0) {
        *p = buf->buffer[buf->tail].p;
        buf->tail = (buf->tail + 1) % MESSAGGI_MASSIMI;
        buf->count--;
        //segnalo che c’è spazio libero
        pthread_cond_signal(&buf->not_full);
        trovato = true;
    }
    //rilascio il mutex
    pthread_mutex_unlock(&buf->mutex);
    return trovato;
}
