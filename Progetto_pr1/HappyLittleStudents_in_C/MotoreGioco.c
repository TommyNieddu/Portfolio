//
// Created by Thomas Nieddu on 22/02/24.
//
#include "MotoreGioco.h"
#include "Strutture.h"
#include "Effetti.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

FILE* cronologia;

void Menu(Giocatore** giocatori, int* n_giocatori, CartaOstacolo** ostacoli, Carte** carteCFU, bool* partitaCaricata) {
    int scelta;
    printf("\n");
    do {
        printf("\n");
        printf(TESTO_GIALLO);
        printf("==== Menu del Gioco ====\n");
        printf("\033[0m");
        printf("[1] Inizia nuova partita\n");
        printf("[2] Carica salvataggio\n");
        printf("[3] Esci\n");
        printf("\033[0;33m");
        printf("=========================\n");
        printf(TESTO_STANDARD);
        printf("Scegli un' opzione: ");
        scanf("%d", &scelta);
        if(scelta < MENUMIN || scelta > MENUMAX){
            printf("\n\nHai scelto un'opzione non valida\n");
        }
    } while (scelta < MENUMIN || scelta > MENUMAX);
    switch(scelta) {
        case MENUMIN:
            PuliziaTerminale();
            printf(TESTO_CIANO);
            printf("\nHai scelto 'Inizia nuova partita'\n");
            printf(TESTO_STANDARD);
            Trascrivi("Iniziata una nuova partita");
            break;
        case NUMMIN:
            PuliziaTerminale();
            printf("\nHai scelto 'Carica salvataggio'\n");
            Trascrivi("Caricata una partita precedente");
            CaricaPartita(giocatori, n_giocatori);
            *partitaCaricata = true;
            break;
        case MENUMAX:
            PuliziaTerminale();
            printf(TESTO_ROSSO);
            printf("\nCHIUSURA GIOCO.\n");
            Trascrivi("Gioco chiuso");
            exit(-MENUMIN);
    }
}
int richiestaNumeroGiocatori() {
    int n_giocatori;
    do {
        printf(TESTO_GIALLO"Inserisci numero giocatori (da 2 a 4):\t");
        printf(TESTO_STANDARD);
        scanf("%d", &n_giocatori);
    } while(n_giocatori < NUMMIN || n_giocatori > NUMMAX);
    fprintf(cronologia, "I giocatori sono %d\n", n_giocatori);
    return n_giocatori; //restituisco il numero di giocatori
}
Giocatore* allocaGiocatore(int i) {
    Giocatore* prossimoGiocatore = malloc(sizeof(Giocatore));
    if (prossimoGiocatore == NULL) {
        printf("Errore durante l'allocazione di memoria.\n");
        exit(-MENUMIN);
    }

    prossimoGiocatore->nomeUtente = malloc(DIM * sizeof(char));
    if (prossimoGiocatore->nomeUtente == NULL) {
        printf("Errore durante l'allocazione di memoria per il nome del giocatore.\n");
        free(prossimoGiocatore);
        exit(-MENUMIN);
    }

    printf("\nInserisci il nome del giocatore %d: ", i + MENUMIN);
    scanf(" %31s", prossimoGiocatore->nomeUtente);
    fprintf(cronologia, "Giocatore inserito:\t%s\n", prossimoGiocatore->nomeUtente);

    prossimoGiocatore->numeroCFU = INIZIO;
    prossimoGiocatore->listaCarteCFU = NULL;
    prossimoGiocatore->listaCarteOstacolo = NULL;
    prossimoGiocatore->next = NULL;
    return prossimoGiocatore;
}
Giocatore* allocaGiocatori(int numerogiocatori) {
    if (numerogiocatori <= INIZIO) {
        printf("Il numero di giocatori deve essere maggiore di 0.\n");
        exit(-MENUMIN);
    }

    Giocatore* testa = NULL;
    Giocatore* attuale = NULL;
    /*Se è il primo giocatore creato viene messo come testa della lista e il riferimento all' attuale diventa quello
     Se invece la lista contiene già almeno un elemento, il giocatore viene aggiunto come next del gioco precedente e il
     riferimento all'elemento attuale viene aggiornato al nuovo giovocatore.
     Infine ritorna la testa della lista, ovvero il riferimento al primo giocatore nella lista creata.*/
    for (int i = INIZIO; i < numerogiocatori; i++) {
        Giocatore* nuovo_giocatore = allocaGiocatore(i);

        if (testa == NULL) {
            testa = nuovo_giocatore;
            attuale = testa;
        } else {
            attuale->next = nuovo_giocatore;
            attuale = attuale->next;
        }
    }

    return testa;
}
Personaggio* leggipersonaggi(const char* fp, int* num_personaggi) {
    Trascrivi("Lettura file personaggi");
    FILE* file = fopen(fp, "r");
    if (fp == NULL) {
        printf(TESTO_ROSSO);
        printf("Impossibile aprire il file!\n");
        printf(TESTO_STANDARD);
        exit(-MENUMIN);
    }
    Personaggio* personaggi = NULL;
    char nome[DIM];
    int bonusMalus[NUMMAX];

    while (fscanf(file, "%d%d%d%d%s", &bonusMalus[INIZIO], &bonusMalus[MENUMIN], &bonusMalus[NUMMIN], &bonusMalus[MENUMAX], nome) == NUMMAX + MENUMIN) {
        /*creo un nuovo personaggio copiando il nome letto
         * dal file e gli elementi del bonus mlus. Dopo di ciò utilizzo realloc per ridimensionare
         * l'array di personaggi e fatto ciò, inserisce il nuovo personaggio all'ultimo posto
         * libero dell'array personaggi, quindi incrementa il conteggio dei personaggi.*/
        Personaggio nuovo_personaggio;
        strcpy(nuovo_personaggio.nome, nome);
        for (int i =INIZIO; i < NUMMAX; i++) {
            nuovo_personaggio.BonusMalus[i] = bonusMalus[i];
        }
        personaggi = realloc(personaggi, (*num_personaggi+MENUMIN)*sizeof(Personaggio));
        personaggi[*num_personaggi] = nuovo_personaggio;
        (*num_personaggi)++; //così ne tengo traccia al di fuori della funzione
    }
    fclose(file);
    return personaggi;
}
Carte* leggicarteCFU(const char* fp, int* numerocarte){
    Trascrivi("Lettura mazzo");
    FILE* file = fopen(fp, "r");
    if (!file) {
        printf("Non posso aprire il file %s\n", fp);
        exit(-MENUMIN);
    }
    Carte* carteCFU = malloc(MAZZOCFU * sizeof(Carte));
    while ((*numerocarte < MAZZOCFU) && (fscanf(file, "%d %d %d %[^\n]s",&carteCFU[*numerocarte].occorrenza,&carteCFU[*numerocarte].tipologia,&carteCFU[*numerocarte].numeroCFU,carteCFU[*numerocarte].nome_carta) == NUMMAX)) {
        (*numerocarte)++; // incremento il conteggio delle carte, incremento col puntatore così ne tengo traccia anche al di fuori
    }
    fclose(file);
    return carteCFU;
}
void RandomizzaPersonaggi(Personaggio* personaggi, int numeropersonaggi){
    Trascrivi("Personaggi mescolati");
    /*faccio partire il ciclo dall'ultimo personaggio, do un valore casuale a j cosi con la
     * variabile di appoggio riesco a scambiare i posti dei personaggi*/
    for (int i=(numeropersonaggi-MENUMIN) ; i > INIZIO ; i--) {
        int j = rand() % (i+MENUMIN);
        Personaggio appoggio = personaggi[i];
        personaggi[i] = personaggi[j];
        personaggi[j] = appoggio;
    }
}
void RandomizzaMazzo(Carte* mazzo, int numerocarte) {
    Trascrivi("Mazzo di carte mescolato");
    /*lo stesso per i personaggi parto dall'ultima carta e la scambio con la j-esima
     * casuale*/
    for (int i =numerocarte-MENUMIN; i > INIZIO; i--) {
        int j = rand() % (i+MENUMIN);
        Carte scambio = mazzo[i];
        mazzo[i] = mazzo[j];
        mazzo[j] = scambio;
    }
}
void AssegnazionePersonaggi(Giocatore* giocatori, Personaggio* personaggi, int numerogiocatori){
    Giocatore* attuale = giocatori; //attuale per conservare il giocatore corrente nella lista di giocatori
    for(int i =INIZIO; i < numerogiocatori && attuale != NULL; i++) {
        printf(TESTO_CIANO);
        //--------------------------------------
        printf("\nAvviso: Giocatore %s riceve il personaggio %s", attuale->nomeUtente, personaggi[i].nome);
        //--------------------------------------
        printf(TESTO_STANDARD);
        fprintf(cronologia, "\nAvviso: Giocatore %s riceve il personaggio %s", attuale->nomeUtente, personaggi[i].nome);
        attuale->personaggio = personaggi[i];
        attuale = attuale->next; //sposto il puntatore attuale al giocatore successivo nella lista
    }
}
void AssegnazioneCarteCfu(Giocatore* giocatori, int numerogiocatori, Carte* mazzo, int numerocarte) {
    // controllo ci siano carte per tutti i giocatori
    if (numerocarte < numerogiocatori * MANO) {
        printf("Disponibilita' carte inferiore.\n");
        return;
    }

    // mescolo le carte
    RandomizzaMazzo(mazzo, numerocarte);

    // assegno le carte ai giocatori
    for (int j = INIZIO; j < MANO; j++) {
        Giocatore* attuale = giocatori;
        for (int i = INIZIO; i < numerogiocatori && attuale != NULL; i++) {
            if (j == INIZIO) {
                attuale->listaCarteCFU = malloc(MANO * sizeof(Carte));
                if (attuale->listaCarteCFU == NULL) {
                    printf("Errore di allocazione della memoria.\n");
                    return;
                }
            }

            int indiceCarta = rand() % numerocarte;
            int tentativi = INIZIO;
            while (mazzo[indiceCarta].occorrenza <= INIZIO && tentativi < numerocarte) {
                indiceCarta = (indiceCarta + MENUMIN) % numerocarte;
                tentativi++;
            }

            if (mazzo[indiceCarta].occorrenza > INIZIO) {
                attuale->listaCarteCFU[j] = mazzo[indiceCarta];
                mazzo[indiceCarta].occorrenza--;
            } else {
                printf("Non è stato possibile assegnare una carta valida al giocatore %s.\n", attuale->nomeUtente);
            }

            attuale = attuale->next;
        }
    }

    // controllo se un giocatore ha 5 carte con 0 CFU e gli cambio le carte
    Giocatore* attuale = giocatori;
    for (int i = INIZIO; i < numerogiocatori && attuale != NULL; i++) {
        int carteZeroCFU = 0;
        for (int j = INIZIO; j < MANO; j++) {
            if (attuale->listaCarteCFU[j].numeroCFU == INIZIO) {
                carteZeroCFU++;
            }
        }
        if (carteZeroCFU == MANO) {
            RiassegnaCarte(attuale, mazzo, numerocarte);
        }
        attuale = attuale->next;
    }
}

