//
// Created by Thomas Nieddu on 21/02/25.
//
#include "regole_gioco.h"
#include "buffer.h"
#include <pthread.h>
#include <signal.h>
int vite = VITEINIZIALI;
int tempo_rimasto = TEMPO_MASSIMO;

pthread_t *coccodrilliThreads = NULL;

bool movimentoManuale = false;

//resetto la rana a una posizione iniziale
void reset_rana(struct personaggio *rana, int x, int y) {
    rana->posizione.x = x;
    rana->posizione.y = y;
}

//controllo se tutte le tane sono chiuse
bool tutte_tane_chiuse() {
    for(int i = 0; i < num_tane; i++) {
        if(tane_aperte[i]) return false;
    }
    return true;
}


//controllo se la rana entra in una tana
bool check_tane(struct personaggio rana) {
    int riga_inizio_buco = tana_inizio_riga + 1;
    int riga_fine_buco = tana_inizio_riga + 4;

    if (rana.posizione.y + rana_altezza - 1 < tana_inizio_riga || rana.posizione.y > tana_fine_riga)
    {
        return false;
    }

    for(int i = 0; i < num_tane; i++) {
        int inizio_tana_x = gioco_sinistra + offset_tane+ i * (larghezza_tana + spazio);
        int fine_tana_x = inizio_tana_x + larghezza_tana - 1;
        int inizio_buco_x = inizio_tana_x + 1;
        int larghezza_buco = larghezza_tana - 1;
        int fine_buco_x = inizio_buco_x + larghezza_buco - 1;

        bool sovrapposizione_tana = (rana.posizione.x + rana.lunghezza - 1 >= inizio_tana_x) && (rana.posizione.x <= fine_tana_x);

        bool sovrapposizione_vert_tana = (rana.posizione.y + rana_altezza - 1 >= tana_inizio_riga) && (rana.posizione.y <= tana_fine_riga);

        if (sovrapposizione_tana && sovrapposizione_vert_tana) {
            bool sovrapposizione_buco =
                    (rana.posizione.x + rana.lunghezza - 1 >= inizio_buco_x)
                    && (rana.posizione.x <= fine_buco_x)
                    && (rana.posizione.y + rana_altezza - 1 >= riga_inizio_buco)
                    && (rana.posizione.y <= riga_fine_buco);

            if(sovrapposizione_buco) {
                if(tane_aperte[i]) {
                    if(vite < 8) vite++;
                    tempo_rimasto = TEMPO_MASSIMO;
                    tane_aperte[i] = false;
                    punteggio += 50;
                } else {
                    perdivita();
                }
            } else {
                perdivita();
            }
            return true;
        }
    }
    return false;
}

//conto i coccodrilli, tante righe quante stanno fra prato2_fine_riga e riga_inizio_prato
int conta_coccodrilli() {
    int count = 0;
    int y = prato2_fine_riga;
    while (y < riga_inizio_prato) {
        count += COCCODRILLIXRIGA;
        y += 2;
    }
    return count;
}

/*
 * spawn_coccodrilli():
 */
void spawn_coccodrilli(struct personaggio *coccodrilli, int n_coccodrilli)
{
    coccodrilliThreads = malloc(n_coccodrilli * sizeof(pthread_t));
    if (coccodrilliThreads == NULL) {
        fprintf(stderr, "Errore: allocazione di memoria fallita per %d coccodrilliThreads.\n", n_coccodrilli);
        exit(5);
    }

    int y_iniziale = riga_inizio_prato - 2;
    int x_iniziale = gioco_sinistra;
    bool destra;

    srand(time(NULL));
    destra = (rand() % 2 == 0);

    for(int i = 0; i < n_coccodrilli; i++) {
        coccodrilli[i].id   = i;
        coccodrilli[i].tipo = COCCODRILLO;
        coccodrilli[i].lunghezza =
                (rand() % (LUNGHEZZA_MASSIMA - LUNGHEZZA_MINIMA + 1)) + LUNGHEZZA_MINIMA;

        if(rand() % 2 == 0){
            coccodrilli[i].animation = true;
        }else{
            coccodrilli[i].animation = false;
        }

        if (i % COCCODRILLIXRIGA == 0 && i != 0) {
            y_iniziale -= 2;
            destra = !destra;
            x_iniziale = gioco_sinistra -  LUNGHEZZA_MASSIMA;
        } else if (i != 0) {
            x_iniziale = coccodrilli[i - 1].posizione.x
                         + coccodrilli[i - 1].lunghezza + (rand() % (DISTANZA_MASSIMA - DISTANZA_MINIMA + 1))
                         + DISTANZA_MINIMA;
        }

        coccodrilli[i].destra       = destra;
        coccodrilli[i].posizione.x  = x_iniziale;
        coccodrilli[i].posizione.y  = y_iniziale;

        CoccodrilloArgs *args = malloc(sizeof(CoccodrilloArgs));
        if (args == NULL) {
            fprintf(stderr, "Errore: allocazione di memoria fallita per CoccodrilloArgs.\n");
            exit(6);
        }
        args->coccodrillo = coccodrilli[i];

        if (pthread_create(&coccodrilliThreads[i], NULL, processo_coccodrilli, args) != 0)
        {
            endwin();
            perror("pthread_create coccodrillo");
            exit(1);
        }
    }
}

