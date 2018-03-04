/**
   \file gestore_segnali.c
   \author Simone Paioletti
   Si dichiara che il contenuto di questo file e' in ogni sua parte opera
   originale dell' autore.
*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include "wator.h"
#include "funzioni.h"

#define MENOUNO(x) if (x==-1) {perror("Errore nel main"); exit(EXIT_FAILURE);}
#define DIFFZERO(x) if (x) {perror("Errore nel main"); exit(EXIT_FAILURE);}
#define EQNULL(x) if (x==NULL) {perror("Errore main"); exit(EXIT_FAILURE);}

/* Questa funzione gestisce i segnali intercettati dal processo; se
   intercetta SIGINT o SIGTERM viene settata ad 1 la variabile
   "segnale_stop_ricevuto".
*/
void gestione_segnali(int numero_segnale) {
    if (numero_segnale==SIGINT || numero_segnale==SIGTERM) {
        segnale_stop_ricevuto=1;
    }
}

/* Si mette in attesa del segnale SIGUSR1; appena lo riceve stampa
   lo stato del pianeta nel file wator.check e si rimette in attesa.
   Se riceve il segnale SIGINT invece il thread termina.
*/
void attesa_segnale_stampa() {
    int segnale_ricevuto;
    FILE *watorcheck=NULL;
    MENOUNO(sigaddset(&maschera_segnali, SIGINT));
    while (1) {
        DIFFZERO(sigwait(&maschera_segnali, &segnale_ricevuto));
        if (segnale_ricevuto==SIGUSR1) {
            EQNULL((watorcheck=fopen(WATORCHECK, "w+")));
            MENOUNO(print_planet(watorcheck, pianeta->plan));
            DIFFZERO(fclose(watorcheck));
        } else {
            pthread_exit(0);
        }
    }
}

/* Questa funzione libera tutta la memoria allocata dal processo
   wator.
*/
void libera_memoria() {
    free(file);
    free(dumpfile);
    free(lista_worker);
    free(mutex_righe);
    free_wator(pianeta);
    memoria_liberata=1;
}

/* Questa funzione serve ad assicurare che, in caso di terminazione
   imprevista del processo wator, venga killato anche il processo
   visualizer.
*/
void gestione_uscita() {
    MENOUNO(kill(pid, SIGKILL));
    MENOUNO(wait(NULL));
    if (!memoria_liberata)
        libera_memoria();
}
