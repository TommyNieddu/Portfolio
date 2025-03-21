//
// Created by Thomas Nieddu on 22/02/24.
//

#ifndef NIEDDU_THOMAS_66401_MOTOREGIOCO_H
#define NIEDDU_THOMAS_66401_MOTOREGIOCO_H

#include <stdbool.h>
#include <stdio.h>
#include "Strutture.h"

void Menu(Giocatore** giocatori, int* n_giocatori, CartaOstacolo** ostacoli, Carte** carteCFU,bool* partitaCaricata);
Giocatore* allocaGiocatore(int i);
Giocatore* allocaGiocatori(int numerogiocatori);
Personaggio* leggipersonaggi(const char* fp, int* num_personaggi);
Carte* leggicarteCFU(const char* fp, int* numerocarte);
void RandomizzaPersonaggi(Personaggio* personaggi, int numeropersonaggi);
void RandomizzaMazzo(Carte* mazzo, int numerocarte);
void AssegnazionePersonaggi(Giocatore* giocatori, Personaggio* personaggi, int numerogiocatori);
void AssegnazioneCarteCfu(Giocatore* giocatori, int numerogiocatori, Carte* mazzo, int numerocarte);
void stampa_giocatori(Giocatore* lista, int numerogiocatori);
void MostraManoGiocatori(Giocatore* lista, int numerogiocatori);
CartaOstacolo* LeggiOstacoli(const char* fp);
CartaOstacolo* EstrazioneOstacolo(CartaOstacolo* testa);
void StampaOstacoloEstratto(CartaOstacolo* ostacolo);
void RiassegnaCarte(Giocatore* giocatore, Carte* mazzo, int numerocarte);
void StampaCFU(Giocatore* testa, int numerogiocatori);
void RinunciaAgliStudi(Giocatore** attuale, Giocatore** testa);
void controllaNumeroDiGiocatori(int n_giocatori);
int EseguiScelta(int scelta, Giocatore** attuale, Giocatore** testa, int* numerogiocatori,  CartaGiocata* carteGiocate, int *i);
CartaGiocata* Giocata(Giocatore* testa, int* numerogiocatori, Carte* mazzo, int numerocarte);
void controllaMazzo(Carte* carteCFU);
void controllaPersonaggi(Personaggio* personaggi);
void controllaPunteggioTemporaneo(int* punteggioTemporaneo);
int richiestaNumeroGiocatori();
void stampaPunteggioTemporaneo(int *punteggioTemporaneo, int n_giocatori);
void CalcolaPunteggio(Giocatore * testa, int numerogiocatori, CartaOstacolo* ostacoloCorrente, CartaGiocata* carteGiocate, int* punteggioTemporaneo);
void StampaAnnuncioEffetti();
void StampaAnnuncioIstantanee();
void AggiungiCartaInFondoAlMazzo(Carte** testaMazzo, Carte cartaGiocata);
void GiocaIstantanee(Giocatore* testa, int numerogiocatori, CartaGiocata* carteGiocate, int* punteggioTemporaneo, CartaOstacolo* Attuale);
Giocatore* ricostruisciGiocatori(CartaGiocata* carteGiocate, int numerogiocatori);
void AssegnaCartaOstacoloAGiocatore(Giocatore* destinatario, CartaOstacolo* cartaDaAssegnare);
void AttivaEffettiCarte(CartaGiocata* carteGiocate, int numerogiocatori, int* punteggioTemporaneo,Carte** testaMazzo,int tipoOstacolo);
void GestisciVincitoreEPerdente(Giocatore* giocatori,int numerogiocatori,int* punteggioTemporaneo,CartaOstacolo* cartaEstratta,CartaGiocata* carteGiocate,CartaOstacolo** testaMazzoOstacoli,Carte* mazzo,int numerocarte );
void RimuoviGiocatori(Giocatore** testa);
void EseguiTurno(Giocatore** giocatori, int* n_giocatori, CartaOstacolo** ostacoli, Carte** carteCFU, int* CarteCFU, CartaGiocata** Giocate, int* punteggioTemporaneo, int* conta_vincitori, int* n_turno);
void InizializzaGioco(Giocatore** giocatori, int* n_giocatori, CartaOstacolo** ostacoli, Carte** carteCFU, int* punteggioTemporaneo, int* num_personaggi, Personaggio** personaggi, int* CarteCFU, char* personaggio, char* carta,bool* partitaCaricata);
void SalvaPartita(Giocatore* giocatori, int n_giocatori);
void CaricaPartita(Giocatore** testaa, int* n_giocatori);
#endif //NIEDDU_THOMAS_66401_MOTOREGIOCO_H
