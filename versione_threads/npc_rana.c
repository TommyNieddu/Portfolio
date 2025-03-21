// created by Thomas Nieddu on 21/02/25.
#include "npc_rana.h"
#include "regole_gioco.h"
#include <pthread.h>
#include <signal.h>

// variabili esterne
extern bool pausa;
extern buffer_circolare buf_principale;
int punteggio;

// costanti e sprite della rana
int rana_altezza = 2;

const char *prima_linea_sprite      = "(0 0)";
const char *seconda_linea_sprite   = "/_ _\\";
const char *prima_linea_sprite2    = "(0 0)";
const char *seconda_linea_sprite2  = "/bvd\\";

// uso questa quando voglio riportare proiettile/granata a x = -1
void reset_proiettile(struct personaggio p) {
    p.posizione.x = INIZIOPRIOIETTILI;
    p.posizione.y = INIZIOPRIOIETTILI;
    produce_messaggio(&buf_principale, p);
}

void* processo_granata(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    GranataArgs *gargs = (GranataArgs*) arg;
    struct personaggio granata = gargs->granata;
    int direction = gargs->direction;
    free(gargs);

    granata.thread_id = pthread_self();
    granata.animation = false;

    produce_messaggio(&buf_principale, granata);

    while(true) {
        while(pausa) {
            usleep(SLEEP);
        }
        granata.posizione.x += direction;

        if(granata.posizione.x < gioco_sinistra || granata.posizione.x > gioco_destra) {
            reset_proiettile(granata);
            break;
        }
        produce_messaggio(&buf_principale, granata);
        usleep(SLEEPGRANATA);
    }
    return NULL;
}

void* processo_proiettile(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    CoccodrilloProjArgs* pargs = (CoccodrilloProjArgs*) arg;
    struct personaggio bullet = pargs->coccodrillo;
    int direction = pargs->direction;
    free(pargs);

    // se non usi direction successivamente:
    (void) direction; // evita warning "unused variable"

    bullet.thread_id = pthread_self();
    bullet.tipo      = PROIETTILE;
    bullet.lunghezza = 1;
    bullet.animation = false;

    produce_messaggio(&buf_principale, bullet);

    bool versoDestra = false;
    if (bullet.destra) {
        versoDestra = false;
    } else {
        versoDestra = true;
    }

    while(true) {
        while(pausa) {
            usleep(SLEEP);
        }

        if (versoDestra) {
            bullet.posizione.x += 1;
            if (bullet.posizione.x > gioco_destra) {
                bullet.posizione.x = gioco_destra;
                versoDestra = false;
            }
        } else {
            bullet.posizione.x -= 1;
            if (bullet.posizione.x < gioco_sinistra) {
                bullet.posizione.x = gioco_sinistra;
                versoDestra = true;
            }
        }

        produce_messaggio(&buf_principale, bullet);
        usleep(SLEEPPROJ);
    }
    return NULL;
}




void* processo_coccodrilli(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    CoccodrilloArgs* cargs = (CoccodrilloArgs*) arg;
    struct personaggio coccodrillo = cargs->coccodrillo;
    free(cargs);

    struct timeval last_update, current_time;
    gettimeofday(&last_update, NULL);

    int riga = (coccodrillo.posizione.y - tana_fine_riga) / 2;
    const int RITARDO_COCCO[5]  = {300000, 350000, 400000, 450000, 500000};
    srand(coccodrillo.id);

    int move_delay = RITARDO_COCCO[riga % 5];
    pthread_t tid_proiettile = 0;

    coccodrillo.thread_id = pthread_self();

    while(true) {
        while(pausa) {
            usleep(SLEEP);
        }
        gettimeofday(&current_time, NULL);
        long elapsed = (current_time.tv_sec  - last_update.tv_sec) * 1000000L
                       + (current_time.tv_usec - last_update.tv_usec);

        if(elapsed >= move_delay) {
            if(tid_proiettile != 0) {
                if(pthread_kill(tid_proiettile, 0) != 0) {
                    tid_proiettile = 0;
                }
            }
            if(tid_proiettile == 0 && (rand() % 100 < 10) && coccodrillo.id > 50) {
                CoccodrilloProjArgs* pargs = malloc(sizeof(CoccodrilloProjArgs));
                if(!pargs) {
                    fprintf(stderr, "Errore: allocazione di memoria fallita per CoccodrilloProjArgs.\n");
                    exit(8);
                }
                pargs->coccodrillo = coccodrillo;
                pargs->direction   = +1;

                pthread_t tid;
                if(pthread_create(&tid, NULL, processo_proiettile, pargs) == 0) {
                    tid_proiettile = tid;
                }
            }

            // sposto coccodrillo
            if(coccodrillo.destra) {
                coccodrillo.posizione.x -= 1;
                if(coccodrillo.posizione.x + coccodrillo.lunghezza < gioco_sinistra) {
                    coccodrillo.posizione.x = gioco_destra + LUNGHEZZA_MASSIMA;
                }
            } else {
                coccodrillo.posizione.x += 1;
                if(coccodrillo.posizione.x > gioco_destra) {
                    coccodrillo.posizione.x = gioco_sinistra - LUNGHEZZA_MASSIMA;
                }
            }

            coccodrillo.animation = !coccodrillo.animation;
            produce_messaggio(&buf_principale, coccodrillo);

            last_update = current_time;
        }
        usleep(SLEEP);
    }
    return NULL;
}

