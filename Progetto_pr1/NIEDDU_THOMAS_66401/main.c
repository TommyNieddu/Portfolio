//Nome: Thomas Nieddu (t.nieddu5@studenti.unica.it)
//Matricola: 60/61/66401
//Tipologia progetto: Medio



#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "Strutture.h"
#include "MotoreGioco.h"
#include "Effetti.h"
FILE* cronologia;




int main() {
    srand(time(NULL));
    bool partitaCaricata = false;
    int n_turno = MENUMIN, conta_vincitori=INIZIO, num_personaggi=INIZIO, CarteCFU = INIZIO, n_giocatori=INIZIO;
    int* punteggioTemporaneo = (int*)calloc(n_giocatori, sizeof(int));
    Carte* carteCFU=NULL;
    Giocatore* giocatori =NULL;
    CartaOstacolo* ostacoli = NULL;
    CartaGiocata* Giocate=NULL;
    Personaggio* personaggi = (Personaggio*) malloc(sizeof(Personaggio));

    InizializzaGioco(&giocatori, &n_giocatori, &ostacoli, &carteCFU, punteggioTemporaneo, &num_personaggi, &personaggi, &CarteCFU, "personaggi.txt", "carte.txt", &partitaCaricata);

    EseguiTurno(&giocatori, &n_giocatori, &ostacoli, &carteCFU, &CarteCFU, &Giocate, punteggioTemporaneo, &conta_vincitori, &n_turno);

    TerminaCronologia();
    free(giocatori);
    free(punteggioTemporaneo);
    free(personaggi);
    free(carteCFU);
    return 0;
}