void stampa_giocatori(Giocatore* lista, int numerogiocatori) {
    Trascrivi("Stampa dei giocatori");
    Giocatore* attuale = lista;
    int i = MENUMIN;
    while(attuale != NULL && i <= numerogiocatori) {
        printf(TESTO_VERDE);
        printf("\n\n");
        printf("=======================\n");
        printf("       GIOCATORE %d\n", i);
        printf("=======================\n");
        printf(TESTO_STANDARD);
        printf("Nome scelto:\t%s\n", attuale->nomeUtente);
        printf("Personaggio ottenuto:\t%s\n", attuale->personaggio.nome);
        printf("Bonus/Malus del personaggio ottenuto:\t");
        for(int j =INIZIO; j < NUMMAX; j++) {
            printf("\t%d\t", attuale->personaggio.BonusMalus[j]);
        }
        printf(TESTO_VERDE);
        printf("\n-------------------\n\n");
        printf(TESTO_STANDARD);
        attuale = attuale->next;
        i++;
    }
}
void AssegnaCartaOstacoloAGiocatore(Giocatore* destinatario, CartaOstacolo* cartaDaAssegnare) {
    CartaOstacolo* nuovaCarta = (CartaOstacolo*)malloc(sizeof(CartaOstacolo));
    if (nuovaCarta==NULL) {
        printf("Errore di allocazione della memoria.\n");
        exit(-MENUMIN);
    }
    /*copio i dati dalla carta da assegnare alla nuova carta quindi
     * copio di tutti i campi della struttura*/

    *nuovaCarta = *cartaDaAssegnare;

    /*metto il campo next del nuovo nodo per puntare al nodo che
    era all'inizio della lista. destinatario->listaCarteOstacolo è
    la testa quidni  il primo nodo della lista di carte ostacolo. ora  la nuova carta
    punta alla vecchia testa .*/
    nuovaCarta->next = destinatario->listaCarteOstacolo;
    /*modifico il puntatore alla testa per farlo puntare alla nuova carta.
     * quindi  ora la nuova testa della lista è nuova carta".*/
    destinatario->listaCarteOstacolo = nuovaCarta;
}
void AggiungiCartaInFondoAlMazzo(Carte** testaMazzo, Carte cartaGiocata) {
    // Crea una nuova istanza di Carte
    Carte* nuovaCarta = (Carte*)malloc(sizeof(Carte));
    if (!nuovaCarta) {
        printf("Errore nell'allocazione della memoria.\n");
        exit(-MENUMIN);
    }
    *nuovaCarta = cartaGiocata; // copio i dati della carta
    nuovaCarta->next = NULL;

    // trovo la fine della lista e ci metto il nuovo nodo
    /*controllose il mazzo è vuoto,se lo è  aggiungo
     * la nuova carta come l'unica carta del mazzo. Se invece il mazzo non è vuoto, cerco
     * l'ultima carta del mazzo e ci collego la nuova carta*/
    if (*testaMazzo == NULL) {
        *testaMazzo = nuovaCarta;
    } else {
        Carte* temp = *testaMazzo;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = nuovaCarta;
    }
}
void MostraManoGiocatori(Giocatore* lista, int numerogiocatori) {
    Giocatore* attuale = lista;
    int i = MENUMIN;
    while(attuale != NULL && i <= numerogiocatori) {
        printf("\n\n");
        printf(TESTO_MAGENTA);
        printf("=======================\n");
        printf("Carte CFU Giocatore %d\n", i);
        printf("=======================\n");
        printf(TESTO_STANDARD);
        printf("Nome giocatore: %s\n", attuale->nomeUtente);
        for (int j = INIZIO; j < MANO; j++) {
            printf("Carta %d: %s\n", j+MENUMIN, attuale->listaCarteCFU[j].nome_carta);
            printf("Tipologia: %d, Valore: %d\n", attuale->listaCarteCFU[j].tipologia, attuale->listaCarteCFU[j].numeroCFU);
            fprintf(cronologia,"Carta %d: %s\n", j+MENUMIN, attuale->listaCarteCFU[j].nome_carta);
            fprintf(cronologia,"Tipologia: %d, Valore: %d\n", attuale->listaCarteCFU[j].tipologia, attuale->listaCarteCFU[j].numeroCFU);
            printf("\n");
        }
        attuale = attuale->next;
        i++;
    }
}
CartaOstacolo* LeggiOstacoli(const char* fp) {
    Trascrivi("Lettura ostacoli");
    FILE *file = fopen(fp, "r");
    if (file == NULL) {
        printf(TESTO_ROSSO);
        printf("Impossibile aprire il file %s\n", fp);
        exit(-MENUMIN);
    }
    CartaOstacolo* testa = NULL;
    CartaOstacolo* coda = NULL;
    int numeroostacoli;
    int tipoOstacolo = -MENUMIN;

    while (fscanf(file, "%d", &numeroostacoli) == MENUMIN) {
        tipoOstacolo++;  // passa al tipo di ostacolo successivo
        for (int i = INIZIO; i < numeroostacoli; i++) {
            CartaOstacolo *ostacolo = malloc(sizeof(CartaOstacolo));
            ostacolo->next = NULL;
            fscanf(file, " %[^\n]s", ostacolo->nome_carta_ostacolo);
            fscanf(file, " %[^\n]s", ostacolo->descrizione_ostacolo);
            ostacolo->tipoOstacolo = tipoOstacolo;  // assegna il tipo di ostacolo attuale

            /* imposto testa a puntare al nuovo ostacolo, altrimenti, fa puntare il campo next
             * dell'ostacolo attualmente indicato da coda al nuovo ostacolo.*/
            if (testa == NULL) {
                testa = ostacolo;
            } else {
                coda->next = ostacolo;
            }
            coda = ostacolo;
        }
    }
    fclose(file);
    return testa;
}
CartaOstacolo* EstrazioneOstacolo(CartaOstacolo* testa){
    Trascrivi("Estrazione ostacolo...");
    if (testa == NULL) {
        return NULL;
    }
    int conteggio = INIZIO;
    CartaOstacolo* scambio = testa;
    while (scambio != NULL){
        conteggio++;
        scambio = scambio->next;
    }
    int elementoCasuale = rand() % conteggio;
    scambio = testa;
    /*ripristino scambio alla testa della lista e attraversa la lista tante volte quanto vale
     * elemento casuale, dopo che quel valore è decrementato a 0, il puntatore scambio punta
     * all'ostacolo casuale scelto e viene restituito.*/
    while(elementoCasuale--){
        scambio = scambio->next;
    }

    return scambio;
}
void StampaOstacoloEstratto(CartaOstacolo* ostacolo){
    char* tipologia = NULL;
    switch (ostacolo->tipoOstacolo) {
        case STUDIO:
            tipologia = "STUDIO";
            break;
        case SOPRAVVIVENZA:
            tipologia = "SOPRAVVIVENZA";
            break;
        case SOCIALE:
            tipologia = "SOCIALE";
            break;
        case ESAME:
            tipologia = "ESAME";
            break;
    }
    printf("\n");
    printf(TESTO_GIALLO);
    printf("=========================================\n");
    printf("           OSTACOLO ESTRATTO :\n");
    printf("=========================================\n");
    printf(TESTO_STANDARD);
    printf("Titolo: %s\n", ostacolo->nome_carta_ostacolo);
    printf("Descrizione: %s\n", ostacolo->descrizione_ostacolo);
    printf("Tipologia: %s\n", tipologia);
    //----------------------------------
    fprintf(cronologia,"Titolo: %s\n", ostacolo->nome_carta_ostacolo);
    fprintf(cronologia,"Descrizione: %s\n", ostacolo->descrizione_ostacolo);
    fprintf(cronologia,"Tipologia: %s\n", tipologia);
    printf(TESTO_GIALLO);
    printf("=========================================\n");
    printf(TESTO_STANDARD);
    printf("\n");
}
void RiassegnaCarte(Giocatore* giocatore, Carte* mazzo, int numerocarte) {
    Trascrivi("Riassegnazione carte...");
    if (giocatore == NULL || mazzo == NULL) {
        printf("Errore: Giocatore o mazzo non valido.\n");
        return;
    }

    if (giocatore->listaCarteCFU != NULL) {
        free(giocatore->listaCarteCFU);
    }

    giocatore->listaCarteCFU = malloc(MANO * sizeof(Carte));
    if (giocatore->listaCarteCFU == NULL) {
        printf("Errore nell'allocazione di memoria per le nuove carte.\n");
        exit(-MENUMIN);
    }

    for (int i = INIZIO; i < MANO; i++) {
        int indiceCarta = rand() % numerocarte;
        // cerco una carta valida con occorrenza maggiore di zero
        int tentativi = INIZIO;
        while ((mazzo[indiceCarta].occorrenza <= INIZIO || mazzo[indiceCarta].numeroCFU <= INIZIO) && tentativi < numerocarte * NUMMIN) {
            indiceCarta = (indiceCarta + MENUMIN) % numerocarte;
            tentativi++;
        }

        // controllo se è stata trovata una carta valida
        if (mazzo[indiceCarta].occorrenza > INIZIO && mazzo[indiceCarta].numeroCFU > INIZIO) {
            // do la carta al giocatore e decrementa l'occorrenza
            giocatore->listaCarteCFU[i] = mazzo[indiceCarta];
            mazzo[indiceCarta].occorrenza--;
        } else {
            // se non è stata trovata una carta valida
            printf("Impossibile assegnare una carta valida al giocatore %s.\n", giocatore->nomeUtente);
            free(giocatore->listaCarteCFU);
            giocatore->listaCarteCFU = NULL;
            return;
        }
    }
}
void StampaCFU(Giocatore* testa, int numerogiocatori) {
    /*seconda voce del menu giocatore, permette di stampare la situazione cfu degli avversari*/
    Giocatore* attuale = testa;
    for (int i = INIZIO; i < numerogiocatori; ++i) {
        if (attuale == NULL) {
            printf("Errore: numero giocatori insufficiente.\n");
            return;
        }
        printf("%s ha %d CFU.\n", attuale->nomeUtente, attuale->numeroCFU);
        attuale = attuale->next;
    }
}

