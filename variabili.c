/**
   \file variabili.c
   \author Simone Paioletti
   Si dichiara che il contenuto di questo file e' in ogni sua parte opera
   originale dell' autore.
*/

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <sys/un.h>
#include "wator.h"

/* variabili globali utilizzate dal main del processo wator e dai thread.
   Di seguito il loro significato:
 
   pid = Process ID del visualizer
   canale = File descriptor della socket
   righe = Numero di righe della matrice pianeta
   colonne = Numero di colonne della matrice pianeta
   chronon_stampa = Numero di chronon che devono passare tra una stampa e l'altra (Default: 5)
   lunghezza_coda = Lunghezza della coda dei lavori pendenti
   fine_dispatcher = indica se il dispatcher ha finito il suo lavoro chronon nel chronon corrente (0 non ha finito | 1 ha finito)
   fine_workers = indica il numero di worker che hanno finito il proprio lavoro nel chronon corrente
   fine_collector = indica se il collector ha finito il suo lavoro nel chronon corrente (0 non ha finito | 1 ha finito)
   misure_inviate = Indica se le misure della matrice sono state già inviate al processo visualizer
   worker_set = Indica se il numero di worker è stato passato come argomento del processo wator
   chronon_set = Indica se il numero di chronon che devono passare tra una stampa e l'altra è stato passato come argomento del processo wator
   memoria_liberata = Indica alla funzione gestione_uscita se la memoria è già stata liberata
   dumpfile_set = Indica se è stato passato un dumpfile come argomento del processo wator
   segnale_stop_ricevuto = Viene settata ad 1 quando viene ricevuto un segnale di tipo SIGINT o SIGTERM; il dispatcher la setta a 2 prima di terminare, ogni
                           worker la incrementa di 1 prima di terminare e infine il collector la incrementa di 1 prima di terminare
   chronons = Variabile utilizzata per controllare che l'argomento passato dopo "-v" sia effettivamente un numero maggiore di 0
   workers = Variabile utilizzata per controllare che l'argomento passato dopo "-n" sia effettivamente un numero maggiore di 0
   dumpfile = Stringa che contiene il nome del dumpfile (se è stato specificato tra gli argomenti)
   file = Stringa che contiene il nome del file che contiene la configurazione iniziale del pianeta
   pianeta = Il puntatore al pianeta
   inizio_coda_rettangoli = Puntatore alla prima posizione della coda dei lavori
   fine_coda_rettangoli = Puntatore all'ultima posizione della coda dei lavori
   indirizzo = Indirizzo della socket
   dispatcher = Thread ID del dispatcher
   collector = Thread ID del collector
   segnale_stampa = Thread ID del thread che si mette in attesa del segnale SIGUSR1
   lista_worker = Array che contiene gli ID di tutti i thread worker
   mutex_globale = Variabile mutex utilizzata per garantire la mutua esclusione durante l'accesso dei thread alle variabili globali
   mutex_righe = array di variabili mutex, ogni posizione ( i ) dell'array corrisponde alla i-esima riga della matrice pianeta
   cond_dispatcher = Variabile di condizione su cui si sospende il thread dispatcher
   cond_worker = Variabile di condizione su cui si sospendono i thread worker
   cond_collector = Variabile di condizione su cui si sospende il thread collector
   maschera_segnali = La maschera per i segnali utilizzata nel main del processo wator
*/
int pid, canale, righe, colonne, chronon_stampa=0, lunghezza_coda=0, fine_dispatcher=0, fine_workers=0, fine_collector=0, misure_inviate=0, worker_set=0, chronon_set=0, dumpfile_set=0, memoria_liberata=0;
volatile sig_atomic_t segnale_stop_ricevuto=0;
long int chronons=0, workers=0;
char *dumpfile=NULL, *file=NULL;
wator_t *pianeta;
coda *inizio_coda_rettangoli, *fine_coda_rettangoli;
struct sockaddr_un indirizzo;
pthread_t dispatcher, collector, segnale_stampa, *lista_worker=NULL;
pthread_mutex_t mutex_globale=PTHREAD_MUTEX_INITIALIZER, *mutex_righe;
pthread_cond_t cond_dispatcher=PTHREAD_COND_INITIALIZER, cond_worker=PTHREAD_COND_INITIALIZER, cond_collector=PTHREAD_COND_INITIALIZER;
sigset_t maschera_segnali;
