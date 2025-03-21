//created by Thomas Nieddu on 21/02/25.
#include "padre.h"
#include <pthread.h>
#include <signal.h>
#include "npc_rana.h"
#define FORCED_UPDATE 999
#define MAX_PROIETTILI_TANA 50

//indica se il gioco è in pausa
bool pausa = false;
//qui salvo il nickname del giocatore
const char *nickname;
//questo è il buffer circolare condiviso
extern buffer_circolare buf_principale;

/*
 * loop principale.
 * qui gestisco la logica di gioco, la lettura di messaggi, la pausa e il disegno.
 */


static int bulletTanaId = 0;

void sparaProiettileDallaTana(int idxTana) {
    // Calcolo x e y di partenza
    int x_inizio_tana = gioco_sinistra + offset_tane + idxTana*(larghezza_tana+spazio);
    int x_centro_tana = x_inizio_tana + larghezza_tana/2;
    int y_centro_tana = (tana_inizio_riga + tana_fine_riga)/2;

    struct personaggio bullet;
    bullet.id = bulletTanaId++;
    bullet.posizione.x = x_centro_tana;
    bullet.posizione.y = y_centro_tana;
    // ... se vuoi altro
    bullet.destra = false; // se non serve
    bullet.lunghezza = 1;
    bullet.animation = false;

    TanaProjArgs *pargs = malloc(sizeof(TanaProjArgs));
    pargs->proiettile = bullet;

    // Creazione del thread:
    pthread_t tid;
    if (pthread_create(&tid, NULL, processo_proiettile_tana, pargs) != 0) {
        perror("pthread_create(proiettile_tana)");
        // gestione errore
    }
}


