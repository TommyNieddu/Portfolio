#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include <unistd.h>
#include <stdbool.h>
#include "prato_tane.h"

#include "padre.h"

#define DIM_NOME 20   //lunghezza massima del nome utente

//vite punteggio timer, pipe , fork, funzione padre, logica principale
void inizio(const char *nome_utente) {
    flushinp();
    clear();
    curs_set(0);

    //calcolo dimensioni dello schermo
    getmaxyx(stdscr, max_righe, max_colonne);

    //limiti orizzontali del campo di gioco
    gioco_sinistra = LIMITESINISTRA;
    gioco_destra = max_colonne - LIMITEDESTRA;
    larghezza_gioco = (gioco_destra - gioco_sinistra + 1);

    //reimposto i parametri di gioco
    punteggio = 0;
    vite = VITEINIZIALI;
    tempo_rimasto = TEMPO_MASSIMO;
    pausa = false;
    nickname = nome_utente;

    //prato in basso
    riga_inizio_prato = (max_righe - 2) - (prato_altezza - 1);
    riga_fine_prato = riga_inizio_prato + (prato_altezza - 1);
    if (riga_inizio_prato % 2 != 0) {
        riga_inizio_prato--;
    }
    riga_fine_prato = riga_inizio_prato + (prato_altezza - 1);
    if (riga_fine_prato % 2 != 0) {
        riga_fine_prato++;
    }

    //zona tane
    tana_inizio_riga = 2;
    tana_fine_riga = tana_inizio_riga + tana_altezza - 1;

    //secondo prato sopra la rana
    prato2_inizio_riga = tana_fine_riga + 1;
    prato2_fine_riga = prato2_inizio_riga + prato2_altezza - 1;

    //dimensioni e offset delle tane
    spazio_totale = (num_tane - 1) * spazio;
    totale_per_le_tane = larghezza_gioco - spazio_totale;
    larghezza_tana = totale_per_le_tane / num_tane;
    offset_tane = (totale_per_le_tane % num_tane) / 2;

    //posizione di spawn della rana
    int prato_y_centrale = (riga_inizio_prato + riga_fine_prato - rana_altezza) / 2;
    int prato_x_centrale = gioco_sinistra + (larghezza_gioco - 3) / 2;
    if (prato_y_centrale % 2 != 0) {
        prato_y_centrale--;
    }

    //apro tutte le tane
    for (int i = 0; i < num_tane; i++) {
        tane_aperte[i] = true;
    }

    //creo le pipe padre→figlio e figlio→padre
    if (pipe(canale_a_figlio) == -1 || pipe(canale_a_padre) == -1) {
        endwin();
        perror("Errore pipe");
        return;
    }

    //fork per generare il figlio processo rana
    pid_t pid = fork();
    if (pid == -1) {
        endwin();
        perror("Errore fork");
        return;
    }
    else if (pid == 0) {
        //RANA
        close(canale_a_figlio[1]); //chiudo lato scrittura
        close(canale_a_padre[0]);  //chiudo lato lettura
        //logica della rana
        processo_rana(prato_y_centrale, prato_x_centrale);
        _exit(0);
    }

    //PADRE
    //avvio la funzione di gestione principale
    funzione_padre(prato_x_centrale, prato_y_centrale, pid);

    return;
}

