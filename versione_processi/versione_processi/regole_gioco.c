//
// Created by Davide Balestrino on 06/01/25.
//

#include "regole_gioco.h"
#include <sys/wait.h>
#include <signal.h>

//variabili di gioco
int vite;  //numero di vite del giocatore
int tempo_rimasto;  //quanto tempo mi rimane

void reset_rana(struct personaggio *rana, int x, int y, pid_t *old, struct personaggio *granate) { //riporto la rana alla posizione iniziale
    rana->posizione.x = x;
    rana->posizione.y = y;
    write(canale_a_figlio[1], rana, sizeof(struct personaggio)); //aggiorno la posizione del figlio
    for(int g = 0; g < MAX_GR_SCHERMO; g++){
        old[g] = granate[g].pid_figlio;
        if(granate[g].pid_figlio != 0){
            kill(granate[g].pid_figlio, SIGKILL);
            waitpid(granate[g].pid_figlio, NULL, 0);
            granate[g].posizione.x = INIZIOPRIOIETTILI;
        }
    }
}

//se tutte le tane sono chiuse => vittoria
bool tutte_tane_chiuse() {
    for (int i = 0; i < num_tane; i++) {
        if (tane_aperte[i]) return false;
    }
    return true;
}

//se la rana entra nell'area della tana e questa è aperta, +vita, se chiusa o colonna gialla, -vita
bool check_tane(struct personaggio rana) {
    int riga_inizio_buco = tana_inizio_riga + 1; //il buco comincia uno sotto
    int riga_fine_buco = tana_inizio_riga + 4;//finisce un po' prima della fine gialla

    // se la rana non tocca neanche la fascia verticale della tana esco
    if (rana.posizione.x + rana_altezza - 1 < tana_inizio_riga || rana.posizione.y > tana_fine_riga) {
        return false;
    }

    for (int i = 0; i < num_tane; i++) {
        //calcolo l'inizio x della tana i
        int inizio_tana_x = gioco_sinistra + offset_tane + i * (larghezza_tana + spazio);
        int fine_tana_x = inizio_tana_x + larghezza_tana - 1;
        int inizio_buco_x = inizio_tana_x + 1;
        int larghezza_buco = larghezza_tana - 1;
        int fine_buco_x = inizio_buco_x + larghezza_buco - 1;

        //controllo orizzontalmente
        bool sovrapposizione_tana = (rana.posizione.x + rana.lunghezza - 1 >= inizio_tana_x) && (rana.posizione.x <= fine_tana_x);

        //controllo verticalmente
        bool sovrapposizione_vert_tana = (rana.posizione.y + rana_altezza - 1 >= tana_inizio_riga) && (rana.posizione.y <= tana_fine_riga);

        if (sovrapposizione_tana && sovrapposizione_vert_tana) {
            //controllo se stiamo nel buco  oppure sui pilastri gialli
            bool sovrapposizione_buco = (rana.posizione.x + rana.lunghezza - 1 >= inizio_buco_x) && (rana.posizione.x <= fine_buco_x) && (rana.posizione.y + rana_altezza - 1 >= riga_inizio_buco) && (rana.posizione.y <= riga_fine_buco);

            if (sovrapposizione_buco) {
                //tana aperta => +1 vita, chiusa => -1 vita
                if (tane_aperte[i]) {
                    if (vite < 8) vite++;
                    tempo_rimasto = TEMPO_MASSIMO;
                    tane_aperte[i] = false;
                    punteggio += 50;
                } else {
                    perdivita();
                }
            } else {
                //se tocc i pilastri gialli => -1 vita
                perdivita();
            }

            return true;
        }
    }
    return false;
}

int conta_coccodrilli() { //funzione di conta, sono 5 coccodrilli per riga
    int count = 0, y = prato2_fine_riga;
    while (y < riga_inizio_prato) {
        count += COCCODRILLIXRIGA;
        y += 2;
    }
    return count;
}

void spawn_coccodrilli(struct personaggio *coccodrilli, int n_coccodrilli, pid_t *coccodrilliPids) {
    int y_iniziale = riga_inizio_prato - 2, x_iniziale = gioco_sinistra - MAXLUNGHEZZA;
    bool destra;

    srand(time(NULL));

    //direzione della riga
    destra = (rand() % 2 == 0);

    for (int i = 0; i < n_coccodrilli; i++) {
        coccodrilli[i].id = i;
        coccodrilli[i].tipo = COCCODRILLO;
        coccodrilli[i].lunghezza = (rand() % (MAXLUNGHEZZA - MINLUNGHEZZA + 1)) + MINLUNGHEZZA; //lunghezza casuale
        if(rand() % 2 == 0){
            coccodrilli[i].animation = true;
        }else{
            coccodrilli[i].animation = false;
        }

        if (i % COCCODRILLIXRIGA == 0 && i != 0) { //se ci sono 5 coccodrilli nella riga
            y_iniziale -= 2; //cambio riga
            destra = !destra; //cambio verso
            x_iniziale = gioco_sinistra - MAXLUNGHEZZA; //riporto i coccodrilli nella posizione iniziale
        } else if(i != 0){ //altrimenti
            x_iniziale = coccodrilli[i - 1].posizione.x + coccodrilli[i - 1].lunghezza + (rand() % (MAXDISTANZA - MINDISTANZA + 1)) + MINDISTANZA; //calcolo casualmente la distanza tra un coccodrillo e l'altro
        }

        coccodrilli[i].destra = destra; //salvo le coordinate e la direzione
        coccodrilli[i].posizione.x = x_iniziale;
        coccodrilli[i].posizione.y = y_iniziale;

        pid_t pid_c = fork();
        if (pid_c == -1) {
            endwin();
            perror("fork coccodrillo");
            exit(1);
        } else if (pid_c == 0) {
            //coccodrillo avvio funzione
            close(canale_a_figlio[1]);
            close(canale_a_padre[0]);
            processo_coccodrilli(coccodrilli[i]);
        } else {
            //salvo il pid del figlio
            coccodrilliPids[i] = pid_c;
        }
    }
}

