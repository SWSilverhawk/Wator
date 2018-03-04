/**
   \file gestore_main.c
   \author Simone Paioletti
   Si dichiara che il contenuto di questo file e' in ogni sua parte opera
   originale dell' autore.
*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "wator.h"
#include "funzioni.h"

/* Serie di macro che controllano il valore di ritorno delle funzioni e delle chiamate di sistema,
   se il valore restituito indica la presenza di errori viene stampato il codice dell'errore e
   il programma termina (Negli altri file sono definite le stesse macro, differenti
   da queste solo per la stringa passata come argomento alla perror).
*/
#define MENOUNO(x) if (x==-1) {perror("Errore nel main"); exit(EXIT_FAILURE);}
#define DIFFZERO(x) if (x) {perror("Errore nel main"); exit(EXIT_FAILURE);}
#define EQNULL(x) if (x==NULL) {perror("Errore main"); exit(EXIT_FAILURE);}
#define THREADCONTROL(x) if ((errno=x)) {perror("Errore nel main"); exit(EXIT_FAILURE);}

/* Questa funzione crea i thread dispatcher, worker, collector e il thread che sta in attesa del segnale
   SIGUSR1 e poi si mette in attesa della loro terminazione.
   Appena i thread dispatcher, worker e collector terminano, la funzione manda il segnale SIGINT al
   thread segnale_stampa per farlo terminare.
*/
void creazione_attesa_thread() {
    int i;
    long int wid;
    EQNULL((lista_worker=malloc(pianeta->nwork*sizeof(pthread_t))));
    EQNULL((mutex_righe=malloc(righe*sizeof(pthread_mutex_t))));
    for (i=0; i<pianeta->plan->nrow; i++) {
        THREADCONTROL(pthread_mutex_init(&mutex_righe[i], NULL));
    }
    for (i=0; i<pianeta->nwork; i++) {
        wid=i;
        THREADCONTROL(pthread_create(&lista_worker[i], NULL, (void*(*)(void*))&funzione_workers, (void*)wid));
    }
    THREADCONTROL(pthread_create(&dispatcher, NULL, (void*(*)(void*))&funzione_dispatcher, NULL));
    THREADCONTROL(pthread_create(&collector, NULL, (void*(*)(void*))&funzione_collector, NULL));
    THREADCONTROL(pthread_create(&segnale_stampa, NULL, (void*(*)(void*))&attesa_segnale_stampa, NULL));
    THREADCONTROL(pthread_join(dispatcher, NULL));
    for (i=0; i<pianeta->nwork; i++) {
        THREADCONTROL(pthread_join(lista_worker[i], NULL));
    }
    THREADCONTROL(pthread_join(collector, NULL));
    DIFFZERO(pthread_kill(segnale_stampa, SIGINT));
    THREADCONTROL(pthread_join(segnale_stampa, NULL));
}

/* Il main si occupa di generare un processo figlio, il quale eseguirà poi il visualizer,
   di creare i thread dispatcher, collector e visualizer e di gestire i segnali.
   Inizialmente registra una funzione di uscita per terminare anche il processo visualizer
   in caso di terminazione errata del processo wator, imposta la signal mask in modo
   da bloccare tutti i segnali ed esegue una fork.
   A questo punto il processo figlio esegue il visualizer mentre il processo padre
   reimposta la signal mask in modo da gestire i segnali SIGINT, SIGTERM e invoca
   la funzione creazione_attesa_thread.
   Prima di terminare il processo invoca la funzione libera_memoria per liberare la memoria
   allocata precedentemente.
*/
int main (int argc, char* argv[]) {
    struct sigaction segnale;
    gestione_argomenti(argc, argv);
    DIFFZERO(atexit(&gestione_uscita));
    MENOUNO(sigfillset(&maschera_segnali));
    segnale.sa_handler=gestione_segnali;
    segnale.sa_mask=maschera_segnali;
    segnale.sa_flags=0;
    DIFFZERO(pthread_sigmask(SIG_SETMASK, &maschera_segnali, NULL));
    MENOUNO((pid=fork()));
    if (pid==0) {
        if (dumpfile_set==1) {
            DIFFZERO(execl("./visualizer", "./visualizer", dumpfile, NULL));
        } else {
            DIFFZERO(execl("./visualizer", "./visualizer", NULL));
        }
    } else {
        EQNULL((pianeta=new_wator(file)));
        righe=pianeta->plan->nrow;
        colonne=pianeta->plan->ncol;
        if (worker_set==1) {
            pianeta->nwork=(int)workers;
        }
        if (chronon_set==1) {
            chronon_stampa=(int)chronons;
        } else {
            chronon_stampa=CHRON_DEF;
        }
        MENOUNO(sigemptyset(&maschera_segnali));
        MENOUNO(sigaddset(&maschera_segnali, SIGUSR1));
        DIFFZERO(pthread_sigmask(SIG_SETMASK, &maschera_segnali, NULL));
        DIFFZERO(sigaction(SIGINT, &segnale, NULL));
        DIFFZERO(sigaction(SIGTERM, &segnale, NULL));
        indirizzo.sun_family=AF_UNIX;
        strncpy(indirizzo.sun_path, SOCKNAME, UNIX_PATH_MAX);
        creazione_attesa_thread();
    }
    libera_memoria();
    return 0;
}
