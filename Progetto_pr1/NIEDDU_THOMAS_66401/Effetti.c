//
// Created by Thomas Nieddu on 22/02/24.
//
#include "Effetti.h"
#include <stdio.h>
#include <stdlib.h>
#include "MotoreGioco.h"
FILE* cronologia;

void EffettoScartaP(Giocatore* giocatore, int* punteggioTemporaneo, int indiceGiocatoreAttuale) {
    Trascrivi("Effetto scarta p attivato");
    if (giocatore == NULL || punteggioTemporaneo == NULL) {
        printf("Errore: Giocatore o punteggio temporaneo non valido.\n");
        return;
    }

    printf("Giocatore %s, scegli una carta CFU punto da scartare:\n", giocatore->nomeUtente);

    // mostro le carte con più di 0 cfu
    Carte* attuale = giocatore->listaCarteCFU;
    int cartaSelezionata = INIZIO;
    int cartaValida = INIZIO;
    int contatore = MENUMIN;

    while (attuale != NULL) {
        if (attuale->numeroCFU > INIZIO) {
            printf("Carta %d: %s (CFU: %d)\n", contatore, attuale->nome_carta, attuale->numeroCFU);
            cartaValida = MENUMIN;
        }
        attuale = attuale->next;
        contatore++;
    }

    if (!cartaValida) {
        printf("Non hai carte CFU punto da scartare.\n");
        return;
    }

    printf("Inserisci il numero della carta da scartare: ");
    scanf("%d", &cartaSelezionata);
    while (getchar() != '\n'); // pulisco buffer

    // cerco e rimuovoo la carta scelta
    Carte* precedente = NULL;
    attuale = giocatore->listaCarteCFU;
    contatore = MENUMIN;

    while (attuale != NULL && contatore < cartaSelezionata) {
        precedente = attuale;
        attuale = attuale->next;
        contatore++;
    }

    if (attuale != NULL && attuale->numeroCFU > INIZIO) {
        // aggiorno il punteggio
        punteggioTemporaneo[indiceGiocatoreAttuale] += attuale->numeroCFU;
        printf("Hai scartato la carta %s e aggiunto %d CFU al tuo punteggio temporaneo.\n", attuale->nome_carta, attuale->numeroCFU);

        // Rimuovo la carta dalla lista
        if (precedente == NULL) {
            giocatore->listaCarteCFU = attuale->next; //se la craata è la prima
        } else {
            precedente->next = attuale->next; // se la carta non è la prima
        }
        free(attuale);
    } else {
        printf("Selezione non valida.\n");
    }
}
void EffettoRuba(Giocatore* giocatori, int numerogiocatori, int indiceGiocatoreAttuale) {
    Trascrivi("Effetto ruba attivato");
    if (giocatori == NULL || indiceGiocatoreAttuale < INIZIO || indiceGiocatoreAttuale >= numerogiocatori) {
        printf("Errore: Parametri non validi.\n");
        return;
    }

    printf("%s, scegli il numero di un giocatore da cui rubare una carta: \n", giocatori[indiceGiocatoreAttuale].nomeUtente);
    for (int i = INIZIO; i < numerogiocatori; ++i) {
        if (i != indiceGiocatoreAttuale) {
            printf("%d: %s\n", i + MENUMIN, giocatori[i].nomeUtente);
        }
    }

    int indiceGiocatoreScelto;
    scanf("%d", &indiceGiocatoreScelto);
    indiceGiocatoreScelto -= MENUMIN;

    // controllo validità scelta
    if (indiceGiocatoreScelto < INIZIO || indiceGiocatoreScelto >= numerogiocatori || indiceGiocatoreScelto == indiceGiocatoreAttuale) {
        printf("Scelta non valida, inserisci un numero valido: ");
        return;
    }

    // mostro le carte del giocatore scelto e faccio decidere quale rubare
    int contatore = MENUMIN;
    Carte* carta = giocatori[indiceGiocatoreScelto].listaCarteCFU;
    while (carta != NULL) {
        if (carta->numeroCFU > INIZIO) { // mostro solo carte con CFU > 0
            printf("%d: %s (CFU: %d)\n", contatore, carta->nome_carta, carta->numeroCFU);
            contatore++;
        }
        carta = carta->next;
    }

    if (contatore == MENUMIN) { //carta validanon trovata
        printf("Il giocatore scelto non ha carte da rubare.\n");
        return;
    }

    printf("Scegli il numero della carta da rubare: ");
    int indiceCartaScelta;
    scanf("%d", &indiceCartaScelta);

    // cerco e rimuovo la carta  dal giocatore scelto
    Carte* precedente = NULL;
    carta = giocatori[indiceGiocatoreScelto].listaCarteCFU;
    contatore = MENUMIN; // resetto per trovare la carta
    while (carta != NULL && contatore < indiceCartaScelta) {
        precedente = carta;
        carta = carta->next;
        contatore++;
    }

    // rimuvoo dal  giocatore scelto e aggiungo al giocatore che ha attivato l'effetto
    if (carta != NULL) {
        if (precedente == NULL) { //se è la prima
            giocatori[indiceGiocatoreScelto].listaCarteCFU = carta->next;
        } else { // se non è la prima
            precedente->next = carta->next;
        }

        // aggiungo la carta rubata alla lista del giocatore che ha attivato l'effetto
        carta->next = giocatori[indiceGiocatoreAttuale].listaCarteCFU;
        giocatori[indiceGiocatoreAttuale].listaCarteCFU = carta;
        printf("Carta rubata con successo.\n");
    } else {
        printf("Selezione non valida.\n");
    }
}
void EffettoScambiaDS(Giocatore* giocatori, int n_giocatori, int indiceGiocatoreAttuale) {
    Trascrivi("Effetto scambia ds attivato");
    if (giocatori == NULL || n_giocatori <= INIZIO || indiceGiocatoreAttuale < INIZIO || indiceGiocatoreAttuale >= n_giocatori) {
        printf("Errore: Dati non validi.\n");
        return;
    }

    printf("%s, scegli un giocatore con cui scambiare una carta senza effetto (escluso te stesso):\n", giocatori[indiceGiocatoreAttuale].nomeUtente);

    for (int i = INIZIO; i < n_giocatori; i++) {
        if (i != indiceGiocatoreAttuale) {
            printf("%d: %s\n", i + MENUMIN, giocatori[i].nomeUtente);
        }
    }

    int indiceGiocatoreScelto;
    scanf("%d", &indiceGiocatoreScelto);
    indiceGiocatoreScelto -= MENUMIN;

    //controllo validità
    while (indiceGiocatoreScelto == indiceGiocatoreAttuale || indiceGiocatoreScelto < INIZIO || indiceGiocatoreScelto >= n_giocatori) {
        printf("Selezione non valida. Per favore, scegli un numero valido: ");
        scanf("%d", &indiceGiocatoreScelto);
        indiceGiocatoreScelto -= MENUMIN;
    }

    //faccio scegliere la carta appartenente al giocatore che attiva l'eeffetto
    printf("Scegli una tua carta senza effetto da scambiare:\n");
    for (int j = INIZIO; j < MANO; j++) {
        if (giocatori[indiceGiocatoreAttuale].listaCarteCFU[j].tipologia == INIZIO) {
            printf("%d: %s\n", j + MENUMIN, giocatori[indiceGiocatoreAttuale].listaCarteCFU[j].nome_carta);
        }
    }

    int cartaSceltaGiocatoreAttuale;
    scanf("%d", &cartaSceltaGiocatoreAttuale);
    cartaSceltaGiocatoreAttuale -= MENUMIN;

    // controllo che la scelta sia valida
    while (cartaSceltaGiocatoreAttuale < INIZIO || cartaSceltaGiocatoreAttuale >= MANO || giocatori[indiceGiocatoreAttuale].listaCarteCFU[cartaSceltaGiocatoreAttuale].tipologia != INIZIO) {
        printf("Selezione non valida. Per favore, scegli un numero valido: ");
        scanf("%d", &cartaSceltaGiocatoreAttuale);
        cartaSceltaGiocatoreAttuale -= MENUMIN;
    }

    // scelta della carta da scambiare dell'avversario
    printf("Scegli una carta senza effetto di %s da scambiare:\n", giocatori[indiceGiocatoreScelto].nomeUtente);
    for (int k = INIZIO; k < MANO; k++) {
        if (giocatori[indiceGiocatoreScelto].listaCarteCFU[k].tipologia == INIZIO) {
            printf("%d: %s\n", k + MENUMIN, giocatori[indiceGiocatoreScelto].listaCarteCFU[k].nome_carta);
        }
    }

    int cartaSceltaAvversario;
    scanf("%d", &cartaSceltaAvversario);
    cartaSceltaAvversario -= MENUMIN;

    // controlllo che la scelta carta sia valida
    while (cartaSceltaAvversario < INIZIO || cartaSceltaAvversario >= MANO || giocatori[indiceGiocatoreScelto].listaCarteCFU[cartaSceltaAvversario].tipologia != INIZIO) {
        printf("Selezione non valida. Per favore, scegli un numero valido: ");
        scanf("%d", &cartaSceltaAvversario);
        cartaSceltaAvversario -= MENUMIN;
    }

    // scambio effettivo
    Carte cartaTemp = giocatori[indiceGiocatoreAttuale].listaCarteCFU[cartaSceltaGiocatoreAttuale];
    giocatori[indiceGiocatoreAttuale].listaCarteCFU[cartaSceltaGiocatoreAttuale] = giocatori[indiceGiocatoreScelto].listaCarteCFU[cartaSceltaAvversario];
    giocatori[indiceGiocatoreScelto].listaCarteCFU[cartaSceltaAvversario] = cartaTemp;

    printf("Scambio completato con successo tra %s e %s.\n", giocatori[indiceGiocatoreAttuale].nomeUtente, giocatori[indiceGiocatoreScelto].nomeUtente);
}
void EffettoScartaE(Giocatore* giocatori, int indiceGiocatoreAttuale, int* punteggioTemporaneo, Carte** testaMazzoCFU) {
    if (!giocatori || indiceGiocatoreAttuale < INIZIO) {
        printf("Parametri non validi.\n");
        return;
    }

    // mostro le carte con effetti (numeroCFU > 0)
    printf("Carte con effetti disponibili per scartare:\n");
    Carte* attuale = giocatori[indiceGiocatoreAttuale].listaCarteCFU;
    int indice = INIZIO;
    while (attuale != NULL) {
        if (attuale->numeroCFU > INIZIO) {
            printf("%d. %s (CFU: %d)\n", indice + MENUMIN, attuale->nome_carta, attuale->numeroCFU);
        }
        attuale = attuale->next;
        indice++;
    }

    // chiedo quale carta scartare
    int indiceCartaScelta;
    printf("Seleziona il numero della carta da scartare: ");
    scanf("%d", &indiceCartaScelta);
    indiceCartaScelta -= MENUMIN; // //imposto a -1 così combaccia con il numero esatto della carta

    // resetto il puntatore attuale per cercare di nuovo la carta
    attuale = giocatori[indiceGiocatoreAttuale].listaCarteCFU;
    Carte* precedente = NULL;
    indice = INIZIO;

    // trovo la carta selezionata
    while (attuale != NULL && indice < indiceCartaScelta) {
        precedente = attuale;
        attuale = attuale->next;
        indice++;
    }

    if (attuale == NULL) {
        printf("Selezione non valida.\n");
        return;
    }

    // aggiungo i CFU al punteggio temporaneo del giocatore
    punteggioTemporaneo[indiceGiocatoreAttuale] += attuale->numeroCFU;

    // rimuovo la carta dalla lista del giocatore
    if (precedente == NULL) { //se la carta da scartare è la prima della lista
        giocatori[indiceGiocatoreAttuale].listaCarteCFU = attuale->next;
    } else {
        precedente->next = attuale->next;
    }

    // aggiungo la carta scartata in fondo al mazzo CFU
    Carte** codaMazzo = testaMazzoCFU;
    while (*codaMazzo != NULL) {
        codaMazzo = &((*codaMazzo)->next);
    }
    *codaMazzo = attuale;
    attuale->next = NULL; // mi assicuro che la carta aggiunta sia l'ultima

    printf("Carta scartata e CFU aggiunti al punteggio temporaneo.\n");
}
void EffettoSbircia(Giocatore* giocatoreAttuale, Carte** testaMazzo) {
    Trascrivi("Effetto sbircia attivato");
    //controllo se ci sono abbastanza carte
    if (*testaMazzo == NULL || (*testaMazzo)->next == NULL) {
        printf("Non ci sono abbastanza carte nel mazzo da sbirciare.\n");
        return;
    }

    //mostro le prime due carte
    printf("Le prime due carte del mazzo sono:\n");
    printf("1. %s\n", (*testaMazzo)->nome_carta);
    printf("2. %s\n", (*testaMazzo)->next->nome_carta);
    //------------------------------------
    fprintf(cronologia,"Le prime due carte del mazzo sono:");
    fprintf(cronologia,"1. %s", (*testaMazzo)->nome_carta);
    fprintf(cronologia,"2. %s", (*testaMazzo)->next->nome_carta);
    // chiedo quale carta vuole tenere
    int scelta;
    do {
        printf("Quale carta vuoi tenere? (1 o 2): ");
        scanf("%d", &scelta);
    } while (scelta != MENUMIN && scelta != NUMMIN);
    fprintf(cronologia,"Carta tenuta: %d",scelta);
    /*una volta presa la scelta del giocatore, assegno ai puntatori
     * cartaTenuta e cartaScartata le carte, e le rimuovo dalla cima del mazzo, la carta scartata
     * viene riaggiunta in fondo al mazzo il quale se è vuoto viene aggiunto come la prima carta
     * se no viene aggiunto dopo l'ultima carta del mazzo.*/
    // quindi gestisco i puntatori per tenere traccia della carta tenuta e di quella scartata.
    Carte* cartaTenuta;
    Carte* cartaScartata;

    //tolgo le carte dal mazzo
    if (scelta == MENUMIN) {
        cartaTenuta = *testaMazzo;
        cartaScartata = (*testaMazzo)->next;
        *testaMazzo = cartaScartata->next;  //aggiornola testa del mazzo
    } else {
        cartaTenuta = (*testaMazzo)->next;
        cartaScartata = *testaMazzo;
        *testaMazzo = cartaTenuta->next;  //aggiorno la testa del mazzo
    }
    //aggiungo la carta tenuta alla lista delle carte del giocatore attuale
    cartaTenuta->next = giocatoreAttuale->listaCarteCFU;
    giocatoreAttuale->listaCarteCFU = cartaTenuta;

    //rimetto la carta scartata in fondo al mazzo
    cartaScartata->next = NULL;
    Carte* codaMazzo = *testaMazzo;
    if (codaMazzo == NULL) {
        *testaMazzo = cartaScartata;
    } else {
        while (codaMazzo->next != NULL) {
            codaMazzo = codaMazzo->next;
        }
        codaMazzo->next = cartaScartata;
    }

    printf("Hai scelto di tenere la carta %s e rimettere la carta %s in fondo al mazzo.\n", cartaTenuta->nome_carta, cartaScartata->nome_carta);
    fprintf(cronologia,"Hai scelto di tenere la carta %s e rimettere la carta %s in fondo al mazzo.", cartaTenuta->nome_carta, cartaScartata->nome_carta);
}
void EffettoScambiaP(Giocatore* giocatori, int* punteggioTemporaneo, int n_giocatori) {
    Trascrivi("Effetto scambia p attivato");
    int indiceMax = INIZIO;
    int indiceMin = INIZIO;
    int maxPunteggio = punteggioTemporaneo[INIZIO];
    int minPunteggio = punteggioTemporaneo[INIZIO];

    // cerco l'indice del punteggio massimo e minimo
    for (int i = MENUMIN; i < n_giocatori; i++) {
        // se il punteggio del giocatore attuale è più alto del max corrente
        // allora aggiorno le variabili indiceMax e maxPunteggio
        if (punteggioTemporaneo[i] > maxPunteggio) {
            indiceMax = i;
            maxPunteggio = punteggioTemporaneo[i];
        }
        // se il punteggio del giocatore attuale è più basso del min corrente
        // allora aggiorno le variabili indiceMin e minPunteggio
        if (punteggioTemporaneo[i] < minPunteggio) {
            indiceMin = i;
            minPunteggio = punteggioTemporaneo[i];
        }
    }

    // scambio i punteggi
    punteggioTemporaneo[indiceMax] = minPunteggio;
    punteggioTemporaneo[indiceMin] = maxPunteggio;

    printf("%s ha scambiato il suo punteggio di %d con il punteggio di %d di %s.\n",giocatori[indiceMax].nomeUtente,minPunteggio,maxPunteggio,giocatori[indiceMin].nomeUtente);
    fprintf(cronologia,"%s ha scambiato il suo punteggio di %d con il punteggio di %d di %s.",giocatori[indiceMax].nomeUtente,minPunteggio,maxPunteggio,giocatori[indiceMin].nomeUtente);

}
void EffettoScambiaCFU(Giocatore* giocatori, int numerogiocatori) {
    Trascrivi("Effetto scambia cfu attivato");
    int indiceGiocatore1, indiceGiocatore2;
    int indiceCartaGiocatore1, indiceCartaGiocatore2;

    // chiedo al giocatore di scegliere due giocatori
    printf("Scegli due giocatori per scambiare una carta CFU tra loro.\n");
    for (int i = INIZIO; i < numerogiocatori; i++) {
        printf("%d: %s\n", i + MENUMIN, giocatori[i].nomeUtente);
    }
    printf("Inserisci il numero del primo giocatore: ");
    scanf("%d", &indiceGiocatore1);
    fprintf(cronologia,"Primo giocatore per lo scambio: %d",indiceGiocatore1);
    indiceGiocatore1--;
    printf("Inserisci il numero del secondo giocatore: ");
    scanf("%d", &indiceGiocatore2);
    fprintf(cronologia,"Secondo giocatore per lo scambio: %d",indiceGiocatore1);
    indiceGiocatore2--;

    // chiedo al giocatore di scegliere una carta  per ogni giocatore
    printf("Scegli una carta CFU per %s:\n", giocatori[indiceGiocatore1].nomeUtente);
    for (int i = INIZIO; i < MANO; i++) {
        if (giocatori[indiceGiocatore1].listaCarteCFU[i].numeroCFU > INIZIO) {
            printf("%d: %s (CFU: %d)\n", i + MENUMIN, giocatori[indiceGiocatore1].listaCarteCFU[i].nome_carta, giocatori[indiceGiocatore1].listaCarteCFU[i].numeroCFU);
        }
    }
    printf("Inserisci il numero della carta CFU da scambiare: ");
    scanf("%d", &indiceCartaGiocatore1);
    fprintf(cronologia,"Prima carta: %d",indiceGiocatore1);
    indiceCartaGiocatore1--;

    printf("Scegli una carta CFU per %s:\n", giocatori[indiceGiocatore2].nomeUtente);
    for (int i = INIZIO; i < MANO; i++) {
        if (giocatori[indiceGiocatore2].listaCarteCFU[i].numeroCFU > INIZIO) {
            printf("%d: %s (CFU: %d)\n", i + MENUMIN, giocatori[indiceGiocatore2].listaCarteCFU[i].nome_carta, giocatori[indiceGiocatore2].listaCarteCFU[i].numeroCFU);
        }
    }
    printf("Inserisci il numero della carta CFU da scambiare: ");
    scanf("%d", &indiceCartaGiocatore2);
    fprintf(cronologia,"Seconda carta: %d",indiceGiocatore1);
    indiceCartaGiocatore2--;

    // Scambio tra i due giocatori le carte
    Carte temporanea = giocatori[indiceGiocatore1].listaCarteCFU[indiceCartaGiocatore1];
    giocatori[indiceGiocatore1].listaCarteCFU[indiceCartaGiocatore1] = giocatori[indiceGiocatore2].listaCarteCFU[indiceCartaGiocatore2];
    giocatori[indiceGiocatore2].listaCarteCFU[indiceCartaGiocatore2] = temporanea;

    printf("Le carte sono state scambiate con successo!\n");
    fprintf(cronologia,"Le carte sono state scambiate con successo!");

}
void EffettoScartaCarte(Giocatore* giocatore) {
    Trascrivi("Effetto scarta carte attivato");

    if (giocatore == NULL || giocatore->listaCarteCFU == NULL) {
        printf("Errore: giocatore o mano del giocatore non valida.\n");
        exit(-MENUMIN);
    }

    int numCarteDaScartare;
    printf("Quante carte vuoi scartare? (1-3): ");
    scanf("%d", &numCarteDaScartare);
    fprintf(cronologia, "Numero di carte scelte da scartare: %d", numCarteDaScartare);

    if (numCarteDaScartare < MENUMIN || numCarteDaScartare > MENUMAX) {
        printf("Numero non valido.\n");
        exit(-MENUMIN);
    }

    for (int i = INIZIO; i < numCarteDaScartare; i++) {
        int posizione = MENUMIN;
        printf("Scegli una carta da scartare:\n");
        Carte* attuale = giocatore->listaCarteCFU;
        while (attuale != NULL) {
            printf("%d: %s\n", posizione, attuale->nome_carta);
            attuale = attuale->next;
            posizione++;
        }

        int scelta, valido = INIZIO;
        do {
            scanf("%d", &scelta);
            if (scelta >= MENUMIN && scelta < posizione) {
                valido = MENUMIN;
            } else {
                printf("Scelta non valida, scegli un numero valido:\n");
            }
        } while (valido == INIZIO);

        //tolgo la carta scelta dalla lista
        Carte* precedente = NULL;
        attuale = giocatore->listaCarteCFU;
        for (int j = MENUMIN; j < scelta; j++) {
            precedente = attuale;
            attuale = attuale->next;
        }

        if (precedente == NULL) { // se la carta è prima
            giocatore->listaCarteCFU = attuale->next;
        } else {
            precedente->next = attuale->next; //se non è prima
        }


        printf("Hai scartato la carta %s.\n", attuale->nome_carta);
        fprintf(cronologia, "Hai scartato la carta %s.", attuale->nome_carta);
        free(attuale);
    }
}
void EffettoAnnulla(CartaGiocata* carteGiocate, int* punteggioTemporaneo, int numerogiocatori, int indiceTipoOstacolo) {
    Trascrivi("Effetto annulla attivato");
    for (int i = INIZIO; i < numerogiocatori; i++) {
        // conservo i cfu della carta giocata del giocatore attuale
        int valoreCFUCartaGiocata = carteGiocate[i].carta.numeroCFU;

        // conservo i bonus e malus del personaggio
        int modificatorePersonaggio = carteGiocate[i].giocatore->personaggio.BonusMalus[indiceTipoOstacolo];

        // calcolo il punteggio temporaneo
        punteggioTemporaneo[i] = valoreCFUCartaGiocata + modificatorePersonaggio;
    }
    printf("Gli effetti di tutte le carte punto sono stati annullati e i punteggi sono stati ripristinati ai valori di inizio turno.\n");
    Trascrivi("Gli effetti di tutte le carte punto sono stati annullati e i punteggi sono stati ripristinati ai valori di inizio turno.");

}
void EffettoAumenta(Giocatore* giocatori, int numerogiocatori, int* punteggioTemporaneo) {
    Trascrivi("Effetto aumenta attivato");
    printf("Scegli un giocatore a cui aumentare di 2 CFU:\n");
    for (int i = INIZIO; i < numerogiocatori; i++) {
        printf("%d: %s\n", i + MENUMIN, giocatori[i].nomeUtente);
    }

    int sceltaGiocatore;
    scanf("%d", &sceltaGiocatore);
    sceltaGiocatore--;

    if (sceltaGiocatore >= INIZIO && sceltaGiocatore < numerogiocatori) {
        punteggioTemporaneo[sceltaGiocatore] += NUMMIN;
        printf("%s ha ora %d CFU.\n", giocatori[sceltaGiocatore].nomeUtente, punteggioTemporaneo[sceltaGiocatore]);
        fprintf(cronologia,"%s ha ora %d CFU.", giocatori[sceltaGiocatore].nomeUtente, punteggioTemporaneo[sceltaGiocatore]);

    } else {
        printf("Scelta non valida.\n");
    }
}
void EffettoDiminuisci(Giocatore* giocatori, int numerogiocatori, int* punteggioTemporaneo) {
    Trascrivi("Effetto diminuisci attivato");
    printf("Scegli un giocatore a cui diminuire di 2 CFU:\n");
    for (int i = INIZIO; i < numerogiocatori; i++) {
        printf("%d: %s\n", i + MENUMIN, giocatori[i].nomeUtente);
    }

    int sceltaGiocatore;
    scanf("%d", &sceltaGiocatore);
    sceltaGiocatore--;

    if (sceltaGiocatore >= INIZIO && sceltaGiocatore < numerogiocatori) {
        punteggioTemporaneo[sceltaGiocatore] -= NUMMIN;
        printf("%s ha ora %d CFU.\n", giocatori[sceltaGiocatore].nomeUtente, punteggioTemporaneo[sceltaGiocatore]);
        fprintf(cronologia,"%s ha ora %d CFU.", giocatori[sceltaGiocatore].nomeUtente, punteggioTemporaneo[sceltaGiocatore]);

    } else {
        printf("Scelta non valida.\n");
    }
}
void EffettoInverti(int* punteggioTemporaneo,int numerogiocatori){
    Trascrivi("Effetto inverti attivato");
    if (numerogiocatori < NUMMIN) {
        printf("Numero di giocatori insufficiente per attivare l'effetto INVERTI.\n");
        exit(-MENUMIN);
    }

    int max = INIZIO, min = INIZIO; //  punteggio massimo e minimo
    for (int i = MENUMIN; i < numerogiocatori; i++) {
        if (punteggioTemporaneo[i] > punteggioTemporaneo[max]) {
            max = i;
        }
        if (punteggioTemporaneo[i] < punteggioTemporaneo[min]) {
            min = i;
        }
    }


    int appoggio = punteggioTemporaneo[max];
    punteggioTemporaneo[max] = punteggioTemporaneo[min];
    punteggioTemporaneo[min] = appoggio;

    printf("Punteggio massimo e minimo invertiti con successo.\n");
    Trascrivi("Punteggio massimo e minimo invertiti con successo.");
}
void EffettoSalva(CartaOstacolo** testaMazzoOstacoli, CartaOstacolo* cartaEstratta) {
    Trascrivi("Effetto salvta attivato");
    printf("Hai attivato l'effetto SALVA.\n");
    if (cartaEstratta == NULL || testaMazzoOstacoli == NULL) {
        printf("Nessuna carta ostacolo da rimettere nel mazzo o mazzo non valido.\n");
        exit(-MENUMIN);
    }
    AggiungiInFondo(testaMazzoOstacoli, cartaEstratta);

    printf("L'effetto SALVA è stato attivato.\n");
}
void EffettoDirotta(Giocatore* giocatori, int numerogiocatori, CartaOstacolo* cartaDaDirottare, int indiceGiocatoreCorrente) {
    Trascrivi("Effetto dirotta attivato");
    printf("Hai attivato l'effetto DIROTTA.\n");

    printf("Scegli a quale giocatore dirottare la carta ostacolo:\n");
    for (int i = INIZIO; i < numerogiocatori; i++) {
        if (i != indiceGiocatoreCorrente) {
            printf("%d: %s\n", i + MENUMIN, giocatori[i].nomeUtente);
        }
    }

    int sceltaGiocatore;
    do {
        printf("Inserisci il numero del giocatore: ");
        scanf("%d", &sceltaGiocatore);
        sceltaGiocatore--;
    } while (sceltaGiocatore == indiceGiocatoreCorrente || sceltaGiocatore < INIZIO || sceltaGiocatore >= numerogiocatori);
    Giocatore* destinatario = &giocatori[sceltaGiocatore];

    AssegnaCartaOstacoloAGiocatore(destinatario, cartaDaDirottare);

    printf("La carta ostacolo '%s' è stata dirottata a %s.\n", cartaDaDirottare->nome_carta_ostacolo, destinatario->nomeUtente);
    fprintf(cronologia,"La carta ostacolo '%s' è stata dirottata a %s.", cartaDaDirottare->nome_carta_ostacolo, destinatario->nomeUtente);

}