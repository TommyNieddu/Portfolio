//
// Created by Davide Balestrino on 06/01/25.
//

#include "npc_rana.h"
#include <sys/wait.h>
#include <signal.h>


// pipe tra padre e figlio
int canale_a_figlio[2];
int canale_a_padre[2];

// lo sprite della rana è 2x3 in ascii
int rana_altezza = 2; //altezza rana
const char *prima_linea_sprite = "(0 0)";
const char *seconda_linea_sprite = "/_ _\\";

const char *prima_linea_sprite2 = "(0 0)";
const char *seconda_linea_sprite2 = "/bvd\\";

// reset proiettile, invia al padre posizione -1 per evitare che rimanga la stampa lingering
void reset_proiettile(struct personaggio p) {
    p.posizione.x = INIZIOPRIOIETTILI;
    p.posizione.y = INIZIOPRIOIETTILI;
    write(canale_a_padre[1], &p, sizeof(struct personaggio));
}

void processo_rana(int spawn_riga, int spawn_colonna) {
    struct personaggio rana;
    rana.posizione.x = spawn_colonna;
    rana.posizione.y = spawn_riga;
    rana.tipo = RANA;
    rana.lunghezza = LUNGHEZZARANA;
    rana.animation = false;

    // Array per i pid delle granate
    pid_t pid_granate[10] = {};
    int granata_counter = 0;
    int granata_minus = 0;
    int id = 0;

    // Setto la pipe in non-blocking
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    curs_set(0);

    int flags = fcntl(canale_a_figlio[0], F_GETFL, 0);
    fcntl(canale_a_figlio[0], F_SETFL, flags | O_NONBLOCK);

    while (1) {
        // Controllo se qualche granata è terminata
        for (int i = 0; i < granata_counter; i++) {
            if (pid_granate[i] != 0) {
                int status;
                pid_t result = waitpid(pid_granate[i], &status, WNOHANG);
                if (result == 0) {
                    // La granata è morta
                    pid_granate[i] = 0;
                    granata_minus++;
                }
            }
        }
        granata_counter -= granata_minus;
        granata_minus = 0;

        // Se il padre aggiorna la posizione della rana la rileggo
        struct personaggio updated;
        ssize_t n = read(canale_a_figlio[0], &updated, sizeof(struct personaggio));
        if (n == sizeof(struct personaggio)) {
            rana = updated;
        }

        // Leggo i tasti
        int tasto = getch();
        if (tasto != ERR) {
            switch (tasto) {
                case KEY_UP:
                    if (rana.posizione.y > 0)
                        rana.posizione.y -= 2;
                    break;
                case KEY_DOWN:
                    rana.posizione.y += 2;
                    break;
                case KEY_LEFT:
                    rana.posizione.x -= 2;
                    break;
                case KEY_RIGHT:
                    rana.posizione.x += 2;
                    break;

                case ' ':
                    // Spara solo se NON ci sono granate attive
                    if (granata_counter < MAX_GR_SCHERMO) {  //controllo se c'è spazio per altre granate
                        punteggio -= 5;
                        struct personaggio granata;
                        granata.tipo = GRANATA;
                        granata.lunghezza = 1;
                        granata.posizione.y = rana.posizione.y;
                        granata.posizione.x = rana.posizione.x + (LUNGHEZZARANA / 2);

                        //granata sinistra
                        granata.id = id;
                        id++;
                        granata_counter++;

                        pid_t pid_gs = fork();
                        if (pid_gs == -1) {
                            perror("fork granata sinistra");
                        } else if (pid_gs == 0) {
                            //gestisco la granata sinistra
                            processo_granata(granata, -1);
                        } else {
                            //padre (processo_rana)
                            for(int i = 0; i < granata_counter; i++){
                                if(pid_granate[i] == 0){
                                    pid_granate[i] = pid_gs;
                                }
                            }
                        }

                        //granata destra
                        granata.id = id;
                        id++;
                        granata_counter++;

                        pid_t pid_gd = fork();
                        if (pid_gd == -1) {
                            perror("fork granata destra");
                        } else if (pid_gd == 0) {
                            //gestisce la granata destra
                            processo_granata(granata, +1);
                        } else {
                            //Padre (processo_rana)
                            for(int i = 0; i < granata_counter; i++){
                                if(pid_granate[i] == 0){
                                    pid_granate[i] = pid_gd;
                                }
                            }
                        }
                    }
                    break;
            }

            //invio la nuova posizione della rana al padre
            write(canale_a_padre[1], &rana, sizeof(struct personaggio));
            rana.animation = !rana.animation;
        }

        usleep(SLEEP);
    }
}