void funzione_padre(int spawn_colonna, int spawn_riga, pthread_t ranaThread)
{
    int n_coccodrilli = conta_coccodrilli();

    // alloco array coccodrilli
    struct personaggio *coccodrilli = calloc(n_coccodrilli, sizeof(struct personaggio));
    if (coccodrilli == NULL) {
        fprintf(stderr, "Errore: allocazione di memoria fallita per coccodrilli.\n");
        exit(1);
    }
    // li avvio
    spawn_coccodrilli(coccodrilli, n_coccodrilli);

    // array proiettili orizzontali (uno per coccodrillo)
    struct personaggio *proiettili = calloc(n_coccodrilli, sizeof(struct personaggio));
    if (proiettili == NULL) {
        fprintf(stderr, "Errore: allocazione di memoria fallita per proiettili.\n");
        exit(2);
    }
    for (int i = 0; i < n_coccodrilli; i++) {
        proiettili[i].posizione.x = INIZIOPRIOIETTILI;
        proiettili[i].posizione.y = INIZIOPRIOIETTILI;
        proiettili[i].tipo        = PROIETTILE;
        proiettili[i].lunghezza   = 1;
        proiettili[i].id          = i;
        proiettili[i].thread_id   = 0;
        proiettili[i].animation   = false;
    }

    // *** Nuovo array di proiettili tana verticali ***
    struct personaggio proiettiliTana[MAX_PROIETTILI_TANA];
    for (int i = 0; i < MAX_PROIETTILI_TANA; i++) {
        proiettiliTana[i].posizione.x = INIZIOPRIOIETTILI;
        proiettiliTana[i].posizione.y = INIZIOPRIOIETTILI;
        proiettiliTana[i].tipo        = PROIETTILE_TANA; // supponiamo tu abbia definito PROIETTILE_TANA
        proiettiliTana[i].lunghezza   = 1;
        proiettiliTana[i].id          = i;  // indice dell'array
        proiettiliTana[i].thread_id   = 0;
        proiettiliTana[i].animation   = false;
    }

    // array di granate
    struct personaggio granate[GRANATE_MASSIME];
    for (int i = 0; i < GRANATE_MASSIME; i++) {
        granate[i].posizione.x = INIZIOPRIOIETTILI;
        granate[i].posizione.y = INIZIOPRIOIETTILI;
        granate[i].tipo        = GRANATA;
        granate[i].id          = INIZIOPRIOIETTILI;
        granate[i].thread_id   = 0;
        granate[i].animation   = false;
    }

    // struct della rana
    struct personaggio rana;
    rana.tipo        = RANA;
    rana.lunghezza   = LUNGHEZZA_RANA;
    rana.posizione.x = spawn_colonna;
    rana.posizione.y = spawn_riga;
    rana.thread_id   = 0;
    rana.animation   = false;

    bool fine_gioco = false;
    bool vittoria   = false;
    nodelay(stdscr, true);

    // timer
    struct timeval tempo_iniziale;
    gettimeofday(&tempo_iniziale, NULL);
    long ultimo_secondo = 0;

    while (!fine_gioco) {
        // 1) Leggo i messaggi se non in pausa
        if (!pausa) {
            struct personaggio rec;
            if (rand() % 1000 < 5) {
                int tanaCasuale = rand() % num_tane;
                sparaProiettileDallaTana(tanaCasuale);
            }
            while (consume_messaggio_non_bloccante(&buf_principale, &rec)) {
                // aggiorno coccodrilli, proiettili, granate, proiettiliTana...
                switch (rec.tipo) {
                    case RANA:
                        rana = rec;
                        break;
                    case COCCODRILLO:
                        coccodrilli[rec.id] = rec;
                        break;
                    case PROIETTILE:
                        // se id < n_coccodrilli => orizzontali
                        proiettili[rec.id] = rec;
                        break;
                    case PROIETTILE_TANA:
                        if (rec.id >= 0 && rec.id < MAX_PROIETTILI_TANA) {
                            proiettiliTana[rec.id] = rec;
                        }
                        break;
                    case GRANATA: {
                        if (rec.id >= 0 && rec.id < GRANATE_MASSIME) {
                            granate[rec.id] = rec;
                        }
                    } break;
                    default:
                        break;
                }
                // Se un proiettileTana colpisce la rana => distruggiamo
                if (rec.tipo == PROIETTILE_TANA) {
                    // controlliamo collisione con la rana
                    if (abs(rec.posizione.x - rana.posizione.x) < 2
                        && rec.posizione.y == rana.posizione.y)
                    {
                        // si distruggono a vicenda
                        pthread_cancel(rec.thread_id); // kill proiettile
                        // la rana subisce reset
                        perdivita();
                        if (vite == 0) {
                            fine_gioco = true;
                        } else {
                            reset_scenario(&ranaThread, &rana,
                                           coccodrilli, n_coccodrilli,
                                           proiettili, granate,
                                           spawn_colonna, spawn_riga);
                        }
                    }
                }
            }
        }

        // 2) Tasto p => pausa
        int tasto = getch();
        switch(tasto) {
            case 'p':
            case 'P':
                pausa = true;
                nodelay(stdscr, false);
                flushinp();
                while (pausa) {
                    erase();
                    attron(COLOR_PAIR(3) | A_BOLD);
                    mvprintw(max_righe/2, (max_colonne - 9)/2, "[ PAUSA ]");
                    attroff(COLOR_PAIR(3) | A_BOLD);

                    int ch = getch();
                    if (ch == 'p' || ch == 'P') {
                        pausa = false;
                        flushinp();
                        nodelay(stdscr, true);
                        gettimeofday(&tempo_iniziale, NULL);
                        ultimo_secondo = 0;
                        break;
                    }
                    usleep(SLEEP);
                }
                break;

                // 3) Se tasti di movimento => mandarli alla rana
            case KEY_UP:
            case KEY_DOWN:
            case KEY_LEFT:
            case KEY_RIGHT:
            case ' ':
                if (!pausa) {
                    struct personaggio input;
                    input.tipo = RANA;
                    input.id   = tasto;
                    input.lunghezza   = 0;
                    input.posizione.x = -1;
                    input.posizione.y = -1;
                    input.animation   = false;
                    produce_messaggio(&buf_principale, input);
                }
                break;
            default:
                break;
        }

        // 4) se non in pausa => logica di coccodrilli, collisioni, timer, ...
        if (!pausa) {
            // coccodrillo + rana
            controllo_coccodrilli(&rana, coccodrilli, n_coccodrilli);

            bool diedOrNest = false;
            if (controllo_bordi(rana) || check_tane(rana)) {
                diedOrNest = true;
            }
            if (diedOrNest) {
                if (vite == 0) {
                    fine_gioco = true;
                } else {
                    reset_scenario(&ranaThread, &rana,
                                   coccodrilli, n_coccodrilli,
                                   proiettili, granate,
                                   spawn_colonna, spawn_riga);
                }
            } else {
                // forced update
                struct personaggio forced;
                forced.tipo = RANA;
                forced.id   = FORCED_UPDATE;
                forced.lunghezza   = rana.lunghezza;
                forced.posizione.x = rana.posizione.x;
                forced.posizione.y = rana.posizione.y;
                forced.animation   = rana.animation;
                produce_messaggio(&buf_principale, forced);
            }

            // se tutte tane chiuse => vittoria
            if (tutte_tane_chiuse()) {
                fine_gioco = true;
                vittoria   = true;
            }



            // timer
            struct timeval tempo_attuale;
            gettimeofday(&tempo_attuale, NULL);
            long secondi_passati = tempo_attuale.tv_sec - tempo_iniziale.tv_sec;
            if (secondi_passati > ultimo_secondo) {
                ultimo_secondo = secondi_passati;
                tempo_rimasto--;
                if (tempo_rimasto < 0) {
                    perdivita();
                    if (vite == 0) {
                        fine_gioco = true;
                    } else {
                        reset_scenario(&ranaThread, &rana,
                                       coccodrilli, n_coccodrilli,
                                       proiettili, granate,
                                       spawn_colonna, spawn_riga);
                    }
                    gettimeofday(&tempo_attuale, NULL);
                    ultimo_secondo = 0;
                }
            }
        }

        // 5) disegno
        erase();
        disegna_scenario();

        // coccodrilli
        for (int i = 0; i < n_coccodrilli; i++) {
            disegna_coccodrillo(coccodrilli[i]);
        }
        // proiettili orizzontali
        for (int i = 0; i < n_coccodrilli; i++) {
            disegna_proiettile(proiettili[i], n_coccodrilli);
        }
        // granate
        for (int i = 0; i < GRANATE_MASSIME; i++) {
            if (granate[i].posizione.x >= 0) {
                disegna_granata(granate[i]);
            }
        }

        for (int i = 0; i < MAX_PROIETTILI_TANA; i++) {
            if (proiettiliTana[i].posizione.x != INIZIOPRIOIETTILI
                && proiettiliTana[i].posizione.y != INIZIOPRIOIETTILI)
            {
                disegna_proiettile_tana(proiettiliTana[i], MAX_PROIETTILI_TANA);
            }
        }

        // la rana
        disegna_sprite(rana);

        // info e timer
        disegna_info();
        disegna_timer();

        if (pausa) {
            attron(COLOR_PAIR(3) | A_BOLD);
            mvprintw(max_righe/2, (max_colonne - 9)/2, "[ PAUSA ]");
            attroff(COLOR_PAIR(3) | A_BOLD);
        }

        doupdate();

        if (vite == 0) {
            fine_gioco = true;
        }
        usleep(SLEEP);
    }

    // fine loop
    kill_coccodrilli(n_coccodrilli);
    free(coccodrilli);
    free(proiettili);

    // kill la rana
    kill_rana(ranaThread);

    // gameover / vittoria
    if (vite == 0) {
        GameOver();
    } else if (vittoria) {
        Vittoria();
    }
}

