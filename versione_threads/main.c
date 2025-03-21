//created by Thomas Nieddu on 21/02/25.
#include <stdio.h>
#include "padre.h"
#include <pthread.h>

//limite nome e dimensione finestra
#define DIM_NOME_UTENTE 20
#define DIM_MINIMA_SCHERMO 55

//buffer circolare condiviso
buffer_circolare buf_principale;

//funzione di supporto
//perché invocherò processo_rana passandogli i parametri di spawn
void* wrapper_rana(void *arg) {
    RanaArgs *rArgs = (RanaArgs*) arg;
    processo_rana(rArgs->spawn_riga, rArgs->spawn_colonna);
    free(rArgs);
    pthread_exit(NULL);
}

//questa funzione iniziofa partire una nuova partita
//calcolo i limiti del campo, resetto vite e punteggio,
//creo il thread della rana e poi chiamo funzione_padre
void inizio(const char *nome_utente) {
    clear();
    curs_set(0);

    //leggo righe e colonne dello schermo
    getmaxyx(stdscr, max_righe, max_colonne);

    //limiti campo di gioco
    gioco_sinistra = LIMITESINISTRA;
    gioco_destra = max_colonne - LIMITEDESTRA;
    larghezza_gioco = (gioco_destra - gioco_sinistra + 1);

    //reimposto parametri di gioco
    punteggio = 0;
    vite = VITEINIZIALI;
    tempo_rimasto = TEMPO_MASSIMO;
    pausa = false;
    nickname = nome_utente;

    //coordinate del prato in basso
    riga_inizio_prato = (max_righe - 2) - (prato_altezza - 1);
    riga_fine_prato = riga_inizio_prato + (prato_altezza - 1);
    if (riga_inizio_prato % 2 != 0)  riga_inizio_prato--;
    riga_fine_prato = riga_inizio_prato + (prato_altezza - 1);
    if (riga_fine_prato % 2 != 0)    riga_fine_prato++;

    //zona delle tane
    tana_inizio_riga = 2;
    tana_fine_riga = tana_inizio_riga + tana_altezza - 1;

    //secondo prato
    prato2_inizio_riga = tana_fine_riga + 1;
    prato2_fine_riga = prato2_inizio_riga + prato2_altezza - 1;

    //dimensioni e offset delle tane
    spazio_totale = (num_tane - 1) * spazio;
    totale_per_le_tane = larghezza_gioco - spazio_totale;
    larghezza_tana = totale_per_le_tane / num_tane;
    offset_tane = (totale_per_le_tane % num_tane) / 2;

    //calcolo coordinate spawn per la rana
    int prato_y_centrale = (riga_inizio_prato + riga_fine_prato - rana_altezza) / 2;
    int prato_x_centrale = gioco_sinistra + (larghezza_gioco - 3) / 2;
    if (prato_y_centrale % 2 != 0)  prato_y_centrale--;

    //apro tutte le tane
    for(int i = 0; i < num_tane; i++){
        tane_aperte[i] = true;
    }

    //creo il thread della rana
    pthread_t thread_rana_id;
    RanaArgs *ra = malloc(sizeof(RanaArgs));
    if (ra == NULL) {
        fprintf(stderr, "Errore: allocazione di memoria fallita per ra.\n");
        exit(3);
    }
    ra->spawn_riga    = prato_y_centrale;
    ra->spawn_colonna = prato_x_centrale;

    //avvio del thread
    if (pthread_create(&thread_rana_id, NULL, wrapper_rana, ra) != 0) {
        endwin();
        perror("pthread_create(rana)");
        exit(1);
    }

    //funzione_padre per gestire la logica di gioco
    funzione_padre(prato_x_centrale, prato_y_centrale, thread_rana_id);
}

