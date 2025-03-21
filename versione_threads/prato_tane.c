//created by Thomas Nieddu on 21/02/25.
#include <ncurses.h>
#include "prato_tane.h"
#include <pthread.h>
#include <signal.h>
//queste due variabili soo per  le dimensioni massime dello schermo
int max_righe;
int max_colonne;

//coordinate per il campo di gioco orizzontale
int gioco_sinistra;
int gioco_destra;
int larghezza_gioco;

//il primo prato
int prato_altezza  = 6;
int riga_inizio_prato;
int riga_fine_prato;

//secondo prato
int prato2_altezza = 6;
int prato2_inizio_riga;
int prato2_fine_riga;

//parametri delle tane
int tana_altezza  = 5;
int tana_inizio_riga;
int tana_fine_riga;
int num_tane  = 5;
int spazio    = 3;
int spazio_totale;
int totale_per_le_tane;
int larghezza_tana;
int offset_tane;

//tane_aperte mi dice se la tana i è aperta (true) o chiusa (false)
bool tane_aperte[5];

//coloro le righe da r a stop-1
void stampa_terreno(int r, int stop, int colore) {
    attron(COLOR_PAIR(colore));
    for(int i = r; i < stop; i++) {
        mvhline(i, gioco_sinistra, ' ', larghezza_gioco);
    }
    attroff(COLOR_PAIR(colore));
}

//qui metto il prato in basso, il prato in alto e la zona delle tane/fiume
void disegna_scenario() {
    //coloro la fascia gialla dove ci sono le tane
    attron(COLOR_PAIR(8));
    for(int r = tana_inizio_riga; r <= tana_fine_riga; r++) {
        mvhline(r, gioco_sinistra, ' ', larghezza_gioco);
    }
    attroff(COLOR_PAIR(8));

    //disegno i buchi neri nelle tane aperte
    int riga_inizio_buco = tana_inizio_riga + 1;
    int riga_fine_buco   = tana_inizio_riga + 4;
    for(int i = 0; i < num_tane; i++) {
        if(tane_aperte[i]) {
            //calcolo l’inizio della tana i
            int inizio_tana_x  = gioco_sinistra + offset_tane+ i * (larghezza_tana + spazio);
            int inizio_buco_x  = inizio_tana_x + 1;
            int larghezza_buco = larghezza_tana - 1;

            attron(COLOR_PAIR(9));
            for(int r = riga_inizio_buco; r <= riga_fine_buco; r++) {
                for(int c = 0; c < larghezza_buco; c++) {
                    mvaddch(r, inizio_buco_x + c, ' ');
                }
            }
            attroff(COLOR_PAIR(9));
        }
    }

    //disegno il secondo prato
    stampa_terreno(prato2_inizio_riga, prato2_fine_riga, 1);

    //disegno il fiume
    stampa_terreno(prato2_fine_riga, riga_inizio_prato, 5);

    //disegno il prato inferiore
    stampa_terreno(riga_inizio_prato, riga_fine_prato, 1);
}