void* processo_rana(int spawn_riga, int spawn_colonna) {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    struct personaggio rana;
    rana.posizione.x = spawn_colonna;
    rana.posizione.y = spawn_riga;
    rana.tipo        = RANA;
    rana.lunghezza   = LUNGHEZZA_RANA;
    rana.animation   = false;
    rana.thread_id   = pthread_self();

    produce_messaggio(&buf_principale, rana);

    pthread_t granateThreads[MAX_GRANATE_ATTIVE];
    for(int i=0; i<MAX_GRANATE_ATTIVE; i++){
        granateThreads[i] = 0;
    }
    int granate_count = 0;
    int granata_id_base = 0;

    while(true) {
        while(pausa) {
            usleep(SLEEP);
        }

        for(int i=0; i<MAX_GRANATE_ATTIVE; i++){
            if(granateThreads[i] != 0) {
                if(pthread_kill(granateThreads[i], 0) != 0) {
                    granateThreads[i] = 0;
                    if(granate_count > 0) granate_count--;
                }
            }
        }

        struct personaggio cmd;
        if(consume_messaggio_non_bloccante(&buf_principale, &cmd)) {
            if(cmd.tipo == RANA) {
                if(cmd.posizione.x == -1 && cmd.posizione.y == -1) {
                    int tasto = cmd.id;
                    switch(tasto) {
                        case KEY_UP:
                            rana.posizione.y -= 2;
                            rana.animation = !rana.animation;
                            break;
                        case KEY_DOWN:
                            rana.posizione.y += 2;
                            rana.animation = !rana.animation;
                            break;
                        case KEY_LEFT:
                            rana.posizione.x -= 2;
                            rana.animation = !rana.animation;
                            movimentoManuale = true;
                            break;
                        case KEY_RIGHT:
                            rana.posizione.x += 2;
                            rana.animation = !rana.animation;
                            movimentoManuale = true;
                            break;
                        case ' ':
                            // spara 2 granate (sx,dx)
                            if(granate_count <= (MAX_GRANATE_ATTIVE - 2)) {
                                punteggio -= 5;
                                struct personaggio g;
                                g.tipo       = GRANATA;
                                g.lunghezza  = 1;
                                g.posizione.y= rana.posizione.y;
                                g.posizione.x= rana.posizione.x + (rana.lunghezza / 2);
                                g.animation  = false;

                                // sinistra
                                g.id = granata_id_base++;
                                GranataArgs *gs_args = malloc(sizeof(GranataArgs));
                                gs_args->granata   = g;
                                gs_args->direction = -1;
                                pthread_t gsin;
                                if(pthread_create(&gsin, NULL, processo_granata, gs_args) == 0) {
                                    for(int k=0; k<MAX_GRANATE_ATTIVE; k++){
                                        if(granateThreads[k] == 0) {
                                            granateThreads[k] = gsin;
                                            granate_count++;
                                            break;
                                        }
                                    }
                                }
                                // destra
                                g.id = granata_id_base++;
                                GranataArgs *gd_args = malloc(sizeof(GranataArgs));
                                gd_args->granata   = g;
                                gd_args->direction = +1;
                                pthread_t gdes;
                                if(pthread_create(&gdes, NULL, processo_granata, gd_args) == 0) {
                                    for(int k=0; k<MAX_GRANATE_ATTIVE; k++){
                                        if(granateThreads[k] == 0) {
                                            granateThreads[k] = gdes;
                                            granate_count++;
                                            break;
                                        }
                                    }
                                }
                            }
                            break;
                    }
                } else {
                    // altrimenti è una posizione forzata dal padre
                    rana.posizione.x = cmd.posizione.x;
                    rana.posizione.y = cmd.posizione.y;
                    rana.animation   = cmd.animation;
                }
            }
        }

        produce_messaggio(&buf_principale, rana);
        usleep(SLEEP);
    }
    return NULL;
}

