/** 
   \file funzioni.h
   \author Simone Paioletti
   Si dichiara che il contenuto di questo file e' in ogni sua parte opera
   originale dell' autore.
*/

#ifndef FUNZIONI_H
#define FUNZIONI_H

#define UNIX_PATH_MAX 104
#define WATORCHECK "wator.check"
#define SOCKNAME "./visual.sck"

extern int pid, canale, righe, colonne, chronon_stampa, lunghezza_coda, fine_dispatcher, fine_workers, fine_collector, misure_inviate, worker_set, chronon_set, dumpfile_set, memoria_liberata;
extern volatile sig_atomic_t segnale_stop_ricevuto;
extern long int chronons, workers;
extern char *dumpfile, *file;
extern wator_t *pianeta;
extern coda *inizio_coda_rettangoli, *fine_coda_rettangoli;
extern struct sockaddr_un indirizzo;
extern pthread_t dispatcher, collector, segnale_stampa, *lista_worker;
extern pthread_mutex_t mutex_globale, *mutex_righe;
extern pthread_cond_t cond_dispatcher, cond_worker, cond_collector;
extern sigset_t maschera_segnali;

void gestione_argomenti(int argc, char* argv[]);

void gestione_segnali(int numero_segnale);

void attesa_segnale_stampa();

void libera_memoria();

void gestione_uscita();

void funzione_dispatcher(void* arg);

void funzione_workers(void* arg);

void funzione_collector(void* arg);

#endif
