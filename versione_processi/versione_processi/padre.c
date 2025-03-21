
#include "padre.h"
#include <sys/wait.h>
#include <signal.h>

int punteggio = 0;
bool pausa;
const char *nickname;

//fork multipli, logica generale di gioco
void funzione_padre(int spawn_colonna, int spawn_riga, pid_t pid_rana)
{
    //genero i coccodrilli e salvo i pid
    int n_coccodrilli = conta_coccodrilli();
    struct personaggio *coccodrilli = (struct personaggio*) calloc(n_coccodrilli, sizeof(struct personaggio));
    if (coccodrilli == NULL) {
        fprintf(stderr, "Errore: allocazione di memoria fallita per %d coccodrilli.\n", n_coccodrilli);
        exit(1);
    }
    pid_t *pidCoccodrilli = (pid_t*) calloc(n_coccodrilli, sizeof(pid_t));
    if (pidCoccodrilli == NULL) {
        fprintf(stderr, "Errore: allocazione di memoria fallita per %d pidCoccodrilli.\n", n_coccodrilli);
        exit(2);
    }
    spawn_coccodrilli(coccodrilli, n_coccodrilli, pidCoccodrilli);

    //preparo array granate e proiettili
    struct personaggio *granate = (struct personaggio*) calloc(MAX_GR_SCHERMO, sizeof(struct personaggio));
    if (granate == NULL) {
        fprintf(stderr, "Errore: allocazione di memoria fallita per %d granate.\n", MAX_GR_SCHERMO);
        exit(3);
    }
    pid_t *oldgranataPids = (pid_t*) calloc(MAX_GR_SCHERMO, sizeof(pid_t));
    if (oldgranataPids == NULL) {
        fprintf(stderr, "Errore: allocazione di memoria fallita per %d oldgranataPids.\n", MAX_GR_SCHERMO);
        exit(4);
    }
    for (int i = 0; i < MAX_GR_SCHERMO; i++) {
        granate[i].posizione.x = INIZIOPRIOIETTILI;
        granate[i].posizione.y = INIZIOPRIOIETTILI;
        granate[i].tipo        = GRANATA;
        granate[i].id          = INIZIOPRIOIETTILI;
        granate[i].pid_figlio  = 0;
    }

    struct personaggio *proiettili = (struct personaggio*) calloc(n_coccodrilli, sizeof(struct personaggio));
    if (proiettili == NULL) {
        fprintf(stderr, "Errore: allocazione di memoria fallita per %d proiettili.\n", n_coccodrilli);
        exit(5);
    }
    for (int i = 0; i < n_coccodrilli; i++) {
        proiettili[i].posizione.x = INIZIOPRIOIETTILI;
        proiettili[i].posizione.y = INIZIOPRIOIETTILI;
        proiettili[i].tipo        = PROIETTILE;
        proiettili[i].lunghezza   = 1;
        proiettili[i].id          = i;
        proiettili[i].pid_figlio  = 0;
    }

    //struttura per la rana
    struct personaggio rana;
    rana.tipo        = RANA;
    rana.lunghezza   = LUNGHEZZARANA;
    rana.posizione.x = spawn_colonna;
    rana.posizione.y = spawn_riga;
    rana.pid_figlio  = 0;

    bool fine_gioco = false;
    bool vittoria   = false;
    bool toggle     = false;
    nodelay(stdscr, true);

    //timer
    struct timeval tempo_iniziale, tempo_attuale;
    gettimeofday(&tempo_iniziale, NULL);
    long ultimo_secondo = 0;

    // *** CICLO DI GIOCO ***
    while (!fine_gioco && !vittoria) {
        //leggo pipe se non in pausa
        if (!pausa) {
            struct personaggio recupero;
            ssize_t n = read(canale_a_padre[0], &recupero, sizeof(struct personaggio));
            if (n == sizeof(struct personaggio)) {
                switch (recupero.tipo) {
                    case RANA:
                        rana = recupero;
                        break;
                    case COCCODRILLO:
                        coccodrilli[recupero.id] = recupero;
                        break;
                    case PROIETTILE:
                        proiettili[recupero.id] = recupero;
                        break;
                    case GRANATA: {
                        //aggiorno granata esistente o occupo slot libero
                        for(int g = 0; g < MAX_GR_SCHERMO; g++){
                            if(granate[g].id == recupero.id) {
                                granate[g] = recupero;
                                toggle = true;
                            }
                        }
                        if(!toggle){
                            for(int g = 0; g < MAX_GR_SCHERMO; g++){
                                if(granate[g].posizione.x == INIZIOPRIOIETTILI){
                                    granate[g] = recupero;
                                    break;
                                }
                            }
                        }
                        toggle = false;
                    } break;
                    default:
                        break;
                }
            }
        }

        //tasto p per pausa
        int tasto = getch();
        if (tasto == 'p' || tasto == 'P') {
            //metto in pausa
            pausa = true;


            toggle_pause_processes(true,
                                   pid_rana,
                                   pidCoccodrilli, n_coccodrilli,
                                   proiettili, n_coccodrilli,
                                   granate);

            //disabilito nodelay e pulisco input in coda
            nodelay(stdscr, false);
            flushinp();

            //loop di pausa
            while (pausa) {
                erase();
                attron(COLOR_PAIR(3) | A_BOLD);
                mvprintw(max_righe/2, (max_colonne - 9)/2, "[ PAUSA ]");
                attroff(COLOR_PAIR(3) | A_BOLD);
                refresh();

                int ch = getch();
                if (ch == 'p' || ch == 'P') {
                    //riprendo il gioco
                    pausa = false;
                    toggle_pause_processes(false,
                                           pid_rana,
                                           pidCoccodrilli, n_coccodrilli,
                                           proiettili, n_coccodrilli,
                                           granate);

                    nodelay(stdscr, true);
                    flushinp();

                    //resettotimer per non perdere secondi durante la pausa
                    gettimeofday(&tempo_iniziale, NULL);
                    ultimo_secondo = 0;
                }
                usleep(SLEEP);
            }
        }

        //logica di gioco se non in pausa
        if (!pausa) {
            //collisioni e controlli
            controllo_coccodrilli(&rana, coccodrilli, n_coccodrilli);

            if (controllo_bordi(rana) ||
                controllo_proiettili(rana, proiettili, n_coccodrilli) ||
                controllo_fiume(rana, coccodrilli, n_coccodrilli) ||
                check_tane(rana))
            {
                //se la rana muore o entra in una tana => reset
                reset_rana(&rana, spawn_colonna, spawn_riga, oldgranataPids, granate);
                kill_coccodrilli(n_coccodrilli, pidCoccodrilli);
                spawn_coccodrilli(coccodrilli, n_coccodrilli, pidCoccodrilli);
            }

            //vittoria se tutte le tane sono chiuse
            if (tutte_tane_chiuse()) {
                vittoria = true;
            }

            //collisioni granate vs proiettili
            collisioni_proiettili(proiettili, granate, n_coccodrilli);

            //timer
            gettimeofday(&tempo_attuale, NULL);
            long secondi_passati = tempo_attuale.tv_sec - tempo_iniziale.tv_sec;
            if (secondi_passati > ultimo_secondo) {
                ultimo_secondo = secondi_passati;
                tempo_rimasto--;
                if (tempo_rimasto < 0) {
                    //tempo scaduto => -1 vita => reset
                    perdivita();
                    reset_rana(&rana, spawn_colonna, spawn_riga, oldgranataPids, granate);
                    kill_coccodrilli(n_coccodrilli, pidCoccodrilli);
                    spawn_coccodrilli(coccodrilli, n_coccodrilli, pidCoccodrilli);
                }
            }
        }

        //pulisco le granate distrutte
        check_granate(granate, oldgranataPids);

        //disegno a schermo
        erase();
        disegna_scenario(); // in prato_tane.c

        for (int i = 0; i < n_coccodrilli; i++) {
            disegna_coccodrillo(coccodrilli[i]);
        }
        for (int i = 0; i < n_coccodrilli; i++) {
            disegna_proiettile(proiettili[i], n_coccodrilli);
        }
        for (int i = 0; i < MAX_GR_SCHERMO; i++) {
            if (granate[i].posizione.x >= 0) {
                disegna_granata(granate[i]);
            }
        }
        disegna_sprite(rana);

        disegna_info();
        disegna_timer();

        if (pausa) {
            attron(COLOR_PAIR(3) | A_BOLD);
            mvprintw(max_righe/2, (max_colonne - 9)/2, "[ PAUSA ]");
            attroff(COLOR_PAIR(3) | A_BOLD);
        }

        doupdate();

        //se vite=0 => game over
        if (vite == 0) {
            fine_gioco = true;
        }

        usleep(SLEEP);
    }

    //fine del ciclo di gioc
    kill_coccodrilli(n_coccodrilli, pidCoccodrilli);
    //kill processo rana
    kill(pid_rana, SIGKILL);
    waitpid(pid_rana, NULL, 0);

    free(coccodrilli);
    free(pidCoccodrilli);
    free(granate);
    free(oldgranataPids);
    free(proiettili);

    //schermata finale
    if (vite == 0) {
        //GAME OVER
        game_over_screen();
    } else if (vittoria) {
        //VITTORIA
        victory_screen();
    }
    return;
}