//processo granata si muove verso sinistra o destra e poi termina
void processo_granata(struct personaggio granata, int direction) {
    // comunico al padre il PID
    granata.pid_figlio = getpid();
    write(canale_a_padre[1], &granata, sizeof(struct personaggio));

    while (1) {
        granata.posizione.x += direction;

        // Se esce dallo schermo => reset_granata + exit
        if (granata.posizione.x < gioco_sinistra || granata.posizione.x > gioco_destra){
            reset_proiettile(granata);
            _exit(0);
        }

        // mando l'aggiornamento al padre
        write(canale_a_padre[1], &granata, sizeof(struct personaggio));

        if (getppid() == 1){ //se la funzione rana è terminata
            reset_proiettile(granata);
            exit(0); //termino anche la funzione granata
        }
        usleep(SLEEPGR);
    }
}

void processo_proiettile(struct personaggio coccodrillo){
    struct personaggio proiettile;
    proiettile.tipo = PROIETTILE;
    proiettile.lunghezza = 1;
    proiettile.posizione.y = coccodrillo.posizione.y;
    if(coccodrillo.destra) {
        proiettile.posizione.x = coccodrillo.posizione.x - 1;
    } else {
        proiettile.posizione.x = coccodrillo.posizione.x + coccodrillo.lunghezza + 1;
    }
    proiettile.id = coccodrillo.id;
    proiettile.animation = false;

    //Assegno il PID al campo della struct
    proiettile.pid_figlio = getpid();

    // Invio la struct al padre
    write(canale_a_padre[1], &proiettile, sizeof(struct personaggio));

    while (1) {

        if (coccodrillo.destra) {
            proiettile.posizione.x -= 1;
            if (proiettile.posizione.x < gioco_sinistra) {
                reset_proiettile(proiettile);
                _exit(0);
            }
        } else {
            proiettile.posizione.x += 1;
            if (proiettile.posizione.x > gioco_destra) {
                reset_proiettile(proiettile);
                _exit(0);
            }
        }

        if (getppid() == 1){
            proiettile.posizione.x = INIZIOPRIOIETTILI;
            write(canale_a_padre[1], &proiettile, sizeof(struct personaggio));
            exit(0);
        }
        // Invio la posizione aggiornata al padre
        write(canale_a_padre[1], &proiettile, sizeof(struct personaggio));
        proiettile.animation = !proiettile.animation;
        usleep(SLEEPROJ);
    }
}


