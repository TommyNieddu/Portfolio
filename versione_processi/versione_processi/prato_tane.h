//
// Created by Davide Balestrino on 19/12/24.
//

#ifndef PROGETTO_SO_PRATO_TANE_H
#define PROGETTO_SO_PRATO_TANE_H
#include <ncurses.h>
#include <sys/wait.h>
#include <signal.h>
#define LIMITESINISTRA 30
#define LIMITEDESTRA 31
#define MINSCHERMO 35

extern int max_righe;  //numero di righe del terminale
extern int max_colonne;  // numero di colonne del terminale

//campo di gioco orizzontale
extern int gioco_sinistra; //da dove parte il campo di gioco sulla sinistra
extern int gioco_destra; //dove finisce il campo di gioco a destra
extern int larghezza_gioco;   //quanti caratteri "larghi" è il campo orizzontalmente

//prato inferiore (alto 6)
extern int prato_altezza; //il  prato in fondo allo schermo è alto 6 righe
extern int riga_inizio_prato;
extern int riga_fine_prato;

//secondo prato dopo le tane (anche lui alto 6)
extern int prato2_altezza;
extern int prato2_inizio_riga;
extern int prato2_fine_riga;

//parametri delle tane
extern int tana_altezza; //le tane sono alte 5 righe
extern int tana_inizio_riga;
extern int tana_fine_riga;
extern int num_tane; //quante tane voglio
extern int spazio;  //distanza orizzontale tra ogni tana
extern int spazio_totale;  //ci calcolo lo spazio totale occupato
extern int totale_per_le_tane; //quanta larghezza ho a disposizione per le tane
extern int larghezza_tana; //larghezza della singola tana
extern int offset_tane;    //offset iniziale orizzontale, se non si dividono perfettamente
//mi serve per allineare bene le tane evitando che l’ultima risulti tagliata o vada a finire fuori dallo spazio di gioco

extern bool tane_aperte[5]; //array bool per sapere se la tana i è aperta o no

void stampa_terreno(int r, int stop, int colore);
void disegna_scenario();
#endif //PROGETTO_SO_PRATO_TANE_H

