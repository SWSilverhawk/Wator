/**
   \file workers.c
   \author Simone Paioletti
   Si dichiara che il contenuto di questo file e' in ogni sua parte opera
   originale dell' autore.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include "wator.h"
#include "funzioni.h"

#define MENOUNO(x) if (x==-1) {perror("Errore in un thread worker"); exit(EXIT_FAILURE);}
#define DIFFZERO(x) if (x) {perror("Errore in un thread worker"); exit(EXIT_FAILURE);}
#define EQNULL(x) if (x==NULL) {perror("Errore in un thread worker"); exit(EXIT_FAILURE);}
#define THREADCONTROL(x) if ((errno=x)) {perror("Errore in un thread worker"); exit(EXIT_FAILURE);}
#define MENOUNOMENOUNO(x) if (x==-1) {perror("Errore in un thread worker"); exit(EXIT_FAILURE);}

/* Questa funzione viene invocata quando viene segnalato ai workers che è stato
   ricevuto un segnale. Ogni thread worker che la esegue incrementa le variabili
   "segnale_stop_ricevuto" e "fine_workers", risveglia il thread collector se
   tutti gli altri worker hanno finito e poi termina.
*/
static void ricezione_segnale() {
    segnale_stop_ricevuto++;
    fine_workers++;
    if (fine_workers==pianeta->nwork) {
        THREADCONTROL(pthread_cond_signal(&cond_collector));
    }
    THREADCONTROL(pthread_mutex_unlock(&mutex_globale));
    pthread_exit(0);
}

/* Questa funzione modifica la riga "lavoro_attuale" del pianeta invocando le funzioni
   shark_rule1, shark_rule2, fish_rule3, fish_rule4 allo stesso modo della funzione
   update_wator.
*/
static void modifica_pianeta(int lavoro_attuale) {
    int i, azione, nuova_riga, nuova_colonna;
    for (i=0; i<colonne; i++) {
        if (pianeta->plan->w[lavoro_attuale][i]==SHARK && pianeta->update[lavoro_attuale][i]==0) {
            MENOUNOMENOUNO(shark_rule2(pianeta, lavoro_attuale, i, &nuova_riga, &nuova_colonna));
            if (nuova_riga!=-1 && nuova_colonna!=-1)
                pianeta->update[nuova_riga][nuova_colonna]=1;
            if (pianeta->plan->w[lavoro_attuale][i]==SHARK) {
                MENOUNOMENOUNO((azione=shark_rule1(pianeta, lavoro_attuale, i, &nuova_riga, &nuova_colonna)));
                if (azione==EAT || azione==MOVE)
                    pianeta->update[nuova_riga][nuova_colonna]=1;
            }
        }
        if (pianeta->plan->w[lavoro_attuale][i]==FISH && pianeta->update[lavoro_attuale][i]==0) {
            MENOUNOMENOUNO(fish_rule4(pianeta, lavoro_attuale, i, &nuova_riga, &nuova_colonna));
            if (nuova_riga!=-1 && nuova_colonna!=-1)
                pianeta->update[nuova_riga][nuova_colonna]=1;
            MENOUNOMENOUNO((azione=fish_rule3(pianeta, lavoro_attuale, i, &nuova_riga, &nuova_colonna)));
            if (azione==MOVE)
                pianeta->update[nuova_riga][nuova_colonna]=1;
        }
    }
}