void processo_coccodrilli(struct personaggio coccodrillo) {
    struct timeval current_time;
    srand(coccodrillo.id); // seme per random basato su id

    // pid del processo proiettile attualmente in volo (max uno alla volta)
    pid_t pid_proiettile = 0;

    while (1) {
        gettimeofday(&current_time, NULL);
        //sposto il coccodrillo orizzontalmente
        if (coccodrillo.destra) {
            // si muove a sinistra
            coccodrillo.posizione.x -= 1;
            if (coccodrillo.posizione.x +  coccodrillo.lunghezza == gioco_sinistra) coccodrillo.posizione.x = gioco_destra + MAXLUNGHEZZA;
        } else {
            // si muove a destra
            coccodrillo.posizione.x += 1;
            if (coccodrillo.posizione.x == gioco_destra) coccodrillo.posizione.x = gioco_sinistra - MAXLUNGHEZZA;
        }

        //se c'era un proiettile lanciato, controllo se è terminato
        if (pid_proiettile != 0) {
            int status;
            pid_t result = waitpid(pid_proiettile, &status, WNOHANG);
            if (result == pid_proiettile) {
                pid_proiettile = 0;
            }
        }

        //genero un proiettile ogni tot, con una certa probabilità se non ne è già attivo un altro
        if (pid_proiettile == 0 && (current_time.tv_sec % 2 == 0) && (rand() % 100 < 10) && coccodrillo.id > 50) {
            pid_t pid = fork();
            if (pid == -1) {
                perror("fork proiettile");
            } else if (pid == 0) { //processo prioettile
                processo_proiettile(coccodrillo);
            }
            else {
                //memorizzo il PID del proiettile
                pid_proiettile = pid;
            }
        }

        //aggiorno la posizione del coccodrillo al padre
        write(canale_a_padre[1], &coccodrillo, sizeof(struct personaggio));
        coccodrillo.animation = !coccodrillo.animation;
        if(coccodrillo.posizione.y % 3 == 0){ //sleep differenti a seconda della fila, determinano la velocità
            usleep(SLEEPCROC3);
        }else if(coccodrillo.posizione.y % 2 == 0){
            usleep(SLEEPCROC2);
        }else{
            usleep(SLEEPCROC1);
        }
    }
}

// disegna sprite rana
void disegna_sprite(struct personaggio p) {
    for (int i = 0; i < p.lunghezza; i++){
        if (p.animation == false) {
            mvaddch(p.posizione.y,     p.posizione.x + i, prima_linea_sprite[i]);
            mvaddch(p.posizione.y + 1, p.posizione.x + i, seconda_linea_sprite[i]);
        }else{
            mvaddch(p.posizione.y,     p.posizione.x + i, prima_linea_sprite2[i]);
            mvaddch(p.posizione.y + 1, p.posizione.x + i, seconda_linea_sprite2[i]);
        }
    }
}

void disegna_coccodrillo(struct personaggio p) {
    for (int i = 0; i < p.lunghezza; i++) {
        char sopra = '^', sotto = '_'; //corpo
        if (p.posizione.x + i > gioco_sinistra && p.posizione.x + i < gioco_destra) {
            if (p.destra) { //controllo direzione
                if (i == 0) {
                    sopra = '\\'; sotto = '/'; //faccia
                } else if (i == 1) {
                    sopra = '0'; sotto = '0'; //occhi
                } else if (i == p.lunghezza - 1) { //coda
                    sopra = p.animation ? '\\' : '/';
                    sotto = p.animation ? '/' : '\\';
                }
            } else { // p.destra == false
                if (i == p.lunghezza - 1) {
                    sopra = '/'; sotto = '\\';
                } else if (i == p.lunghezza - 2) {
                    sopra = '0'; sotto = '0';
                } else if (i == 0) {
                    sopra = p.animation ? '/' : '\\';
                    sotto = p.animation ? '\\' : '/';
                }
            }
            mvaddch(p.posizione.y, p.posizione.x + i, sopra);
            mvaddch(p.posizione.y + 1, p.posizione.x + i, sotto);
        }
    }
}

// disegna un proiettile
void disegna_proiettile(struct personaggio p, int n_coccodrilli) {
    if(p.id >= 0 && p.id < n_coccodrilli && p.posizione.x >= gioco_sinistra && p.posizione.x <= gioco_destra){
        if(p.animation){
            mvaddch(p.posizione.y, p.posizione.x, 'L');
            mvaddch(p.posizione.y + 1, p.posizione.x, 'T');
        }else{
            mvaddch(p.posizione.y, p.posizione.x, '|');
            mvaddch(p.posizione.y + 1, p.posizione.x, '|');
        }
    }
}

// disegna una granata
void disegna_granata(struct personaggio p) {
    if (p.posizione.x >= gioco_sinistra && p.posizione.x <= gioco_destra) {
        mvaddch(p.posizione.y, p.posizione.x, 'G');
        mvaddch(p.posizione.y + 1, p.posizione.x, 'G');
    }
}