void RinunciaAgliStudi(Giocatore** attuale, Giocatore** testa) {
    // Implementazione fittizia
    printf(TESTO_ROSSO);
    printf("===============================\n");
    printf("=   Giocatore %s              =\n", (*attuale)->nomeUtente);
    printf("=   Ha rinunciato agli studi  =\n");
    printf("===============================\n");
    printf(TESTO_STANDARD);    if (*attuale == *testa) {
        *testa = (*attuale)->next;
    } else {
        Giocatore* temp = *testa;
        while (temp && temp->next != *attuale) temp = temp->next;
        if (temp) temp->next = (*attuale)->next;
    }
    Giocatore* toDelete = *attuale;
    *attuale = (*attuale)->next;
    free(toDelete);
}
CartaGiocata giocaCarta(Giocatore* attuale, int i, CartaGiocata* carteGiocate) {
    int n_carta;
    do {
        printf("Scegli una carta da giocare (1-5): ");
        scanf("%d", &n_carta);
        while (getchar() != '\n');  // pulisco buff
        n_carta--; // converto per l'indice del vettoree
        if(n_carta >= INIZIO && n_carta < MANO && attuale->listaCarteCFU[n_carta].numeroCFU <= INIZIO) {
            printf(TESTO_MAGENTA "Puoi giocare solo carte punto CFU in questo momento.\n" TESTO_STANDARD);
        }
    } while (n_carta < INIZIO || n_carta >= MANO || attuale->listaCarteCFU[n_carta].numeroCFU <= INIZIO);

    Carte selezionata = attuale->listaCarteCFU[n_carta];
    printf("Carta giocata:\t%s\nTipo:\t%d\nDa:\t%d CFU\n", selezionata.nome_carta, selezionata.tipologia, selezionata.numeroCFU);

    // tolgo la carta dalla lista
    for (int j = n_carta; j < MANO - MENUMIN; j++) {
        attuale->listaCarteCFU[j] = attuale->listaCarteCFU[j + MENUMIN];
    }

    // salvo la carta giocata
    carteGiocate[i].giocatore = attuale;
    carteGiocate[i].carta = selezionata;
    return carteGiocate[i];
}
int EseguiScelta(int scelta, Giocatore** attuale, Giocatore** testa, int* numerogiocatori,  CartaGiocata* carteGiocate, int *i) {
    switch (scelta) {
        case MENUMIN:
            carteGiocate[*i] = giocaCarta(*attuale, *i, carteGiocate);
            (*i)++;
            return INIZIO;
        case NUMMIN: // mostro resoconto punti
            StampaCFU(*testa, *numerogiocatori);
            return MENUMIN;
        case MENUMAX: // Rage quitta
            RinunciaAgliStudi(attuale, testa);
            (*numerogiocatori)--;
            if (*numerogiocatori < NUMMIN) {
                free(carteGiocate);
                return -MENUMIN;
            }
            return INIZIO;
    }
    return INIZIO;
}