void menu() {
    char *opzioni[] = {"Inizia a giocare", "Esci"};
    int scelta = 0;
    int tasto;
    char nome_utente[DIM_NOME];
    int num_opzioni;

    getmaxyx(stdscr, max_righe, max_colonne);
    num_opzioni = sizeof(opzioni) / sizeof(opzioni[0]);

    nodelay(stdscr, false);
    keypad(stdscr, true);

    while (true) {
        clear();
        attron(COLOR_PAIR(2));
        mvprintw(max_righe / 2 - 3, (max_colonne - 33) / 2,
                 "#################################");
        mvprintw(max_righe / 2 - 2, (max_colonne - 33) / 2,
                 "#       Frogger Resurrection    #");
        mvprintw(max_righe / 2 - 1, (max_colonne - 33) / 2,
                 "#################################");
        attroff(COLOR_PAIR(2));

        //opzioni di menu
        for (int i = 0; i < num_opzioni; i++) {
            if (i == scelta) {
                attron(A_REVERSE | COLOR_PAIR(2));
            }
            mvprintw(max_righe / 2 + i + 1,
                     (max_colonne - (int)strlen(opzioni[i])) / 2,
                     "%s", opzioni[i]);
            if (i == scelta) {
                attroff(A_REVERSE | COLOR_PAIR(2));
            }
        }

        tasto = getch();
        switch (tasto) {
            case KEY_UP:
                scelta = (scelta - 1 + num_opzioni) % num_opzioni;
                break;
            case KEY_DOWN:
                scelta = (scelta + 1) % num_opzioni;
                break;
            case 10: //invio
            case 13:
            case KEY_ENTER:
                if (scelta == 0) {
                    //inizia partita
                    if (max_righe < MINSCHERMO || max_colonne < MINSCHERMO) {
                        clear();
                        mvprintw(max_righe / 2, max_colonne / 2 - 20,
                                 "La finestra è troppo piccola per giocare!");
                        mvprintw(max_righe / 2 + 2, max_colonne / 2 - 15,
                                 "Ridimensiona la finestra e riprova.");
                        refresh();
                        getch();
                        clear();
                    } else {
                        clear();
                        attron(A_BOLD);
                        mvprintw(max_righe / 2 - 1, (max_colonne - 25) / 2,
                                 "INSERISCI IL TUO USERNAME:");
                        attroff(A_BOLD);

                        echo();
                        curs_set(1);
                        int colonna_inizio = (max_colonne - DIM_NOME) / 2;
                        mvprintw(max_righe / 2 + 1, colonna_inizio, " ");
                        mvgetnstr(max_righe / 2 + 1, colonna_inizio,
                                  nome_utente, DIM_NOME - 1);
                        noecho();
                        curs_set(0);

                        // Se il nome utente è vuoto, avviso
                        if (strlen(nome_utente) == 0) {
                            attron(A_BOLD);
                            mvprintw(max_righe / 2 - 2,
                                     (max_colonne - 20) / 2,
                                     "SELEZIONARE UN NOME CORRETTO");
                            attroff(A_BOLD);
                            refresh();
                            getch();
                            clear();
                        } else {
                            //avvio la partita
                            inizio(nome_utente);
                            // Al ritorno da inizio la partita è terminata
                            // e siamo di nuovo qui nel menu.
                        }
                    }
                }
                else if (scelta == 1) {
                    clear();
                    mvprintw(max_righe / 2,
                             (max_colonne - 28) / 2,
                             "Grazie per aver giocato!");
                    refresh();
                    getch();
                    return; //chiudo il menu -> fine main
                }
                break;
            default:
                break;
        }
    }
}

//inizializzo ncurses, colori, e avvio il menu
int main() {
    initscr();
    start_color();
    keypad(stdscr, true);

    //colori
    init_pair(1,  COLOR_BLACK,   COLOR_GREEN);
    init_pair(2,  COLOR_YELLOW,  COLOR_BLACK);
    init_pair(3,  COLOR_RED,     COLOR_BLACK);
    init_pair(4,  COLOR_CYAN,    COLOR_BLACK);
    init_pair(5,  COLOR_WHITE,   COLOR_BLACK);
    init_pair(6,  COLOR_MAGENTA, COLOR_BLACK);
    init_pair(7,  COLOR_WHITE,   COLOR_RED);
    init_pair(8,  COLOR_BLACK,   COLOR_YELLOW);
    init_pair(9,  COLOR_BLACK,   COLOR_BLACK);
    init_pair(10, COLOR_GREEN,   COLOR_BLACK);

    noecho();
    cbreak();
    curs_set(0);

    //avvio menu iniziale
    menu();

    endwin();
    return 0;
}
