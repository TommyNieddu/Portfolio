//
// Created by Davide Balestrino on 19/12/24.
//

#include "prato_tane.h"
#include <signal.h>

int max_righe;
int max_colonne;

int gioco_sinistra; //da dove parte il campo di gioco sulla sinistra
int gioco_destra; //dove finisce il campo di gioco a destra
int larghezza_gioco;   //quanti caratteri "larghi" è il campo orizzontalmente

//prato inferiore
int prato_altezza = 6;       // definizione e inizializzazione
int riga_inizio_prato;
int riga_fine_prato;

//secondo prato dopo le tane
int prato2_altezza = 6;
int prato2_inizio_riga;
int prato2_fine_riga;

//parametri delle tane
int tana_altezza = 5;
int num_tane = 5;
int spazio = 3;
int tana_inizio_riga;
int tana_fine_riga;
int spazio_totale;
int totale_per_le_tane;
int larghezza_tana;
int offset_tane;

bool tane_aperte[5]; //array bool per sapere se la tana i è aperta o no

void stampa_terreno(int r, int stop, int colore) {
    attron(COLOR_PAIR(colore));
    for(int i = r; i< stop; i++) {
        mvhline(i, gioco_sinistra, ' ', larghezza_gioco);
    }
    attroff(COLOR_PAIR(colore));
}

//disegno tutt,le tane gialle, i buchi neri, e i due prati
void disegna_scenario() {
    //coloro in giallo le righe da tana_inizio_riga a tana_fine_riga
    attron(COLOR_PAIR(8));
    for(int r = tana_inizio_riga; r <= tana_fine_riga; r++) {
        mvhline(r, gioco_sinistra, ' ', larghezza_gioco);
    }
    attroff(COLOR_PAIR(8));

    //se la tana i è aperta, ci disegno un buco nero
    int riga_inizio_buco = tana_inizio_riga + 1; //parto un po' più sotto per non coprire
    int riga_fine_buco   = tana_inizio_riga + 4; //altezza del buco
    for(int i = 0; i < num_tane; i++) {
        if(tane_aperte[i]) {
            //calcolo l'inizio della tana in x
            int inizio_tana_x = gioco_sinistra + offset_tane + i * (larghezza_tana + spazio);
            //fine a sinistra + offset + i*(larghezza + spazio)

            int inizio_buco_x = inizio_tana_x + 1;
            int larghezza_buco = larghezza_tana - 1;

            //e lo coloro di nero
            attron(COLOR_PAIR(9));
            for(int r = riga_inizio_buco; r <= riga_fine_buco; r++) {
                for(int c = 0; c < larghezza_buco; c++) {
                    mvaddch(r, inizio_buco_x + c, ' ');
                }
            }
            attroff(COLOR_PAIR(9));
        }
    }

    //disegno il prato2
    stampa_terreno(prato2_inizio_riga, prato2_fine_riga, 1);

    //disegno il fiume
    stampa_terreno(prato2_fine_riga, riga_inizio_prato, 5);

    //disegno il prato inferiore
    stampa_terreno(riga_inizio_prato, riga_fine_prato, 1);
}