CartaGiocata* Giocata(Giocatore* testa, int* numerogiocatori, Carte* mazzo, int numerocarte) {
    if (testa == NULL) {
        printf("Errore: Non ci sono giocatori.\n");
        return NULL;
    }

    CartaGiocata* carteGiocate = (CartaGiocata*)calloc(*numerogiocatori, sizeof(CartaGiocata));
    if (carteGiocate == NULL) {
        printf("Errore nell'allocazione di memoria per le carte giocate.\n");
        return NULL;
    }

    Giocatore* attuale = testa;
    int i = INIZIO;
    while (attuale != NULL && *numerogiocatori >= NUMMIN) {
        int scelta;
        int ripeti;
        do {
            printf("\n");
            printf(TESTO_MAGENTA);
            printf("===================================\n");
            printf("=         TURNO DI %s         =\n", attuale->nomeUtente);
            printf("===================================\n");
            printf("\n");
            printf(TESTO_STANDARD);
            printf(TESTO_CIANO);
            printf("===================================\n");
            printf("|   Cosa vuoi fare?               |\n");
            printf("|=================================|\n");
            printf(TESTO_STANDARD"| 1. Gioca una carta              |\n");
            printf("| 2. Mostra resoconto punti       |\n");
            printf("| 3. Rage quitta                  |\n");
            printf(TESTO_CIANO);
            printf("===================================\n");
            printf(TESTO_STANDARD);

            printf("Scegli un'opzione:\t");
            scanf("%d", &scelta);

            ripeti = EseguiScelta(scelta, &attuale, &testa, numerogiocatori, carteGiocate, &i);
            if (ripeti == -MENUMIN) {
                return NULL;
            }

        } while (ripeti);

        if (attuale != NULL && scelta != MENUMAX) {
            attuale = attuale->next;
        }
    }

    return carteGiocate;
}


