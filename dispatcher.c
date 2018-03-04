/**
   \file dispatcher.c
   \author Simone Paioletti
   Si dichiara che il contenuto di questo file e' in ogni sua parte opera
   originale dell' autore.
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include "wator.h"
#include "funzioni.h"

#define MENOUNO(x) if (x==-1) {perror("Errore nel dispatcher"); exit(EXIT_FAILURE);}
#define DIFFZERO(x) if (x) {perror("Errore nel dispatcher"); exit(EXIT_FAILURE);}
#define EQNULL(x) if (x==NULL) {perror("Errore nel dispatcher"); exit(EXIT_FAILURE);}
#define THREADCONTROL(x) if ((errno=x)) {perror("Errore nel thread dispatcher"); exit(EXIT_FAILURE);}

/* Questa funzione viene invocata quando viene segnalato al dispatcher che è stato
   ricevuto un segnale. Quando viene eseguita incrementa la variabile "segnale_stop_ricevuto",
   risveglia tutti i workers, libera la memoria restante allocata per la coda (se ce n'è) e
   termina.
*/

static void ricezione_segnale() {
    coda* temp;
    segnale_stop_ricevuto=2;
    fine_dispatcher=1;
    THREADCONTROL(pthread_cond_broadcast(&cond_worker));
    THREADCONTROL(pthread_mutex_unlock(&mutex_globale));
    fine_dispatcher=1;
    while (lunghezza_coda) {
        temp=inizio_coda_rettangoli;
        inizio_coda_rettangoli=inizio_coda_rettangoli->successivo;
        free(temp);
        lunghezza_coda--;
    }
    free(inizio_coda_rettangoli);
    pthread_exit(0);
}

/* Funzione per la divisione e l'inserimento dei rettangoli nella coda
   dei lavori. Il dispatcher divide la matrice pianeta in tanti rettangoli
   quanti sono i workers. 
   La divisione avviene in questo modo:
   n: numero di workers
   q: parte intera inferiore della divisione del numero delle righe per il numero di workers
   r: resto della divisione del numero delle righe per il numero di workers
   altezza rettangoli: corrisponde al numero delle colonne
   lunghezza rettangoli: r rettangoli hanno lunghezza q+1, i restanti n-r hanno lunghezza q
   
   Se l'altezza risulta essere minore di 3 righe il numero di rettangoli in cui viene divisa 
   la matrice viene diminuito finchè l'altezza non arriva ad essere di minimo 3 righe;
   questo aumenta le possibilità di lavorare in modo parallelo dei thread worker.
 
   Ogni rettangolo è rappresentato da una struct che contiene la riga che lo delimita
   superiormente e quella che lo delimita inferiormente; la struct contiene anche un
   puntatore al rettangolo successivo.
*/
static void divisione_lavori() {
    int i, altezza_rettangoli, scarto, workers_effettivi=pianeta->nwork;
    altezza_rettangoli=righe/workers_effettivi;
    while (altezza_rettangoli<3) {
        workers_effettivi--;
        altezza_rettangoli=righe/workers_effettivi;
    }
    scarto=righe%altezza_rettangoli;
    i=0;
    while (i<=righe-1) {
        fine_coda_rettangoli->riga_iniziale=i;
        if (scarto) {
            fine_coda_rettangoli->riga_finale=i+altezza_rettangoli;
            i=i+altezza_rettangoli+1;
            scarto--;
        } else {
            fine_coda_rettangoli->riga_finale=i+altezza_rettangoli-1;
            i=i+altezza_rettangoli;
        }
        EQNULL((fine_coda_rettangoli->successivo=malloc(sizeof(coda))));
        fine_coda_rettangoli=fine_coda_rettangoli->successivo;
        fine_coda_rettangoli->successivo=NULL;
        lunghezza_coda++;
    }
}

/* Il dispatcher invoca la funzione divisione_lavori per riempire la coda dei lavori;
   una volta finito setta ad 1 la variabile "fine_dispatcher", manda una broadcast
   ai workers e si sospende sulla variabile di condizione "cond_dispatcher".
 
   Se il dispatcher trova la variabile "segnale_stop_ricevuto" settata, invoca la funzione
   ricezione_segnale.
*/
void funzione_dispatcher (void* arg) {
    int i, j;
    inizio_coda_rettangoli=malloc(sizeof(coda));
    fine_coda_rettangoli=inizio_coda_rettangoli;
    while (1) {
        THREADCONTROL(pthread_mutex_lock(&mutex_globale));
        fine_collector=0;
        fine_dispatcher=0;
        fine_workers=0;
        if (segnale_stop_ricevuto==1) {
            ricezione_segnale();
        }
        for (i=0; i<righe; i++) {
            for (j=0; j<colonne; j++) {
                pianeta->update[i][j]=0;
            }
        }
        divisione_lavori();
        fine_dispatcher=1;
        THREADCONTROL(pthread_cond_broadcast(&cond_worker));
        while (lunghezza_coda || fine_collector==0) {
            THREADCONTROL(pthread_cond_wait(&cond_dispatcher, &mutex_globale));
        }
        pianeta->chronon++;
        pianeta->ns=shark_count(pianeta->plan);
        pianeta->nf=fish_count(pianeta->plan);
        THREADCONTROL(pthread_mutex_unlock(&mutex_globale));
    }
}