/* Questa funzione si occupa di gestire la modifica del pianeta. Per ogni riga assegnata
   al worker la funzione controlla di quali lock c'è bisogno per modificarla garantendo 
   la mutua esclusione e poi invoca la funzione modifica_pianeta per eseguire le adeguate
   modifiche sulla riga.
*/
static void gestione_modifica(int inizio_lavoro, int fine_lavoro) {
    int lavoro_attuale=0;
    lavoro_attuale=inizio_lavoro;
    THREADCONTROL(pthread_mutex_lock(&mutex_righe[modulo(inizio_lavoro-1, righe)]));
    THREADCONTROL(pthread_mutex_lock(&mutex_righe[inizio_lavoro]));
    modifica_pianeta(lavoro_attuale);
    lavoro_attuale++;
    THREADCONTROL(pthread_mutex_unlock(&mutex_righe[modulo(inizio_lavoro-1, righe)]));
    if (fine_lavoro-inizio_lavoro==2) {
        THREADCONTROL(pthread_mutex_lock(&mutex_righe[fine_lavoro]));
        modifica_pianeta(lavoro_attuale);
        lavoro_attuale++;
        THREADCONTROL(pthread_mutex_unlock(&mutex_righe[inizio_lavoro]));
        THREADCONTROL(pthread_mutex_lock(&mutex_righe[modulo(fine_lavoro+1, righe)]));
        modifica_pianeta(lavoro_attuale);
        THREADCONTROL(pthread_mutex_unlock(&mutex_righe[fine_lavoro]));
        THREADCONTROL(pthread_mutex_unlock(&mutex_righe[modulo(fine_lavoro+1, righe)]));
    } else {
        modifica_pianeta(lavoro_attuale);
        lavoro_attuale++;
        THREADCONTROL(pthread_mutex_unlock(&mutex_righe[inizio_lavoro]));
        for (; lavoro_attuale<fine_lavoro-1; lavoro_attuale++) {
            modifica_pianeta(lavoro_attuale);
        }
        THREADCONTROL(pthread_mutex_lock(&mutex_righe[fine_lavoro]));
        modifica_pianeta(lavoro_attuale);
        lavoro_attuale++;
        THREADCONTROL(pthread_mutex_lock(&mutex_righe[modulo(fine_lavoro+1, righe)]));
        modifica_pianeta(lavoro_attuale);
        THREADCONTROL(pthread_mutex_unlock(&mutex_righe[fine_lavoro]));
        THREADCONTROL(pthread_mutex_unlock(&mutex_righe[modulo(fine_lavoro+1, righe)]));
    }
}

/* I workers cominciano creando il file "wator_worker_wid", poi entrano in un ciclo
   infinito all'interno del quale controllano se la coda dei lavori contiene elementi;
   in questo caso prendono un lavoro, rilasciano la mutua esclusione e lavorano sul  
   rettangolo corrispondente a quel lavoro.
   Quando la coda viene svuotata i worker incrementano la variabile "fine_workers",
   inviano una signal al collector e si sospendono sulla variabile di condizione
   "cond_worker".
   Prima di controllare se ci sono lavori in coda i thread controllano sempre il valore
   della variabile "segnale_stop_ricevuto"; se è uguale a 2 viene invocata la
   funzione "ricezione_segnale".
*/
void funzione_workers (void* arg) {
    int inizio_lavoro=0, fine_lavoro=0;
    long wid=0;
    FILE* file;
    coda* temp;
    char numero_thread[4], thread_file[17];
    wid=(long)arg;
    bzero(numero_thread, 4);
    bzero(thread_file, 17);
    strncpy(thread_file, "wator_worker_", 13);
    sprintf(numero_thread, "%ld", wid);
    strncat(thread_file, numero_thread, sizeof(thread_file)-strlen(thread_file)-1);
    EQNULL((file=fopen(thread_file, "w+")));
    DIFFZERO(fclose(file));
    while (1) {
        THREADCONTROL(pthread_mutex_lock(&mutex_globale));
        while (fine_dispatcher==0 || fine_workers!=0) {
            fine_workers++;
            if (fine_workers==pianeta->nwork) {
                THREADCONTROL(pthread_cond_signal(&cond_collector));
            }
            THREADCONTROL(pthread_cond_wait(&cond_worker, &mutex_globale));
            if (segnale_stop_ricevuto>=2) {
                ricezione_segnale();
            }
        }
        if (!lunghezza_coda && fine_dispatcher==1) {
            fine_workers++;
            if (fine_workers==pianeta->nwork) {
                THREADCONTROL(pthread_cond_signal(&cond_collector));
            }
            THREADCONTROL(pthread_cond_wait(&cond_worker, &mutex_globale));
            if (segnale_stop_ricevuto>=2) {
                ricezione_segnale();
            }
        }
        if (lunghezza_coda) {
            inizio_lavoro=inizio_coda_rettangoli->riga_iniziale;
            fine_lavoro=inizio_coda_rettangoli->riga_finale;
            temp=inizio_coda_rettangoli;
            inizio_coda_rettangoli=inizio_coda_rettangoli->successivo;
            free(temp);
            lunghezza_coda--;
        }
        THREADCONTROL(pthread_mutex_unlock(&mutex_globale));
        if (fine_lavoro!=0) {
            gestione_modifica(inizio_lavoro, fine_lavoro);
        }
    }
}