void controllo_coccodrilli(struct personaggio *rana, struct personaggio coccodrilli[], int n_coccodrilli) {
    static int relativeOffset = 0;       // offset dalla sinistra del coccodrillo alla rana

    //calcoloil centro orizzontale della rana e il bordo inferiore
    int frogCenter = rana->posizione.x + rana->lunghezza / 2;

    for (int i = 0; i < n_coccodrilli; i++) {
        //la base della rana frogBottomY deve essere quasi uguale alla riga superiore del coccodrillo,
        //cioè la differenza assoluta deve essere al massimo 1.
        //il centro della rana frogCenter deve essere compreso nell'intervallo orizzontale
        //del coccodrillo.
        if (rana->posizione.y == coccodrilli[i].posizione.y && frogCenter >= coccodrilli[i].posizione.x && frogCenter <= coccodrilli[i].posizione.x + coccodrilli[i].lunghezza) {
            //controllo se la posizione corrente differisce dall'attesa
            int expectedX = coccodrilli[i].posizione.x + relativeOffset;
            int diff = rana->posizione.x - expectedX;
            if (abs(diff) >= 2) {  //se la differenz è significativa voglio avre solo movim manuale
                relativeOffset = rana->posizione.x - coccodrilli[i].posizione.x;
            }
            //aggiormola posizione della rana in modo che rimanga fissata al coccodrillo
            rana->posizione.x = coccodrilli[i].posizione.x + relativeOffset;
            //inviola struttura aggiornata al processo rana tramite la pipe padre → rana
            write(canale_a_figlio[1], rana, sizeof(struct personaggio));
            break;
        }
    }
}

bool perdivita() { //funzione di reset
   vite--;
   punteggio -= 20;
   tempo_rimasto = TEMPO_MASSIMO;
    return true;
}

bool controllo_bordi(struct personaggio rana) { //se vado oltre i limiti a sinistra o destra o sotto
    if (rana.posizione.x < gioco_sinistra || (rana.posizione.x + rana.lunghezza - 1) > gioco_destra || rana.posizione.y > riga_fine_prato - 2) {
        return perdivita();
    }
    return false;
}

/*bool controllo_proiettili(struct personaggio rana, struct personaggio proiettili[], int n_coccodrilli) { //funzione per controllare la collisione tra rana e proiettile
    for (int i = 0; i < n_coccodrilli; i++) { //ciclo tra i proiettili
        if (proiettili[i].posizione.x >= gioco_sinistra && proiettili[i].posizione.x <= gioco_destra) { //se il proiettile è stato generato
            if (rana.posizione.x < proiettili[i].posizione.x + proiettili[i].lunghezza && rana.posizione.x + rana.lunghezza > proiettili[i].posizione.x && rana.posizione.y == proiettili[i].posizione.y) { //controllo la collisione
                return perdivita();
            }
        }
    }
    return false;
}*/

/*bool controllo_fiume(struct personaggio rana, struct personaggio coccodrilli[], int n_coccodrilli) { //funzione per controllare se la rana si trova sul fiume senza un coccodrillo sopra
    if (rana.posizione.y < riga_inizio_prato && rana.posizione.y > prato2_fine_riga) { //se la rana è sul fiume
        for (int i = 0; i < n_coccodrilli; i++) { //ciclo tra i coccodrilli
            if (coccodrilli[i].posizione.y == rana.posizione.y) { //se la rana è su un coccodrillo non faccio nulla
                if (rana.posizione.x + rana.lunghezza - 1 >= coccodrilli[i].posizione.x && rana.posizione.x <= coccodrilli[i].posizione.x + coccodrilli[i].lunghezza - 1) {
                    return false;
                }
            }
        }
        return perdivita(); //altrimenti se la rana è sul fiume e non su un coccodrillo resetto
    }
    return false; //se la rana non è sul fiume
}*/

/*void collisioni_proiettili(struct personaggio *proiettili, struct personaggio *granate, int n_proiettili){
    for (int g = 0; g < MAX_GR_SCHERMO; g++) {
        if (granate[g].posizione.x > 0){ //se la granata si trova nel campo di gioco
            for (int p = 0; p < n_proiettili; p++) {
                if (proiettili[p].posizione.x > 0){ //se il proiettile si trova sul campo di gioco
                    //controllo la collisione
                    bool collisione = (granate[g].posizione.y == proiettili[p].posizione.y) && (abs(granate[g].posizione.x - proiettili[p].posizione.x) <= 2);
                    if (collisione) { //se vi è una collisione
                        punteggio += 10; //aumento  il punteggio
                        if (granate[g].pid_figlio != 0) { //elimino il processo granata
                            kill(granate[g].pid_figlio, SIGKILL);
                            waitpid(granate[g].pid_figlio, NULL, 0);
                            granate[g].pid_figlio = 0;
                        }
                        if (proiettili[p].pid_figlio != 0) { //elimino il processo prioettile
                            kill(proiettili[p].pid_figlio, SIGKILL);
                            proiettili[p].pid_figlio = 0;
                        }
                        granate[g].posizione.x = INIZIOPRIOIETTILI; //resetto le posizioni salvate di proiettili e granate
                        granate[g].posizione.y = INIZIOPRIOIETTILI;
                        proiettili[p].posizione.x = INIZIOPRIOIETTILI;
                        proiettili[p].posizione.y = INIZIOPRIOIETTILI;
                    }
                }
            }
        }
    }
}*/