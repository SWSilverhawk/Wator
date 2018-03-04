/**
   \file gestore_argomenti.c
   \author Simone Paioletti
   Si dichiara che il contenuto di questo file e' in ogni sua parte opera
   originale dell' autore.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include "wator.h"
#include "funzioni.h"

#define EQNULL(x) if (x==NULL) {perror("Errore nel main"); exit(EXIT_FAILURE);}
#define USAGE {printf("Usage: wator file [-n nwork] [-v chronon] [-f dumpfile]\n"); exit(EXIT_FAILURE);}

/* Questa funzione effettua il parsing degli argomenti passati al processo wator.
   Controlla che siano nel giusto ordine, che siano sintatticamente corretti e
   che il file passato come primo argomento esista.
*/
void gestione_argomenti(int argc, char* argv[]) {
    int i;
    char *controllo=NULL;
    if (argc<2 || argc==3 || argc==5 || argc==7 || argc>8) {
        USAGE;
    }
    EQNULL((file=malloc((strlen(argv[1])+1)*sizeof(char))));
    strcpy(file, argv[1]);
    for (i=2; i<argc; i=i+2) {
        if (strcmp(argv[i], "-n")==0 && worker_set==0) {
            worker_set=1;
            workers=strtol(argv[i+1], &controllo, 10);
            if (*controllo || workers<=0) {
                USAGE;
            }
        } else if (strcmp(argv[i], "-v")==0 && chronon_set==0) {
            chronon_set=1;
            chronons=strtol(argv[i+1], &controllo, 10);
            if (*controllo || chronons<=0) {
                USAGE;
            }
        } else if (strcmp(argv[i], "-f")==0 && dumpfile_set==0) {
            dumpfile_set=1;
            EQNULL((dumpfile=malloc((strlen(argv[i+1])+1)*sizeof(char))));
            strcpy(dumpfile, argv[i+1]);
        } else {
            USAGE;
        }
    }
    if (file==NULL) {
        USAGE;
    }
    if (file!=NULL && access(file, F_OK)==-1) {
        USAGE;
    }
}
