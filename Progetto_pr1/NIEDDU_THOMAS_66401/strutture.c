#include "Strutture.h"
#include "Effetti.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
FILE* cronologia;

void Titolo(){
    printf(TESTO_MAGENTA);
    printf("██████████████████████████████████████████████████████████████\n"
           "█▌                                                          ▐█\n"
           "█▌                                                          ▐█\n"
           "█▌     ░█░█░█▀█░█▀█░█▀█░█░█░░░░                             ▐█\n"
           "█▌     ░█▀█░█▀█░█▀▀░█▀▀░░█░░░░░                             ▐█\n"
           "█▌     ░▀░▀░▀░▀░▀░░░▀░░░░▀░░░░░                             ▐█\n"
           "█▌     ░░░░░░░░░░░█░░░▀█▀░▀█▀░▀█▀░█░░░█▀▀                   ▐█\n"
           "█▌     ░░░░░░░░░░░█░░░░█░░░█░░░█░░█░░░█▀▀                   ▐█\n"
           "█▌     ░░░░░░░░░░░▀▀▀░▀▀▀░░▀░░░▀░░▀▀▀░▀▀▀                   ▐█\n"
           "█▌     ░░░░░░░░░░░░░░░░░█▀▀░▀█▀░█░█░█▀▄░█▀▀░█▀█░▀█▀░█▀▀     ▐█\n"
           "█▌     ░░░░░░░░░░░░░░░░░▀▀█░░█░░█░█░█░█░█▀▀░█░█░░█░░▀▀█     ▐█\n"
           "█▌     ░░░░░░░░░░░░░░░░░▀▀▀░░▀░░▀▀▀░▀▀░░▀▀▀░▀░▀░░▀░░▀▀▀     ▐█\n"
           "█▌                                                          ▐█\n"
           "█▌                                                          ▐█\n"
           "██████████████████████████████████████████████████████████████");
}
void CreaCronologia() {
    cronologia = fopen("cronologia.txt","a");

    if (cronologia == NULL) {
        printf(TESTO_ROSSO);
        printf("Errore nell'apertura del file!\n");
        printf(TESTO_STANDARD);
        return;
    }
}
void Trascrivi(char* evento) {
    FILE* cronologia = fopen("cronologia.txt", "a");
    if (cronologia) {
        fprintf(cronologia, "%s\n", evento);
        fclose(cronologia);
    } else {
        printf("File cronologia non aperto!\n");
    }
}
void TerminaCronologia() {
    fclose(cronologia);
}
void PuliziaTerminale(){
#ifdef _WIN32
    system("cls");
#elif __APPLE__
    system("clear");
#else
    system("clear");
#endif
}