/*
 * cancello i thread dei coccodrilli
 */
void kill_coccodrilli(int n_coccodrilli) {
    if (!coccodrilliThreads) return;
    for (int i = 0; i < n_coccodrilli; i++) {
        if (coccodrilliThreads[i] != 0) {
            pthread_cancel(coccodrilliThreads[i]);
            pthread_join(coccodrilliThreads[i], NULL);
            coccodrilliThreads[i] = 0;
        }
    }
}

/*
 * cancello e joino il thread della rana
 */
void kill_rana(pthread_t ranaThread) {
    if (ranaThread != 0) {
        pthread_cancel(ranaThread);
        pthread_join(ranaThread, NULL);
    }
}

/*
 * scorro e cancello i thread proiettile
 */
void kill_all_proiettili(struct personaggio *proiettili, int n_coccodrilli) {
    for (int i = 0; i < n_coccodrilli; i++) {
        if (proiettili[i].thread_id != 0) {
            pthread_cancel(proiettili[i].thread_id);
            pthread_join(proiettili[i].thread_id, NULL);
            proiettili[i].thread_id = 0;
        }
    }
}

/*
 * scorro e cancello i thread granate
 */
void kill_all_granate(struct personaggio *granate, int n_granate) {
    for (int i = 0; i < n_granate; i++) {
        if (granate[i].thread_id != 0) {
            pthread_cancel(granate[i].thread_id);
            pthread_join(granate[i].thread_id, NULL);
            granate[i].thread_id = 0;
        }
    }
}