void stampaPunteggioTemporaneo(int *punteggioTemporaneo, int n_giocatori) {
    for (int i = INIZIO; i < n_giocatori; i++) {
        printf(TESTO_MAGENTA "Punteggio temporaneo per giocatore %d: %d\n", i + MENUMIN,punteggioTemporaneo[i]);
        printf(TESTO_STANDARD);
        fprintf(cronologia,"Punteggio temporaneo per giocatore %d: %d", i + MENUMIN,punteggioTemporaneo[i]);

    }
}
void controllaMazzo(Carte* carteCFU) {
    if (carteCFU == NULL) {
        printf(TESTO_ROSSO);
        printf("Non e' stato possibile trovare il mazzo di carte!.\n");
        fprintf(cronologia,"Non e' stato possibile trovare il mazzo di carte!.");

        exit(-MENUMIN);
    }
}
void controllaPunteggioTemporaneo(int* punteggioTemporaneo) {
    if (!punteggioTemporaneo) {
        printf("Errore nell'allocazione di memoria per punteggioTemporaneo.\n");
        exit(-MENUMIN);
    }
}
void controllaPersonaggi(Personaggio* personaggi) {
    if (personaggi == NULL) {
        printf(TESTO_ROSSO);
        printf("Non e' stato possibile trovare la lista personaggi!\n");
        exit(-MENUMIN);
    }
}
void CalcolaPunteggio(Giocatore* testa, int numerogiocatori, CartaOstacolo* ostacoloCorrente, CartaGiocata* carteGiocate, int* punteggioTemporaneo) {
    if (testa == NULL || ostacoloCorrente == NULL || carteGiocate == NULL || punteggioTemporaneo == NULL) {
        printf("Errore: Parametri non validi per CalcolaPunteggio.\n");
        return;
    }

    // indice per l'array BonusMalus in base al tipo dell'ostacolo
    int indiceBonusMalus = (int)ostacoloCorrente->tipoOstacolo;
    if (indiceBonusMalus < STUDIO || indiceBonusMalus > ESAME) {
        printf("Errore: Tipo di ostacolo non valido. Indice trovato: %d\n", indiceBonusMalus);
        return;
    }

    Giocatore* giocatoreCorrente = testa;
    for (int i = INIZIO; i < numerogiocatori; ++i) {
        if (giocatoreCorrente == NULL) {
            printf("Errore: Il numero di giocatori nella lista non corrisponde al numero previsto.\n");
            return;
        }

        int modificatore = giocatoreCorrente->personaggio.BonusMalus[indiceBonusMalus];
        int valoreCFUCartaGiocata = carteGiocate[i].carta.numeroCFU;
        punteggioTemporaneo[i] += valoreCFUCartaGiocata + modificatore;

        printf("Giocatore: %s, Carta CFU: %d, Modificatore: %d, Punteggio temporaneo: %d\n",giocatoreCorrente->nomeUtente, valoreCFUCartaGiocata, modificatore, punteggioTemporaneo[i]);
        fprintf(cronologia,"Giocatore: %s, Carta CFU: %d, Modificatore: %d, Punteggio temporaneo: %d",giocatoreCorrente->nomeUtente, valoreCFUCartaGiocata, modificatore, punteggioTemporaneo[i]);

        // passo giocatore successivo
        giocatoreCorrente = giocatoreCorrente->next;
    }
}
void StampaAnnuncioEffetti() {
    Trascrivi("Si inizia con gli effetti delle carte");
    printf(TESTO_CIANO);
    printf("\n=========================================\n");
    printf(" Giocate gli effetti delle vostre carte!    \n");
    printf("=========================================\n");
    printf(TESTO_STANDARD);
    printf("\n");
}
void StampaAnnuncioIstantanee() {
    Trascrivi("Si inizia con gli effetti delle carte istantanee");
    printf(TESTO_CIANO);
    printf("\n=========================================\n");
    printf("    Giocate le vostre carte istantanee!    \n");
    printf("=========================================\n");
    printf(TESTO_STANDARD);
    printf("\n");
}
Giocatore* ricostruisciGiocatori(CartaGiocata* carteGiocate, int numerogiocatori) {
    Trascrivi("Ricostruzione spazio di memoria per i giocatori");
    /*recuperaro tutti i dati dei giocatori partendo dalla lista delle
     * carte giocate e restituisco un vettore di Giocatore, in cui ogni giocatore corrisponde a
     * quello che ha giocato la carta corrispondente nell'array di CartaGiocata.*/
    Giocatore* giocatori = (Giocatore*) malloc(numerogiocatori * sizeof(Giocatore));
    for(int i = INIZIO; i < numerogiocatori; i++) {
        giocatori[i] = *carteGiocate[i].giocatore;
    }
    return giocatori;
}
void AttivaEffettiCarte(CartaGiocata* carteGiocate, int numerogiocatori, int* punteggioTemporaneo, Carte** testaMazzo, int tipoOstacolo) {
    Trascrivi("Attivazione effetti carte");
    if (carteGiocate == NULL || punteggioTemporaneo == NULL) {
        printf("Errore: parametri non validi.\n");
        exit(-MENUMIN);
    }
    /*ordino in modo crescente le carte giocate perchè per attivare l'effetto inizia
     * il giocatore che ha giocato la carta con valore cfu minore*/


    for (int i = INIZIO; i < numerogiocatori - MENUMIN; i++) {
        int min = i;
        for (int j = i + MENUMIN; j < numerogiocatori; j++)
            if (carteGiocate[j].carta.numeroCFU < carteGiocate[min].carta.numeroCFU)
                min = j;
        CartaGiocata scambia = carteGiocate[min];
        carteGiocate[min] = carteGiocate[i];
        carteGiocate[i] = scambia;
    }

    Giocatore *giocatori = ricostruisciGiocatori(carteGiocate, numerogiocatori);
    for (int i = INIZIO; i < numerogiocatori; i++) {
        int indiceGiocatore=i;
        Giocatore *giocatoreCorrente = carteGiocate[i].giocatore;
        Carte cartaGiocata = carteGiocate[i].carta;
        if (cartaGiocata.numeroCFU >= INIZIO && cartaGiocata.numeroCFU <=EFF_MAX) { //EFFMAX rappresenta il valore più alto dell'effetto attivabile per le carte punto, gli effetti successivi sono per le istantanee
            printf("\n%s, vuoi attivare l'effetto della carta %s? (s/n): ", giocatoreCorrente->nomeUtente,
                   cartaGiocata.nome_carta);
            char risposta;
            scanf(" %c", &risposta);
            fprintf(cronologia,"%s ha scelto che %c vuole attivare l'effetto della propria carta giocata",giocatoreCorrente->nomeUtente,risposta);
            while (getchar() != '\n'); //pulisce il buffer

            if (risposta == 's' || risposta == 'S') {
                switch (carteGiocate[i].carta.tipologia) {
                    case NESSUNO:
                        printf(TESTO_ROSSO);
                        printf("Hai giocato una carta priva di effetto!");
                        printf(TESTO_STANDARD);
                        break;
                    case SCARTAP:
                        printf(TESTO_GIALLO);
                        printf("Hai scelto di scartare una carta e aggiungere il suo punteggio.\n");
                        printf(TESTO_STANDARD);
                        EffettoScartaP(giocatoreCorrente, punteggioTemporaneo, i);
                        break;
                    case RUBA:
                        printf(TESTO_GIALLO);
                        printf("Hai scelto di rubare una carta da un altro giocatore.\n");
                        printf(TESTO_STANDARD);
                        EffettoRuba(giocatori, numerogiocatori, i);
                        break;
                    case SCAMBIADS:
                        printf(TESTO_GIALLO);
                        printf("Hai scelto di scambiare la tua carta con quella di un altro giocatore.\n");
                        printf(TESTO_STANDARD);
                        EffettoScambiaDS(giocatori, numerogiocatori, i);
                        break;
                    case SCARTAE:
                        printf(TESTO_GIALLO);
                        printf("Hai scelto di scartare una carta cfu punto con effetto e aggiungere il suo valore cfu al punteggio del turno corrente\n");
                        printf(TESTO_STANDARD);
                        EffettoScartaE(giocatori, indiceGiocatore, punteggioTemporaneo, testaMazzo);
                        break;

                    case SCARTAC:
                        printf(TESTO_GIALLO);
                        printf("Hai scelto di scartare da una a tre carte della tua mano.\n");
                        printf(TESTO_STANDARD);
                        EffettoScartaCarte(giocatoreCorrente);
                        break;

                    case SCAMBIAP:
                        printf(TESTO_GIALLO);
                        printf("Hai scelto di scambiare il punteggio massimo con quello minimo.\n");
                        printf(TESTO_STANDARD);
                        EffettoScambiaP(giocatori, punteggioTemporaneo, numerogiocatori);
                        break;
                    case DOPPIOE:
                        printf(TESTO_GIALLO);
                        printf("Hai scelto raddoppiare gli effetti delle carte che aumentano o diminuiscono il punteggio (per tutti).\n");
                        printf(TESTO_STANDARD);
                        for (int j = INIZIO; j < numerogiocatori; j++) {
                            switch (carteGiocate[i].carta.tipologia) {
                                case AUMENTA:
                                    punteggioTemporaneo[j] += NUMMIN;
                                    break;
                                case DIMINUISCI:
                                    punteggioTemporaneo[j] -= NUMMIN;
                                    break;
                            }
                        }
                        break;

                    case SBIRCIA:
                        printf(TESTO_GIALLO);
                        printf("Hai scelto di guardare due carte in cima al mazzo, scegli quale prendere in mano e quale scartare \n");
                        printf(TESTO_STANDARD);
                        EffettoSbircia(giocatoreCorrente, testaMazzo);
                        break;

                    case SCAMBIAC:
                        printf(TESTO_GIALLO);
                        printf("Hai scelto di scambia le carte con valore cfu tra due giocatori qualsiasi\n");
                        printf(TESTO_STANDARD);
                        EffettoScambiaCFU(giocatori, numerogiocatori);
                        break;

                }
            }
        }
        AggiungiCartaInFondoAlMazzo(testaMazzo, carteGiocate[i].carta);
    }
}
void AggiungiInFondo(CartaOstacolo** testa, CartaOstacolo* nuovaCarta) {
    if (*testa == NULL) {
        /*se il nodo iniziale puntato da *testa, è NULL, la lista è vuota quindi *testa viene
         * impostato per puntare alla nuova carta quind indicherà che la nuova carta è ora
         * il primoelemento nella lista.*/
        *testa = nuovaCarta;
    } else {
        /*Se *testa non è NULL, la lista non è vuota quindi creo un nodo temporaneo d'
         * apppoggio e lo uso per scorrere la lista fino
         * a raggiungere l'ultimo nodo della lista quindi quello per il quale next è NULL*/
        CartaOstacolo* apppoggio = *testa;
        while (apppoggio->next != NULL) {
            apppoggio = apppoggio->next;
        }
        apppoggio->next = nuovaCarta; //assegno al campo next l'indirizzo della nuova carrta così la collego alla fine della lista
    }
    nuovaCarta->next = NULL;
}
void GiocaIstantanee(Giocatore* testa, int numerogiocatori, CartaGiocata* carteGiocate, int* punteggioTemporaneo, CartaOstacolo* Attuale) {
    Trascrivi("Si inizia con gli effetti delle carte istantanee");
    if (testa == NULL) {
        printf("Errore: Non ci sono giocatori.\n");
        exit(-MENUMIN);
    }

    Giocatore* attuale = testa;
    for (int i = INIZIO; i < numerogiocatori; ++i) {
        if (attuale == NULL) {
            printf("Errore: meno giocatori di quelli previsti.\n");
            exit(-MENUMIN);
        }

        printf(TESTO_ROSSO);
        printf("E' il turno di %s\n", attuale->nomeUtente);
        printf(TESTO_STANDARD);
        fprintf(cronologia,"E' il turno di %s\n", attuale->nomeUtente);


        printf("Vuoi giocare una carta istantanea? (s/n): ");
        char risposta;
        scanf(" %c", &risposta);
        while (getchar() != '\n');

        if (risposta == 's' || risposta == 'S') {
            printf("Carte istantanee disponibili:\n");
            int checkIstantanee = INIZIO;
            for (int j = INIZIO; j < MANO; j++) {
                if (attuale->listaCarteCFU[j].numeroCFU == INIZIO) {
                    printf("Carta %d: %s\n", j+MENUMIN, attuale->listaCarteCFU[j].nome_carta);
                    checkIstantanee = MENUMIN;
                }
            }

            if (checkIstantanee) {
                int n_carta;
                do {
                    printf("Scegli una carta istantanea da giocare (inserisci il numero della carta): ");
                    scanf("%d", &n_carta);
                    while (getchar() != '\n');
                    n_carta--;
                } while (n_carta < INIZIO || n_carta >= MANO || attuale->listaCarteCFU[n_carta].numeroCFU != INIZIO);
                fprintf(cronologia,"%s vuole attivare la carta istantanea %d",attuale->nomeUtente,n_carta);
                printf("Hai giocato la carta istantanea: %s\n", attuale->listaCarteCFU[n_carta].nome_carta);
                switch (attuale->listaCarteCFU[n_carta].tipologia) {
                    case ANNULLA:
                        printf(TESTO_GIALLO);
                        printf("Hai scelto di annullare gli effetti di tutte le carte punto durante il turno .\n");
                        printf(TESTO_STANDARD);
                        EffettoAnnulla(carteGiocate, punteggioTemporaneo, numerogiocatori, Attuale->tipoOstacolo);
                        break;
                    case AUMENTA:
                        printf(TESTO_GIALLO);
                        printf("Effetto AUMENTA attivato.\n");
                        printf(TESTO_STANDARD);
                        EffettoAumenta(testa,numerogiocatori,punteggioTemporaneo);
                        break;
                    case DIMINUISCI:
                        printf(TESTO_GIALLO);
                        printf("Effetto DIMINUISCI attivato.\n");
                        printf(TESTO_STANDARD);
                        EffettoDiminuisci(testa,numerogiocatori,punteggioTemporaneo);
                        break;
                    case INVERTI:
                        printf(TESTO_GIALLO);
                        printf("Effetto INVERTI attivato.\n");
                        printf(TESTO_STANDARD);
                        EffettoInverti(punteggioTemporaneo, numerogiocatori);
                        break;
                }
                for (int j = n_carta; j < MANO - MENUMIN; j++) {
                    attuale->listaCarteCFU[j] = attuale->listaCarteCFU[j + MENUMIN];
                }
            } else {
                printf(TESTO_ROSSO);
                printf("Non hai carte istantanee da giocare.\n");
                printf(TESTO_STANDARD);
            }
        }
        attuale = attuale->next;
    }
}
void GestisciVincitoreEPerdente(Giocatore* giocatori, int numerogiocatori, int* punteggioTemporaneo, CartaOstacolo* cartaEstratta, CartaGiocata* carteGiocate, CartaOstacolo** testaMazzoOstacoli, Carte* mazzo, int numerocarte) {
    Trascrivi("Gestione vincitore/i e perdente/i");
    int max_punteggio = INT16_MIN;
    int min_punteggio = INT16_MAX;
    int* vincitori = malloc(numerogiocatori * sizeof(int));
    int* perdenti = malloc(numerogiocatori * sizeof(int));
    int numero_vincitori = INIZIO;
    int numero_perdenti = INIZIO;

    // cerco  i vincitori
    Giocatore* attuale = giocatori;
    for (int i = INIZIO; i < numerogiocatori; i++) {
        if (punteggioTemporaneo[i] > max_punteggio) {
            max_punteggio = punteggioTemporaneo[i];
            numero_vincitori = MENUMIN;
            vincitori[INIZIO] = i;
        } else if (punteggioTemporaneo[i] == max_punteggio) {
            vincitori[numero_vincitori++] = i;
        }

        if (punteggioTemporaneo[i] < min_punteggio) {
            min_punteggio = punteggioTemporaneo[i];
            numero_perdenti = MENUMIN;
            perdenti[INIZIO] = i;
        } else if (punteggioTemporaneo[i] == min_punteggio) {
            perdenti[numero_perdenti++] = i;
        }
    }

    // stampo dei vincitori
    printf(TESTO_MAGENTA "Vincitori del turno:\n" TESTO_STANDARD);
    for (int i = INIZIO; i < numero_vincitori; i++) {
        int c = INIZIO;
        attuale = giocatori;
        while (c < vincitori[i]) {
            attuale = attuale->next;
            c++;
        }
        printf(TESTO_MAGENTA "Il giocatore %s vince il turno con un punteggio di %d CFU.\n" TESTO_STANDARD, attuale->nomeUtente, max_punteggio);
        attuale->numeroCFU += max_punteggio; // aggiornoi CFU del vincitore
    }

    // gestisco i perdenti
    if (numero_perdenti > MENUMIN) {
        printf("Turno extra tra i perdenti a pari merito.\n");

        // creo un nuovo  array per tenere traccia dei punteggi temporanei durante il turno extra
        int* punteggioTemporaneoExtra = calloc(numerogiocatori, sizeof(int));
        if (!punteggioTemporaneoExtra) {
            printf("Errore nell'allocazione memoria per i punteggi temporanei extra.\n");
            exit(-MENUMIN);
        }

        //turno extra
        for (int i = INIZIO; i < numero_perdenti; i++) {
            int indiceGiocatore = perdenti[i];
            CartaGiocata* cartaGiocata = Giocata(&giocatori[indiceGiocatore], &numerogiocatori, mazzo, numerocarte);
            AttivaEffettiCarte(cartaGiocata, MENUMIN, punteggioTemporaneoExtra, (Carte **) testaMazzoOstacoli, cartaGiocata->carta.tipologia);
        }

        // cerco il perdente finale
        int minPunteggioExtra = INT16_MAX;
        int indicePerdenteExtra = -MENUMIN;
        for (int i = INIZIO; i < numero_perdenti; i++) {
            int indiceGiocatore = perdenti[i];
            if (punteggioTemporaneoExtra[indiceGiocatore] < minPunteggioExtra) {
                minPunteggioExtra = punteggioTemporaneoExtra[indiceGiocatore];
                indicePerdenteExtra = indiceGiocatore;
            }
        }

        // se la carta giocata è di tipologia DIROTTA o SALVA permetto al giocatore di slavarsi dal prendersi la carta ostacolo
        printf("Gestione dell'effetto della carta per il perdente del turno extra.\n");
        if (indicePerdenteExtra >= INIZIO) {
            Giocatore* giocatorePerdenteExtra = &giocatori[indicePerdenteExtra];
            CartaGiocata cartaPerdenteExtra = carteGiocate[indicePerdenteExtra];

            switch (cartaPerdenteExtra.carta.tipologia) {
                case DIROTTA:
                    printf(TESTO_GIALLO "Effetto DIROTTA attivato dal giocatore %s.\n" TESTO_STANDARD, giocatorePerdenteExtra->nomeUtente);
                    EffettoDirotta(giocatori, numerogiocatori, cartaEstratta, indicePerdenteExtra);
                    break;
                case SALVA:
                    printf(TESTO_GIALLO "Effetto SALVA attivato dal giocatore %s.\n" TESTO_STANDARD, giocatorePerdenteExtra->nomeUtente);
                    EffettoSalva(testaMazzoOstacoli, cartaEstratta);
                    break;
                default:
                    printf(TESTO_ROSSO "Al giocatore %s è stata assegnata la carta ostacolo %s.\n" TESTO_STANDARD, giocatorePerdenteExtra->nomeUtente, cartaEstratta->nome_carta_ostacolo);
                    AssegnaCartaOstacoloAGiocatore(giocatorePerdenteExtra, cartaEstratta);
                    giocatorePerdenteExtra->numeroCFU += MENUMIN;
                    break;
            }
        }

        free(punteggioTemporaneoExtra);
    } else if (numero_perdenti == MENUMIN) {
        int idPerdente = perdenti[INIZIO];
        attuale = giocatori;
        for (int j = INIZIO; j < idPerdente; j++) {
            attuale = attuale->next;
        }
        // se c'è solo un perdente
        switch (carteGiocate[idPerdente].carta.tipologia) {
            case DIROTTA:
                printf(TESTO_GIALLO "Effetto DIROTTA attivato dal giocatore %s.\n" TESTO_STANDARD, attuale->nomeUtente);
                EffettoDirotta(giocatori, numerogiocatori, cartaEstratta, idPerdente);
                break;
            case SALVA:
                printf(TESTO_GIALLO "Effetto SALVA attivato dal giocatore %s.\n" TESTO_STANDARD, attuale->nomeUtente);
                EffettoSalva(testaMazzoOstacoli, cartaEstratta);
                break;
            default:
                AssegnaCartaOstacoloAGiocatore(attuale, cartaEstratta);
                printf(TESTO_GIALLO "Al giocatore %s è stata assegnata la carta ostacolo %s.\n" TESTO_STANDARD, attuale->nomeUtente, cartaEstratta->nome_carta_ostacolo);

                attuale->numeroCFU += MENUMIN;
        }
    }
    for (int i = INIZIO; i < numerogiocatori; i++) {
        punteggioTemporaneo[i] = INIZIO;
    }
    free(vincitori);
    free(perdenti);
}
void RimuoviGiocatori(Giocatore** testa) {
    Giocatore* temporaneo = *testa;
    Giocatore* prec = NULL;
    int contaGiocatori = INIZIO;

    while (temporaneo != NULL) {
        int contatoreostacoli[NUMMAX] = {INIZIO}; // 4 come il numero di tipo ostacolo diversi
        int jolly = INIZIO;

        //controllo le carte ostacolo del giocatore
        CartaOstacolo* carta = temporaneo->listaCarteOstacolo;
        while (carta != NULL) {
            if (carta->tipoOstacolo == ESAME) {
                jolly++;
            } else {
                contatoreostacoli[carta->tipoOstacolo]++;
            }
            carta = carta->next;
        }

        // Conto gli ostacoli
        int tipiDiversi = INIZIO;
        for (int i = INIZIO; i < NUMMAX - MENUMIN; i++) { // -1 perchè ESAME è l'ultimo valore dell'enumerazione tipologia
            if (contatoreostacoli[i] > INIZIO) {
                tipiDiversi++;
            }
        }

        // aggiungo ESAME al conteggio dei tipi diversi
        if (jolly > INIZIO && tipiDiversi < MENUMAX) {
            tipiDiversi += jolly;
            if (tipiDiversi > MENUMAX) {
                tipiDiversi = MENUMAX;
            }
        }

        // controllo o 3 carte uguali o la combinazione con una carta jolly ESAME
        bool treUguali = false;
        for (int i = INIZIO; i < NUMMAX - MENUMIN; i++) { // non considero ESAME nel controllo
            if (contatoreostacoli[i] >= MENUMAX || (contatoreostacoli[i] >= NUMMIN && jolly >= MENUMIN) || (contatoreostacoli[i] == MENUMIN && jolly >= NUMMIN)) {
                treUguali = true;
            }
        }

        if (tipiDiversi >= MENUMAX || treUguali) {
            printf("Il giocatore %s è stato eliminato.\n", temporaneo->nomeUtente);
            // elimino giocatore e carte
            Giocatore* daEliminare = temporaneo;

            // elimino le ostacolo
            while (daEliminare->listaCarteOstacolo != NULL) {
                CartaOstacolo* tempCarta = daEliminare->listaCarteOstacolo;
                daEliminare->listaCarteOstacolo = tempCarta->next;
                free(tempCarta);
            }

            // modifico i puntatori per gestire l'eliminazione
            if (prec != NULL) {
                prec->next = temporaneo->next;
            } else { // se il giocatore da eliminare è la testa della lista
                *testa = temporaneo->next;
            }
            temporaneo = temporaneo->next;

            // deallocazione della memoria del giocatore eliminato
            free(daEliminare);
        } else {
            prec = temporaneo;
            temporaneo = temporaneo->next;
        }
    }

    // conto i giocatori rimanenti per controllare se la partita può continuare
    Giocatore* controllo = *testa;
    while (controllo != NULL) {
        contaGiocatori++;
        controllo = controllo->next;
    }

    if (contaGiocatori == MENUMIN) {
        printf(TESTO_ROSSO "Rimane un solo giocatore. Partita terminata.\n" TESTO_STANDARD);
        exit(-MENUMIN);
    }
}

