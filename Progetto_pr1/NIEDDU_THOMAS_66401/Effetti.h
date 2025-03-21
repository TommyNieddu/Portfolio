//
// Created by Thomas Nieddu on 22/02/24.
//

#ifndef NIEDDU_THOMAS_66401_EFFETTI_H
#define NIEDDU_THOMAS_66401_EFFETTI_H

#include "Strutture.h"

void EffettoScartaP(Giocatore* giocatore, int* punteggioTemporaneo, int indiceGiocatoreAttuale);
void EffettoRuba(Giocatore* giocatori, int numerogiocatori, int indiceGiocatoreAttuale);
void EffettoScambiaP(Giocatore* giocatori, int* puntiTemporanei, int n_giocatori);
void EffettoSbircia(Giocatore* giocatoreAttuale, Carte** testaMazzo);
void EffettoScambiaDS(Giocatore* giocatori, int n_giocatori, int indiceGiocatoreAttuale);
void EffettoScartaE(Giocatore* giocatori, int indiceGiocatoreAttuale, int* punteggioTemporaneo, Carte** testaMazzoCFU);
void EffettoScambiaCFU(Giocatore* giocatori, int numerogiocatori);
void EffettoScartaCarte(Giocatore* giocatore);
void EffettoAnnulla(CartaGiocata* carteGiocate, int* punteggioTemporaneo, int numerogiocatori, int indiceTipoOstacolo);
void EffettoAumenta(Giocatore* giocatori, int numerogiocatori, int* punteggioTemporaneo);
void EffettoDiminuisci(Giocatore* giocatori, int numerogiocatori, int* punteggioTemporaneo);
void EffettoInverti(int* punteggioTemporaneo,int numerogiocatori);
void AggiungiInFondo(CartaOstacolo** testa, CartaOstacolo* nuovaCarta);
void EffettoSalva(CartaOstacolo** testaMazzoOstacoli, CartaOstacolo* cartaEstratta);
void EffettoDirotta(Giocatore* giocatori, int numerogiocatori, CartaOstacolo* cartaDaDirottare, int indiceGiocatoreCorrente);

#endif //NIEDDU_THOMAS_66401_EFFETTI_H