/*
 * killo tutto e ricreo la rana, i coccodrilli e le strutture
 */
void reset_scenario(pthread_t *pRanaThread,
                    struct personaggio *rana,
                    struct personaggio *coccodrilli,
                    int n_coccodrilli,
                    struct personaggio *proiettili,
                    struct personaggio *granate,
                    int spawn_colonna, int spawn_riga)
{
    //killo la rana e i coccodrilli, e libero proiettili e granate
    kill_rana(*pRanaThread);
    kill_coccodrilli(n_coccodrilli);
    kill_all_proiettili(proiettili, n_coccodrilli);
    kill_all_granate(granate, GRANATE_MASSIME);

    //reinizializzo il buffer
    init_buffer(&buf_principale);

    //rimetto i proiettili e le granate a x=-1
    for (int i = 0; i < n_coccodrilli; i++) {
        proiettili[i].posizione.x = INIZIOPRIOIETTILI;
        proiettili[i].posizione.y = INIZIOPRIOIETTILI;
        proiettili[i].thread_id = 0;
        proiettili[i].animation = false;
    }
    for (int i = 0; i < GRANATE_MASSIME; i++) {
        granate[i].posizione.x = INIZIOPRIOIETTILI;
        granate[i].posizione.y = INIZIOPRIOIETTILI;
        granate[i].thread_id = 0;
        granate[i].animation = false;
    }

    //ricreo i coccodrilli
    spawn_coccodrilli(coccodrilli, n_coccodrilli);

    //resetto la rana
    reset_rana(rana, spawn_colonna, spawn_riga);

    //ricreo il thread della rana
    RanaArgs *ra = malloc(sizeof(RanaArgs));
    if (ra == NULL) {
        fprintf(stderr, "Errore: allocazione di memoria fallita per ra.\n");
        exit(4);
    }
    ra->spawn_riga = spawn_riga;
    ra->spawn_colonna = spawn_colonna;
    if (pthread_create(pRanaThread, NULL, wrapper_rana, ra) != 0) {
        endwin();
        perror("pthread_create(rana)");
        exit(1);
    }
}

/*
 * ddisegno un piccolo blocco per indicare la vita
 */
void disegna_quadrato_vita(int riga, int colonna) {
    move(riga, colonna);
    addch(' ');
    attron(COLOR_PAIR(7));
    addstr("__");
    attroff(COLOR_PAIR(7));
    addch(' ');

    move(riga + 1, colonna);
    addch('|');
    attron(COLOR_PAIR(7));
    addstr("__");
    attroff(COLOR_PAIR(7));
    addch('|');
}

/*
 * disegno il punteggio, il nickname e le vite
 */
