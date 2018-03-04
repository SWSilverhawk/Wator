/**
   \file visualizer.c
   \author Simone Paioletti
   Si dichiara che il contenuto di questo file e' in ogni sua parte opera
   originale dell' autore.
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "wator.h"

#define MENOUNO(x) if (x==-1) {perror("Errore nel visualizer"); exit(EXIT_FAILURE);}
#define DIFFZERO(x) if (x) {perror("Errore nel visualizer"); exit(EXIT_FAILURE);}
#define EQNULL(x) if (x==NULL) {perror("Errore nel visualizer"); exit(EXIT_FAILURE);}
#define THREADCONTROL(x) if ((errno=x)) {perror("Errore nel visualizer"); exit(EXIT_FAILURE);}
#define UNLINKERR(x) if (x==-1) {if (errno==ENOENT) errno=0; else {perror("Errore nel visualizer"); exit(EXIT_FAILURE);}}
#define UNIX_PATH_MAX 104
#define SOCKNAME "./visual.sck"

static int canale, misure_ricevute;
static char controllo_fine;
static planet_t *pianeta=NULL;

/* Appena arriva una connessione questa funzione riceve il carattere "controllo_stop"
   controlla la variabile "misure_ricevute" per sapere se questa è la prima connessione
   e c'è bisogno di ricevere le misure della matrice e poi riceve l'intera matrice.
   A questo punto controlla su quale output va stampata la matrice e la stampa.
   Fatto questo la funzione manda un carattere al collector per comunicare che
   la stampa è finita e chiude la connessione.
*/

static void ricezione_matrice(int argc, char* argv[]) {
    int i, j, connessione_collector, contatore_celle=0;
    char num_righe[10], num_colonne[10], cella=0;
    FILE* dumpfile=NULL;
    MENOUNO((connessione_collector=accept(canale, NULL, 0)));
    DIFFZERO(sleep(1));
    MENOUNO(system("clear"));
    MENOUNO(read(connessione_collector, &controllo_fine, sizeof(char)));
    if (misure_ricevute==0) {
        EQNULL((pianeta=malloc(sizeof(planet_t))));
        MENOUNO(read(connessione_collector, num_righe, 10));
        pianeta->nrow=atoi(num_righe);
        MENOUNO(read(connessione_collector, num_colonne, 10));
        pianeta->ncol=atoi(num_colonne);
        EQNULL((pianeta->w=malloc((pianeta->nrow)*sizeof(int*))));
        for (i=0; i<pianeta->nrow; i++) {
            EQNULL((pianeta->w[i]=malloc((pianeta->ncol)*sizeof(int))));
        }
        misure_ricevute=1;
    }
    for (i=0; i<pianeta->nrow; i++) {
        for (j=0; j<pianeta->ncol; j++) {
            switch (contatore_celle) {
                case 0:
                    MENOUNO(read(connessione_collector, &cella, sizeof(char)));
                    pianeta->w[i][j]=(cella>>6)&3;
                    contatore_celle++;
                    break;
                case 1:
                    pianeta->w[i][j]=(cella>>4)&3;
                    contatore_celle++;
                    break;
                case 2:
                    pianeta->w[i][j]=(cella>>2)&3;
                    contatore_celle++;
                    break;
                case 3:
                    pianeta->w[i][j]=(cella)&3;
                    contatore_celle=0;
            }
        }
    }
    if (argc==2) {
        EQNULL((dumpfile=fopen(argv[1], "w+")));
        MENOUNO(print_planet(dumpfile, pianeta));
        DIFFZERO(fclose(dumpfile));
    } else {
        MENOUNO(color_print_planet(stdout, pianeta));
    }
    MENOUNO(write(connessione_collector, "1", sizeof(char)));
    MENOUNO(close(connessione_collector));
}

/* Il processo visualizer inizia la sua esecuzione creando la socket "visual.sck"
   e mettendosi in attesa di connessioni in entrata.
   Fatto questo invoca la funzione "ricezione_matrice" per ricevere la matrice pianeta
   poi controlla il carattere "controllo_fine" per sapere se quella avvenuta è stata 
   l'ultima connessione e, in base al suo valore, decide se ricominciare il ciclo oppure 
   terminare.
*/
int main (int argc, char* argv[]) {
    int i;
    struct sockaddr_un indirizzo;
    indirizzo.sun_family=AF_UNIX;
    strncpy(indirizzo.sun_path, SOCKNAME, UNIX_PATH_MAX);
    UNLINKERR(unlink(SOCKNAME));
    MENOUNO((canale=socket(AF_UNIX, SOCK_STREAM, 0)));
    DIFFZERO(bind(canale, (const struct sockaddr*)&indirizzo, sizeof(indirizzo)));
    DIFFZERO(listen(canale, SOMAXCONN));
    while (1) {
        ricezione_matrice(argc, argv);
        if (controllo_fine=='1') {
            for (i=0; i<pianeta->nrow; i++) {
                free(pianeta->w[i]);
            }
            free(pianeta->w);
            free(pianeta);
            close(canale);
            UNLINKERR(unlink(SOCKNAME));
            return 0;
        }
    }
    return 0;
}