//invio SIGSTOP o SIGCONT a tutti i processi
//sigstop interrompe il processo in maniera forzata senza possibilità che il processo lo gestisca o lo ignori
//sigcont riattiva il processo, permettendogli di continuare l'esecuzione da dove era stato interrotto
void toggle_pause_processes(bool pause_on,
                            pid_t pid_rana,
                            pid_t *coccodrilliPids, int n_coccodrilli,
                            struct personaggio *proiettile, int n_proiettili,
                            struct personaggio *granate)
{
    int sig = pause_on ? SIGSTOP : SIGCONT;

    //RANA
    if (pid_rana > 0) {
        kill(pid_rana, sig);
    }
    //COCCODRILLI
    for (int i = 0; i < n_coccodrilli; i++) {
        if (coccodrilliPids[i] != 0) {
            kill(coccodrilliPids[i], sig);
        }
    }
    //PROIETTILI
    for (int i = 0; i < n_proiettili; i++) {
        if (proiettile[i].pid_figlio != 0) {
            kill(proiettile[i].pid_figlio, sig);
        }
    }
    //GRANATE
    for (int i = 0; i < MAX_GR_SCHERMO; i++) {
        if (granate[i].pid_figlio != 0) {
            kill(granate[i].pid_figlio, sig);
        }
    }
}