void disegna_info() {
    attron(COLOR_PAIR(6) | A_BOLD);
    mvprintw(5, 0, "Username: ");
    attroff(A_BOLD);
    printw("%s", nickname);

    attron(A_BOLD);
    mvprintw(3, 0, "Punteggio: ");
    attroff(A_BOLD);
    printw("%d", punteggio);
    attroff(COLOR_PAIR(6));

    int vite_colonna = max_colonne - 7;
    if (vite_colonna < 0) vite_colonna = 0;

    int vite_riga_inizio = 5;
    mvprintw(vite_riga_inizio, vite_colonna, "VITE:");

    for(int v = 0; v < vite; v++) {
        int riga_blocco = vite_riga_inizio + 2 + v * 4;
        disegna_quadrato_vita(riga_blocco, vite_colonna);
    }
}

/*
 * disegno una barra che va a decremento man mano che il tempo scorre
 */
void disegna_timer() {
    int dimensione_barra = 50 * tempo_rimasto / TEMPO_MASSIMO;
    const char *etichetta = "Timer: ";
    int lung_etichetta = (int)strlen(etichetta);
    int riga_in_basso  = max_righe - 1;

    attron(COLOR_PAIR(4));
    mvprintw(riga_in_basso, 5, "%s", etichetta);

    for(int i = 0; i < dimensione_barra; i++) {
        mvprintw(riga_in_basso, 5 + lung_etichetta + i, "|");
    }
    attroff(COLOR_PAIR(4));
}

/*
 * schermata di game over che aspetta barra spaziatrice
 */
void GameOver() {
    nodelay(stdscr, false);
    clear();

    attron(COLOR_PAIR(3) | A_BOLD);
    const char *msgGameOver = "[GAME OVER]";
    int xGameOver = (max_colonne - (int)strlen(msgGameOver)) / 2;
    int yGameOver = max_righe / 2;
    mvprintw(yGameOver, xGameOver, "%s", msgGameOver);
    attroff(COLOR_PAIR(3) | A_BOLD);

    char msgPunteggio[100];
    snprintf(msgPunteggio, sizeof(msgPunteggio),
             "Hai terminato la partita con: %d punti", punteggio);
    int xPunteggio = (max_colonne - (int)strlen(msgPunteggio)) / 2;
    mvprintw(yGameOver + 1, xPunteggio, "%s", msgPunteggio);

    const char *msgPressSpace = "(premi la barra spaziatrice per continuare)";
    int xPressSpace = (max_colonne - (int)strlen(msgPressSpace)) / 2;
    mvprintw(yGameOver + 3, xPressSpace, "%s", msgPressSpace);

    refresh();
    int ch;
    do {
        ch = getch();
    } while(ch != ' ');
    clear();
    refresh();
}

/*
 * schermata di vittoria che aspetta barra spaziatrice
 */
void Vittoria() {
    nodelay(stdscr, false);
    clear();

    attron(COLOR_PAIR(10) | A_BOLD);
    const char *msgVictory = "[HAI VINTO]";
    int xVictory = (max_colonne - (int)strlen(msgVictory)) / 2;
    int yVictory = max_righe / 2;
    mvprintw(yVictory, xVictory, "%s", msgVictory);
    attroff(COLOR_PAIR(10) | A_BOLD);

    char msgPunteggio[100];
    snprintf(msgPunteggio, sizeof(msgPunteggio),
             "Hai terminato la partita con: %d punti", punteggio);
    int xPunteggio = (max_colonne - (int)strlen(msgPunteggio)) / 2;
    mvprintw(yVictory + 1, xPunteggio, "%s", msgPunteggio);

    const char *msgPressSpace = "(premi la barra spaziatrice per continuare)";
    int xPressSpace = (max_colonne - (int)strlen(msgPressSpace)) / 2;
    mvprintw(yVictory + 3, xPressSpace, "%s", msgPressSpace);

    refresh();
    int ch;
    do {
        ch = getch();
    } while(ch != ' ');
    clear();
    refresh();
}
