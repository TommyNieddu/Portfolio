#ifndef NIEDDU_THOMAS_66401_STRUTTURE_H
#define NIEDDU_THOMAS_66401_STRUTTURE_H
#define DIM 32
#define FINEGIOCO 60
#define DES 128
#define NUMMIN 2
#define NUMMAX 4
#define INIZIO 0
#define MENUMIN 1
#define MENUMAX 3
#define MAZZOCFU 24
#define EFF_MAX 9
#define MANO 5
#define TESTO_MAGENTA "\033[0;35m"
#define TESTO_GIALLO "\033[0;33m"
#define TESTO_STANDARD "\033[0m"
#define TESTO_ROSSO "\033[0;31m"
#define TESTO_CIANO "\033[0;36m"
#define TESTO_VERDE "\033[0;32m"

typedef enum {STUDIO, SOPRAVVIVENZA, SOCIALE, ESAME} TipoOstacolo;
typedef enum {NESSUNO=INIZIO,SCARTAP,RUBA,SCAMBIADS,SCARTAE,SCARTAC,SCAMBIAP,DOPPIOE,SBIRCIA,SCAMBIAC,ANNULLA,AUMENTA,DIMINUISCI,INVERTI,SALVA,DIROTTA}Effetto;

typedef struct carte {
    char nome_carta[DIM];
    int occorrenza;
    int numeroCFU;
    int tipologia;
    struct carte *next;
} Carte;

typedef struct carta_ostacolo {
    char nome_carta_ostacolo[DIM];
    char descrizione_ostacolo[DES];
    TipoOstacolo tipoOstacolo;
    struct carta_ostacolo *next;
} CartaOstacolo;


typedef struct {
    char nome[DIM];
    int BonusMalus[NUMMAX];
} Personaggio;


typedef struct giocatore {
    char* nomeUtente;
    Personaggio personaggio;
    int numeroCFU;
    Carte *listaCarteCFU;
    CartaOstacolo *listaCarteOstacolo;
    struct giocatore *next;
} Giocatore;

typedef struct {
    Giocatore* giocatore;
    Carte carta;
} CartaGiocata;


void CreaCronologia();
void Trascrivi(char* evento);
void TerminaCronologia();
void Titolo();
void PuliziaTerminale();


#endif //NIEDDU_THOMAS_66401_STRUTTURE_H