/**
   \file collector.c
   \author Simone Paioletti
   Si dichiara che il contenuto di questo file e' in ogni sua parte opera
   originale dell' autore.
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "wator.h"
#include "funzioni.h"

#define MENOUNO(x) if (x==-1) {perror("Errore nel thread collector"); exit(EXIT_FAILURE);}
#define DIFFZERO(x) if (x) {perror("Errore nel thread collector"); exit(EXIT_FAILURE);}
#define EQNULL(x) if (x==NULL) {perror("Errore nel thread collector"); exit(EXIT_FAILURE);}
#define THREADCONTROL(x) if ((errno=x)) {perror("Errore nel thread collector"); exit(EXIT_FAILURE);}
#define ERRCONN(x) while (x==-1) {if (errno==ENOENT||errno==ENOTCONN||errno==ECONNREFUSED) errno=0; else {perror("Errore nel thread collector"); exit(EXIT_FAILURE);}}

static char num_righe[10], num_colonne[10], risposta_visualizer, *controllo_fine="0";

/* Questa funzione gestisce la comunicazione tra il thread collector e
   il processo visualizer.
   Per prima cosa apre la connessione, invia il valore di "controllo_fine"
   e poi controlla se è necessario inviare le misure della matrice. 
   Poi invia la matrice pianeta spedendo un char per volta; negli 8 bit del
   char sono contenuti i valori di 4 celle della matrice.
   Una volta finito aspetta che il processo visualizer abbia finito la stampa
   e chiude la connessione.
*/

static void invio_matrice() {
    int i, j, contatore_celle=0;
    char cella=0;
    MENOUNO((canale=socket(AF_UNIX, SOCK_STREAM, 0)));
    ERRCONN(connect(canale, (const struct sockaddr*)&indirizzo, sizeof(indirizzo)));
    MENOUNO(write(canale, controllo_fine, sizeof(char)));
    if (misure_inviate==0) {
        MENOUNO(write(canale, num_righe, 10));
        MENOUNO(write(canale, num_colonne, 10));
        misure_inviate=1;
    }
    for (i=0; i<righe; i++) {
        for (j=0; j<colonne; j++) {
            switch (contatore_celle) {
                case 0:
                    cella=cella+((pianeta->plan->w[i][j])<<6);
                    contatore_celle++;
                    break;
                case 1:
                    cella=cella+((pianeta->plan->w[i][j])<<4);
                    contatore_celle++;
                    break;
                case 2:
                    cella=cella+((pianeta->plan->w[i][j])<<2);
                    contatore_celle++;
                    break;
                case 3:
                    cella=cella+(pianeta->plan->w[i][j]);
                    contatore_celle=0;
                    MENOUNO(write(canale, &cella, sizeof(char)));
                    cella=0;
                    break;
            }
        }
    }
    if (((righe*colonne)%4)!=0) {
        MENOUNO(write(canale, &cella, sizeof(char)));
    }
    MENOUNO(read(canale, &risposta_visualizer, sizeof(char)));
    MENOUNO(close(canale));
}

/* Questa funzione viene invocata dal thread collector quando, controllando il
   valore della variabile "segnale_stop_ricevuto", si accorge che è stato ricevuto
   un segnale di tipo SIGINT o SIGTERM e serve a impostare ad 1 la variabile
   "controllo_fine" che sarà poi inviata al processo visualizer, ad invocare 
   la funzione "invio_matrice" per inviare il pianeta per l'ultima volta e a 
   far terminare il thread collector.
*/

static void ricezione_segnale() {
    controllo_fine="1";
    invio_matrice();
    fine_collector=1;
    DIFFZERO(pthread_mutex_unlock(&mutex_globale));
    pthread_exit(0);
}

/* Il thread collector viene risvegliato ogni volta che un thread worker finisce il suo
   lavoro; si sospende nuovamente se non tutti i worker hanno finito, oppure il dispatcher 
   non ha ancora finito oppure il collector stesso ha appena finito e il dispatcher non si 
   è ancora riattivato; altrimenti controlla se "pianeta->chronon"%"chronon_stampa"==0.
   In caso di risposta affermativa il collector stabilisce una connessione con il processo
   visualizer, controlla se è la prima connessione e quindi c'è bisogno di inviare anche
   il numero di righe e il numero di colonne, invia il carattere '0' per indicare che questa
   non è l'ultima connessione e poi invia la matrice.
   Fatto questo il thread aspetta la comunicazione che il visualizer ha finito la stampa,
   chiude la connessione, risveglia il thread dispatcher e si sospende.
*/
void funzione_collector(void* arg) {
    sprintf(num_righe, "%d", righe);
    sprintf(num_colonne, "%d", colonne);
    while (1) {
        THREADCONTROL(pthread_mutex_lock(&mutex_globale));
        while (fine_workers<pianeta->nwork || fine_dispatcher==0 || fine_collector==1) {
            DIFFZERO(pthread_cond_wait(&cond_collector, &mutex_globale));
            if (segnale_stop_ricevuto==pianeta->nwork+2) {
                    ricezione_segnale();
            }
        }
        if (segnale_stop_ricevuto==pianeta->nwork+2) {
            ricezione_segnale();
        }
        if (pianeta->chronon%chronon_stampa==0 && pianeta->chronon!=0) {
            invio_matrice();
        }
        fine_collector=1;
        THREADCONTROL(pthread_cond_signal(&cond_dispatcher));
        THREADCONTROL(pthread_mutex_unlock(&mutex_globale));
    }
}