//perdo vita in caso di collisioni
bool perdivita() {
    vite--;
    punteggio -= 20;
    tempo_rimasto = TEMPO_MASSIMO;
    return true;
}

//controllo se la rana sta su un coccodrillo
void controllo_coccodrilli(struct personaggio *rana, struct personaggio coccodrilli[], int n_coccodrilli) {
    static int currentCocco = -1;
    static int relativeOffset = 0;

    int frogCenter = rana->posizione.x + rana->lunghezza / 2;

    for (int i = 0; i < n_coccodrilli; i++) {
        if (rana->posizione.y == coccodrilli[i].posizione.y &&
            frogCenter >= coccodrilli[i].posizione.x &&
            frogCenter <= coccodrilli[i].posizione.x + coccodrilli[i].lunghezza)
        {
            // Se atterra su un nuovo coccodrillo, calcola l'offset iniziale
            if (currentCocco != i) {
                currentCocco = i;
                relativeOffset = rana->posizione.x - coccodrilli[i].posizione.x;
            }
            else {
                // Se c'è stato un movimento manuale, aggiorna l'offset
                if (movimentoManuale) {
                    relativeOffset = rana->posizione.x - coccodrilli[i].posizione.x;
                    movimentoManuale = false;  // reset del flag dopo l'aggiornamento
                }
            }

            // Limita l'offset in modo che la rana resti sul coccodrillo
            int maxOffset = coccodrilli[i].lunghezza - rana->lunghezza;
            if (relativeOffset < 0)
                relativeOffset = 0;
            if (relativeOffset > maxOffset)
                relativeOffset = maxOffset;

            // Aggiorna la posizione della rana mantenendo l'offset
            rana->posizione.x = coccodrilli[i].posizione.x + relativeOffset;
            break; // Gestiamo solo il primo coccodrillo su cui la rana è appoggiata
        }
    }
}

//controllo collisioni con proiettili
bool controllo_proiettili(struct personaggio rana,
                          struct personaggio proiettili[],
                          int n_coccodrilli)
{
    for (int i = 0; i < n_coccodrilli; i++) {
        if (proiettili[i].posizione.x >= gioco_sinistra
            && proiettili[i].posizione.x <= gioco_destra)
        {
            bool collisioneX =
                    (rana.posizione.x < proiettili[i].posizione.x + proiettili[i].lunghezza)
                    && (rana.posizione.x + rana.lunghezza > proiettili[i].posizione.x);
            bool collisioneY =
                    (rana.posizione.y == proiettili[i].posizione.y);

            if (collisioneX && collisioneY) {
                return perdivita();
            }
        }
    }
    return false;
}

//controllo se la rana è sul fiume senza coccodrillo
bool controllo_fiume(struct personaggio rana,
                     struct personaggio coccodrilli[],
                     int n_coccodrilli)
{
    if (rana.posizione.y < riga_inizio_prato
        && rana.posizione.y > prato2_fine_riga)
    {
        for(int i = 0; i < n_coccodrilli; i++) {
            if (coccodrilli[i].posizione.y == rana.posizione.y) {
                bool overlapX =
                        (rana.posizione.x + rana.lunghezza - 1 >= coccodrilli[i].posizione.x)
                        && (rana.posizione.x <= coccodrilli[i].posizione.x + coccodrilli[i].lunghezza - 1);
                if (overlapX) {
                    return false;
                }
            }
        }
        return perdivita();
    }
    return false;
}

//controllo se la rana esce dal campo
bool controllo_bordi(struct personaggio rana) {
    if (rana.posizione.x < gioco_sinistra
        || (rana.posizione.x + rana.lunghezza - 1) > gioco_destra
        || rana.posizione.y > riga_fine_prato - 2)
    {
        return perdivita();
    }
    return false;
}