//mostro il menu iniziale.
//se seleziono "gioca", chiamo inizio; al ritorno,
//posso nuovamente scegliere dal menu
void menu() {
    flushinp();

    char *opzioni[] = {"Inizia a giocare", "Esci"};
    int  scelta = 0;
    int  tasto;
    char nome_utente[DIM_NOME_UTENTE];
    int  num_opzioni;

    getmaxyx(stdscr, max_righe, max_colonne);
    num_opzioni = sizeof(opzioni) / sizeof(opzioni[0]);

    nodelay(stdscr, false);
    keypad(stdscr, true);

    //ciclo finché non decido di uscire
    while(true) {
        clear();
        attron(COLOR_PAIR(2));
        mvprintw(max_righe/2 - 3, (max_colonne - 33)/2,
                 "#################################");
        mvprintw(max_righe/2 - 2, (max_colonne - 33)/2,
                 "#       Frogger Resurrection    #");
        mvprintw(max_righe/2 - 1, (max_colonne - 33)/2,
                 "#################################");
        attroff(COLOR_PAIR(2));

        //disegno le opzioni
        for(int i = 0; i < num_opzioni; i++){
            if(i == scelta) {
                attron(A_REVERSE | COLOR_PAIR(2));
            }
            mvprintw(max_righe/2 + i + 1,
                     (max_colonne - (int)strlen(opzioni[i])) / 2,
                     "%s", opzioni[i]);
            if(i == scelta) {
                attroff(A_REVERSE | COLOR_PAIR(2));
            }
        }

        tasto = getch();
        switch(tasto) {
            case KEY_UP:
                scelta = (scelta - 1 + num_opzioni) % num_opzioni;
                break;
            case KEY_DOWN:
                scelta = (scelta + 1) % num_opzioni;
                break;

                //invio
            case 10:
            case 13:
            case KEY_ENTER:
                if(scelta == 0) {
                    //inizia a giocare
                    if(max_righe < DIM_MINIMA_SCHERMO || max_colonne < DIM_MINIMA_SCHERMO) {
                        clear();
                        mvprintw(max_righe/2,   max_colonne/2 - 20,
                                 "La finestra è troppo piccola per giocare!");
                        mvprintw(max_righe/2+2, max_colonne/2 - 15,
                                 "Ridimensiona la finestra e riprova.");
                        refresh();
                        getch();
                        clear();
                    } else {
                        clear();
                        attron(A_BOLD);
                        mvprintw(max_righe/2 - 1, (max_colonne - 25)/2,
                                 "INSERISCI IL TUO USERNAME:");
                        attroff(A_BOLD);

                        echo();
                        curs_set(1);
                        mvgetnstr(max_righe/2 + 1,(max_colonne - 50)/2,
                                  nome_utente, DIM_NOME_UTENTE - 1);
                        noecho();
                        curs_set(0);

                        if(strlen(nome_utente) == 0) {
                            attron(A_BOLD);
                            mvprintw(max_righe/2 + 2, (max_colonne - 20)/2,
                                     "NOME NON VALIDO!");
                            attroff(A_BOLD);
                            refresh();
                            getch();
                            clear();
                        } else {
                            inizio(nome_utente);
                        }
                    }
                } else if(scelta == 1) {
                    //esco
                    clear();
                    mvprintw(3, 5, "Grazie per aver giocato!");
                    refresh();
                    getch();
                    return;
                }
                break;
            default:
                break;
        }
    }
}

int main() {
    initscr();
    start_color();
    keypad(stdscr, true);

    //inizializzo  colori
    init_pair(1, COLOR_BLACK,   COLOR_GREEN);
    init_pair(2, COLOR_YELLOW,  COLOR_BLACK);
    init_pair(3, COLOR_RED,     COLOR_BLACK);
    init_pair(4, COLOR_CYAN,    COLOR_BLACK);
    init_pair(5, COLOR_WHITE,   COLOR_BLACK);
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(7, COLOR_WHITE,   COLOR_RED);
    init_pair(8, COLOR_BLACK,   COLOR_YELLOW);
    init_pair(9, COLOR_BLACK,   COLOR_BLACK);
    init_pair(10, COLOR_GREEN,  COLOR_BLACK);
    noecho();
    cbreak();
    curs_set(0);

    //inizializzo buffer circolare condiviso
    init_buffer(&buf_principale);

    //avvio il menu iniziale
    menu();

    endwin();
    return 0;
}