void SalvaPartita(Giocatore* giocatori, int n_giocatori) {
    FILE* file = fopen("savegame.sav", "wb");
    if (!file) {
        printf("Errore nell'apertura del file di salvataggio");
        exit(-MENUMIN);
    }

    //salvo il numero dei giocatori
    fwrite(&n_giocatori, sizeof(int), MENUMIN, file);

    // ciclo sui giocatori per salvare le informazioni di ciascuno
    Giocatore* g = giocatori;
    while (g != NULL) {
        // salvo il nome del giocatore
        int lunghezzaNome = strlen(g->nomeUtente) + MENUMIN;
        fwrite(&lunghezzaNome, sizeof(int), MENUMIN, file);
        fwrite(g->nomeUtente, sizeof(char), lunghezzaNome, file);

        //salvo le info del giocatore
        fwrite(&g->numeroCFU, sizeof(int), MENUMIN, file); //numero CFU

        //salvo il numero di carte CFU in mano al giocatore
        int n_carteCFU = INIZIO;
        Carte* c = g->listaCarteCFU;
        while (c != NULL) {
            n_carteCFU++;
            c = c->next;
        }
        fwrite(&n_carteCFU, sizeof(int), MENUMIN, file);

        // salvo le carte CFU
        c = g->listaCarteCFU;
        while (c != NULL) {
            int lunghezzaCarta = strlen(c->nome_carta) + MENUMIN;
            fwrite(&lunghezzaCarta, sizeof(int), MENUMIN, file);
            fwrite(c->nome_carta, sizeof(char), lunghezzaCarta, file);
            //salvo le altre info della carta
            fwrite(&c->occorrenza, sizeof(int), MENUMIN, file);
            fwrite(&c->numeroCFU, sizeof(int), MENUMIN, file);
            fwrite(&c->tipologia, sizeof(int), MENUMIN, file);
            c = c->next;
        }

        //salvo il numero di carte ostacolo
        int n_carteOstacolo = INIZIO;
        CartaOstacolo* o = g->listaCarteOstacolo;
        while (o != NULL) {
            n_carteOstacolo++;
            o = o->next;
        }
        fwrite(&n_carteOstacolo, sizeof(int), MENUMIN, file);

        //salvo le carte ostacolo
        o = g->listaCarteOstacolo;
        while (o != NULL) {
            int lunghezzaOstacolo = strlen(o->nome_carta_ostacolo) + MENUMIN;
            fwrite(&lunghezzaOstacolo, sizeof(int), MENUMIN, file);
            fwrite(o->nome_carta_ostacolo, sizeof(char), lunghezzaOstacolo, file);
            // info dell'ostacolo
            int lunghezzaDesc = strlen(o->descrizione_ostacolo) + MENUMIN;
            fwrite(&lunghezzaDesc, sizeof(int), MENUMIN, file);
            fwrite(o->descrizione_ostacolo, sizeof(char), lunghezzaDesc, file);
            fwrite(&o->tipoOstacolo, sizeof(int), MENUMIN, file);
            o = o->next;
        }
        //stessa cosa salvo i perosnaggi
        int lunghezzaNomePersonaggio = strlen(g->personaggio.nome) + MENUMIN;
        fwrite(&lunghezzaNomePersonaggio, sizeof(int), MENUMIN, file);
        fwrite(g->personaggio.nome, sizeof(char), lunghezzaNomePersonaggio, file);
        fwrite(g->personaggio.BonusMalus, sizeof(int), NUMMAX, file);

        g = g->next; // passo al giocatore dopo
    }

    fclose(file);
}
void CaricaPartita(Giocatore** giocatori, int* n_giocatori) {
    FILE* file = fopen("savegame.sav", "rb");
    Giocatore* ultimo = NULL;

    if (!file) {
        printf("Errore nell'apertura del file di caricamento\n");
        return;
    }

    fread(n_giocatori, sizeof(int), MENUMIN , file);

    *giocatori = (Giocatore*) calloc(*n_giocatori, sizeof(Giocatore));

    for (int i=INIZIO; i< *n_giocatori; i++) {
        Giocatore* gioc = &(*giocatori)[i];

        if (ultimo != NULL) {
            ultimo->next = gioc;
        }
        ultimo = gioc;

        // lettura del nome utente
        int lunghezzaNome = INIZIO;
        fread(&lunghezzaNome, sizeof(int), MENUMIN , file);
        gioc->nomeUtente = (char*) calloc(lunghezzaNome, sizeof(char));
        fread(gioc->nomeUtente, sizeof(char), lunghezzaNome , file);

        fread(&gioc->numeroCFU, sizeof(int), MENUMIN , file);

        // lettura delle carte CFU
        int n_carteCFU = INIZIO;
        fread(&n_carteCFU, sizeof(int), MENUMIN , file);
        gioc->listaCarteCFU = NULL;

        for (int j=INIZIO; j<n_carteCFU; j++) {
            Carte* cart = (Carte*) calloc(MENUMIN, sizeof(Carte));
            int lunghezzaCarta = INIZIO;

            fread(&lunghezzaCarta, sizeof(int), MENUMIN , file);
            char* nomeCarta = (char*) calloc(lunghezzaCarta, sizeof(char));
            fread(nomeCarta, sizeof(char), lunghezzaCarta , file);
            strncpy(cart->nome_carta, nomeCarta, DIM);
            free(nomeCarta);

            fread(&cart->occorrenza, sizeof(int), MENUMIN , file);
            fread(&cart->numeroCFU, sizeof(int), MENUMIN , file);
            fread(&cart->tipologia, sizeof(int), MENUMIN , file);

            cart->next = gioc->listaCarteCFU;
            gioc->listaCarteCFU = cart;
        }

        // lettura delle carte Ostacolo
        int n_carteOstacolo = INIZIO;
        fread(&n_carteOstacolo, sizeof(int), MENUMIN , file);
        gioc->listaCarteOstacolo = NULL;

        for(int j=INIZIO; j<n_carteOstacolo; j++){
            CartaOstacolo* ost = (CartaOstacolo*) calloc(MENUMIN, sizeof(CartaOstacolo));
            int lunghezzaOstacolo = INIZIO;

            fread(&lunghezzaOstacolo, sizeof(int), MENUMIN , file);
            char* nomeOstacolo = (char*) calloc(lunghezzaOstacolo, sizeof(char));
            fread(nomeOstacolo, sizeof(char), lunghezzaOstacolo , file);
            strncpy(ost->nome_carta_ostacolo, nomeOstacolo, DIM);
            free(nomeOstacolo);

            int lunghezzaDesc = INIZIO;
            fread(&lunghezzaDesc, sizeof(int), MENUMIN , file);
            char* descOstacolo = (char*) calloc(lunghezzaDesc, sizeof(char));
            fread(descOstacolo, sizeof(char), lunghezzaDesc , file);
            strncpy(ost->descrizione_ostacolo, descOstacolo, DES);
            free(descOstacolo);

            fread(&ost->tipoOstacolo, sizeof(TipoOstacolo), MENUMIN , file);

            ost->next = gioc->listaCarteOstacolo;
            gioc->listaCarteOstacolo = ost;
        }

        // leggo il personaggio e i dettagli
        int lunghezzaNomePersonaggio = INIZIO;
        fread(&lunghezzaNomePersonaggio, sizeof(int), MENUMIN , file);
        char* nomePersonaggio = (char*) calloc(lunghezzaNomePersonaggio, sizeof(char));
        fread(nomePersonaggio, sizeof(char), lunghezzaNomePersonaggio , file);
        strncpy(gioc->personaggio.nome, nomePersonaggio, DIM);

        free(nomePersonaggio);
        fread(gioc->personaggio.BonusMalus, sizeof(int), NUMMAX , file);
    }

    if (ultimo != NULL) {
        ultimo->next = NULL;  // facccio puntre a null l'ultimo giocatore
    }

    fclose(file);
}
void controllaNumeroDiGiocatori(int n_giocatori) {
    if (n_giocatori < NUMMIN) {
        printf(TESTO_ROSSO);
        printf("Non ci sono abbastanza giocatori. Il gioco terminerà.\n");
        Trascrivi("Gioco finito a casua dell'insufficienza del numero dei giocatori");
        printf(TESTO_STANDARD);
        exit(-MENUMIN);
    }
}
void EseguiTurno(Giocatore** giocatori, int* n_giocatori, CartaOstacolo** ostacoli, Carte** carteCFU, int* CarteCFU, CartaGiocata** Giocate, int* punteggioTemporaneo, int* conta_vincitori, int* n_turno) {
    bool partitaTerminata = false; // uso questa flag per concludere la partita


    do {
        SalvaPartita(*giocatori, *n_giocatori);
        printf(TESTO_ROSSO "\nTURNO %d\n", *n_turno);
        printf(TESTO_STANDARD);
        AssegnazioneCarteCfu(*giocatori, *n_giocatori, *carteCFU, *CarteCFU);
        MostraManoGiocatori(*giocatori, *n_giocatori);
        *ostacoli = LeggiOstacoli("ostacoli.txt");
        CartaOstacolo *OstacoloRandom = EstrazioneOstacolo(*ostacoli);
        StampaOstacoloEstratto(OstacoloRandom);
        *Giocate = Giocata(*giocatori, n_giocatori, *carteCFU, *CarteCFU);
        CalcolaPunteggio(*giocatori, *n_giocatori, OstacoloRandom, *Giocate, punteggioTemporaneo);
        stampaPunteggioTemporaneo(punteggioTemporaneo, *n_giocatori);
        StampaAnnuncioEffetti();
        AttivaEffettiCarte(*Giocate, *n_giocatori, punteggioTemporaneo, carteCFU, OstacoloRandom->tipoOstacolo);
        StampaAnnuncioIstantanee();
        GiocaIstantanee(*giocatori, *n_giocatori, *Giocate, punteggioTemporaneo, OstacoloRandom);
        GestisciVincitoreEPerdente(*giocatori, *n_giocatori, punteggioTemporaneo, OstacoloRandom, *Giocate, ostacoli, *carteCFU, *CarteCFU);

        *conta_vincitori = INIZIO; //resetto il conteggio dei vincitori per questo turno
        Giocatore* attuale=*giocatori;
        while (attuale != NULL) {
            if (attuale->numeroCFU >= FINEGIOCO) {
                (*conta_vincitori)++;
                printf("Il giocatore %s ha raggiunto %d CFU e vince la partita!\n", attuale->nomeUtente, attuale->numeroCFU);
                partitaTerminata = true; // cambio il controllo
            }
            attuale = attuale->next; // prossimo giocatre
        }

        RimuoviGiocatori(giocatori);
        (*n_turno)++;
    } while(!partitaTerminata);
    printf(TESTO_ROSSO);
    printf("Partita conclusa! Un/piu' giocatore/i ha/hanno raggiunto 60 CFU.\n");
    printf(TESTO_STANDARD);
    Trascrivi("Partita conclusa! Un/piu' giocatore/i ha/hanno raggiunto 60 CFU.");
}
void InizializzaGioco(Giocatore** giocatori, int* n_giocatori, CartaOstacolo** ostacoli, Carte** carteCFU, int* punteggioTemporaneo, int* num_personaggi, Personaggio** personaggi, int* CarteCFU, char* personaggio, char* carta, bool* partitaCaricata) {
    Titolo();
    CreaCronologia();
    Menu(giocatori, n_giocatori, ostacoli, carteCFU,partitaCaricata);
    if(*partitaCaricata){ //se carico la partita eseguo solo queste funzioni
        *personaggi = leggipersonaggi(personaggio, num_personaggi);
        *carteCFU = leggicarteCFU(carta, CarteCFU);
        controllaMazzo(*carteCFU);
        RandomizzaMazzo(*carteCFU, *CarteCFU);
        stampa_giocatori(*giocatori, *n_giocatori);

    }
    if (!*partitaCaricata) { //se non carico la partita eseguo l'intero ciclo di funzioni
        *n_giocatori = richiestaNumeroGiocatori();
        *giocatori = allocaGiocatori(*n_giocatori);
        controllaPunteggioTemporaneo(punteggioTemporaneo);
        *personaggi = leggipersonaggi(personaggio, num_personaggi);
        controllaPersonaggi(*personaggi);
        RandomizzaPersonaggi(*personaggi, *num_personaggi);
        AssegnazionePersonaggi(*giocatori, *personaggi, *n_giocatori);
        PuliziaTerminale();
        stampa_giocatori(*giocatori, *n_giocatori);
        *carteCFU = leggicarteCFU(carta, CarteCFU);
        controllaMazzo(*carteCFU);
        RandomizzaMazzo(*carteCFU, *CarteCFU);
    }
}