// si occupa di disegnare la rana su due righe
void disegna_sprite(struct personaggio p) {
    for(int i = 0; i < p.lunghezza; i++) {
        if(p.posizione.x + i < gioco_sinistra || p.posizione.x + i > gioco_destra)
            continue;

        if(!p.animation) {
            mvaddch(p.posizione.y, p.posizione.x + i, prima_linea_sprite[i]);
            mvaddch(p.posizione.y + 1, p.posizione.x + i, seconda_linea_sprite[i]);
        } else {
            mvaddch(p.posizione.y,     p.posizione.x + i, prima_linea_sprite2[i]);
            mvaddch(p.posizione.y + 1, p.posizione.x + i, seconda_linea_sprite2[i]);
        }
    }
}



// sprite personalizzato
void disegna_coccodrillo(struct personaggio p) {
    for(int i = 0; i < p.lunghezza; i++) {
        char sopra='^', sotto='_';
        if(p.destra) {
            if(i == 0) {
                sopra='\\'; sotto='/';
            } else if(i == 1) {
                sopra='0';  sotto='0';
            } else if(i == p.lunghezza -1) {
                if(p.animation) {
                    sopra='\\'; sotto='/';
                } else {
                    sopra='/';  sotto='\\';
                }
            }
        } else {
            if(i == p.lunghezza -1) {
                sopra='/'; sotto='\\';
            } else if(i == p.lunghezza -2) {
                sopra='0'; sotto='0';
            } else if(i == 0) {
                if(p.animation) {
                    sopra='/'; sotto='\\';
                } else {
                    sopra='\\'; sotto='/';
                }
            }
        }
        if(p.posizione.x + i >= gioco_sinistra && p.posizione.x + i <= gioco_destra) {
            mvaddch(p.posizione.y,     p.posizione.x + i, sopra);
            mvaddch(p.posizione.y + 1, p.posizione.x + i, sotto);
        }
    }
}

void* processo_proiettile_tana(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    TanaProjArgs *pargs = (TanaProjArgs*) arg;
    struct personaggio bullet = pargs->proiettile;
    free(pargs);

    bullet.thread_id = pthread_self();
    bullet.tipo = PROIETTILE_TANA;
    bullet.lunghezza = 1;
    bullet.animation = false;

    produce_messaggio(&buf_principale, bullet);

    while (true) {
        while (pausa) {
            usleep(SLEEP);
        }

        bullet.posizione.y += 1;

        if (bullet.posizione.y >= riga_inizio_prato) {
            reset_proiettile(bullet);
            break;
        }

        produce_messaggio(&buf_principale, bullet);
        usleep(SLEEPGRANATA);
    }
    return NULL;
}





void disegna_proiettile_tana(struct personaggio p, int max_proiettili_tana)
{

    if (p.id >= 0 && p.id < max_proiettili_tana
        && p.posizione.x >= gioco_sinistra
        && p.posizione.x <= gioco_destra
        && p.posizione.y >= 0
        && p.posizione.y < max_righe - 1)
    {

        mvaddch(p.posizione.y,     p.posizione.x, 'v');
        mvaddch(p.posizione.y + 1, p.posizione.x, 'v');
    }
}

// sprite verticale
void disegna_proiettile(struct personaggio p, int n_coccodrilli) {
    if(p.id >= 0 && p.id < n_coccodrilli
       && p.posizione.x >= gioco_sinistra
       && p.posizione.x <= gioco_destra)
    {
        mvaddch(p.posizione.y,     p.posizione.x, '|');
        mvaddch(p.posizione.y + 1, p.posizione.x, '|');
    }
}

// due 'G' in verticale
void disegna_granata(struct personaggio p) {
    if(p.posizione.x >= gioco_sinistra && p.posizione.x <= gioco_destra) {
        mvaddch(p.posizione.y,     p.posizione.x, 'G');
        mvaddch(p.posizione.y + 1, p.posizione.x, 'G');
    }
}