//chiudo tutti i processi coccodrillo con SIGKILL
void kill_coccodrilli(int n_coccodrilli, pid_t *coccodrilliPids) {
    for (int i = 0; i < n_coccodrilli; i++) {
        if (coccodrilliPids[i] != 0) {
            kill(coccodrilliPids[i], SIGKILL);
            waitpid(coccodrilliPids[i], NULL, 0);
            coccodrilliPids[i] = 0;
        }
    }
}

//varie di disegno
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

void check_granate(struct personaggio *granate, pid_t *oldgranataPids) {
    for(int g = 0; g < MAX_GR_SCHERMO; g++){
        for(int i = 0; i < MAX_GR_SCHERMO; i++){
            if(oldgranataPids[i] == granate[g].pid_figlio){
                granate[g].posizione.x = INIZIOPRIOIETTILI;
                granate[g].pid_figlio  = 0;
            }
        }
    }
}

void game_over_screen() {
    nodelay(stdscr, false);
    clear();

    attron(COLOR_PAIR(3) | A_BOLD); // Rosso
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

    const char *msgPressSpace = "(premi la barra spaziatrice per tornare al menu)";
    int xPressSpace = (max_colonne - (int)strlen(msgPressSpace)) / 2;
    mvprintw(yGameOver + 3, xPressSpace, "%s", msgPressSpace);

    refresh();
    int ch;
    do {
        ch = getch();
    } while(ch != ' ');
    flushinp();
    clear();
    refresh();
    nodelay(stdscr, true);
}

void victory_screen() {
    nodelay(stdscr, false);
    clear();

    attron(COLOR_PAIR(10) | A_BOLD); // Verde
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

    const char *msgPressSpace = "(premi la barra spaziatrice per tornare al menu)";
    int xPressSpace = (max_colonne - (int)strlen(msgPressSpace)) / 2;
    mvprintw(yVictory + 3, xPressSpace, "%s", msgPressSpace);

    refresh();
    int ch;
    do {
        ch = getch();
    } while(ch != ' ');
    flushinp();
    clear();
    refresh();
    nodelay(stdscr, true);
}
