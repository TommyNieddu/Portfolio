//created by Thomas Nieddu on 21/02/25.
#ifndef PROGETTO_SO_PRATO_TANE_H
#define PROGETTO_SO_PRATO_TANE_H

#include <stdbool.h>
#include <ncurses.h>
#include <pthread.h>
#include <signal.h>
//limitisinistra e limitedestra mi servono per avere bordi
#define LIMITESINISTRA 30
#define LIMITEDESTRA 31

//queste variabili mper l’ampiezza dello schermo
extern int max_righe;
extern int max_colonne;

//campo di gioco
extern int gioco_sinistra;
extern int gioco_destra;
extern int larghezza_gioco;

//primo prato in basso
extern int prato_altezza;
extern int riga_inizio_prato;
extern int riga_fine_prato;

//secondo prato
extern int prato2_altezza;
extern int prato2_inizio_riga;
extern int prato2_fine_riga;

//tane
extern int tana_altezza;
extern int tana_inizio_riga;
extern int tana_fine_riga;
extern int num_tane;
extern int spazio;
extern int spazio_totale;
extern int totale_per_le_tane;
extern int larghezza_tana;
extern int offset_tane;

//array che mi dice quali tane sono aperte
extern bool tane_aperte[5];

//disegno
void stampa_terreno(int r, int stop, int colore);
void disegna_scenario();

#endif
