/** 
   \file wator.c
   \author Simone Paioletti
   Si dichiara che il contenuto di questo file e' in ogni sua parte opera
   originale dell' autore.
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include "wator.h"
#define NWORK_DEF 4
#define CHRON_DEF 5

/** macro per la generazione di numeri casuali */
#define RANDOM(x) random()%x;
#define RANDOMIZE srandom((unsigned)time(NULL));

#define MENOUNONULL(x) if (x==-1) {perror("Errore in una funzione della libreria wator"); return NULL;}
#define MENOUNOMENOUNO(x) if (x==-1) {perror("Errore in una funzione della libreria wator"); return -1;}
#define DIFFZERONULL(x) if (x) {perror("Errore in una funzione della libreria wator"); return NULL;}
#define DIFFZEROMENOUNO(x) if (x) {perror("Errore in una funzione della libreria wator"); return -1;}
#define EQNULLNULL(x) if (x==NULL) {perror("Errore in una funzione della libreria wator"); return NULL;}
#define EQNULLMENOUNO(x) if (x==NULL) {perror("Errore in una funzione della libreria wator"); return -1;}
#define EOFNULL(x) if (x==EOF) {perror("Errore in una funzione della libreria wator"); return NULL;}
#define NOCONF(x) if (x==NULL) {perror("Errore: il file wator.conf non è presente"); return NULL;}

/** Scambia il valore di due interi utilizzando i rispettivi puntatori. */
void swap(int *i, int *j) {
    int t = *i;
    *i = *j;
    *j = t;
}

/** Analizza il tipo della cella passata come parametro e restitusce il char
    corrispondente.
*/
char cell_to_char (cell_t a) {
    if (a==SHARK)
        return 'S';
    else if (a==FISH)
        return 'F';
    else if (a==WATER)
        return 'W';
    else return '?';
}

/** Analizza il carattere passato come parametro e restituisce il tipo di cella
    corrispondente.
*/
int char_to_cell (char c) {
    if (c=='S')
        return SHARK;
    else if (c=='F')
        return FISH;
    else if (c=='W')
        return WATER;
    else return -1;
}

/** Utilizza la funzione malloc per allocare dinamicamente le matrici contenute
    nella struct planet_t e le inizializza adeguatamente.
*/
planet_t* new_planet (unsigned int nrow, unsigned int ncol) {
    int i, j;
    planet_t* pianeta;
    if (nrow!=0 && ncol!=0) {
        errno=0;
        EQNULLNULL((pianeta=malloc(sizeof(planet_t))));
        pianeta->nrow=nrow;
        pianeta->ncol=ncol;
        EQNULLNULL((pianeta->w=malloc(nrow*sizeof(cell_t*))));
        EQNULLNULL((pianeta->btime=malloc(nrow*sizeof(int*))));
        EQNULLNULL((pianeta->dtime=malloc(nrow*sizeof(int*))));
        for (i=0; i<nrow; i++) {
            EQNULLNULL((pianeta->w[i]=malloc(ncol*sizeof(cell_t))));
            EQNULLNULL((pianeta->btime[i]=malloc(ncol*sizeof(int))));
            EQNULLNULL((pianeta->dtime[i]=malloc(ncol*sizeof(int))));
            for (j=0; j<ncol; j++) {
                pianeta->w[i][j]=WATER;
                pianeta->btime[i][j]=0;
                pianeta->dtime[i][j]=0;
            }
        }
        return pianeta;
    }
    errno=EINVAL;
    return NULL;
}

/** Utilizza la funzione free per deallocare la memoria che era stata allocata
    dalla funzione new_planet.
*/
void free_planet (planet_t* p) {
    int i;
    if (p!=NULL) {
        for (i=0; i<p->nrow; i++) {
            free(p->w[i]);
            free(p->btime[i]);
            free(p->dtime[i]);
        }
        free(p->w);
        free(p->btime);
        free(p->dtime);
        free(p);
    }
}

/** Per prima cosa controlla che i parametri siano corretti: se non lo sono setta errno a
    EINVAL (tutte le funzioni da qui in poi settano in questo modo errno se ci sono problemi
    con i parametri) e restituisce -1, altrimenti stampa il pianeta sul file f secondo il
    formato indicato dalle specifiche e restituisce 0.
*/
int print_planet (FILE* f, planet_t* p) {
    int i, j;
    char cella;
    if (f!=NULL && p!=NULL) {
        errno=0;
        MENOUNOMENOUNO(fprintf(f, "%d\n", p->nrow));
        MENOUNOMENOUNO(fprintf(f, "%d\n", p->ncol));
        for (i=0; i<p->nrow; i++) {
            for (j=0; j<(p->ncol)-1; j++) {
                cella=cell_to_char(p->w[i][j]);
                MENOUNOMENOUNO(fprintf(f, "%c ", cella));
            }
            cella=cell_to_char(p->w[i][j]);
            MENOUNOMENOUNO(fprintf(f, "%c\n", cella));
        }
        return 0;
    }
    errno=EINVAL;
    return -1;
}

/** Funzione identica alla print_planet ma che stampa il pianeta utilizzando
    caratteri colorati.
*/
int color_print_planet (FILE* f, planet_t* p) {
    int i, j;
    char cella;
    if (f!=NULL && p!=NULL) {
        errno=0;
        MENOUNOMENOUNO(fprintf(f, "%d\n", p->nrow));
        MENOUNOMENOUNO(fprintf(f, "%d\n", p->ncol));
        for (i=0; i<p->nrow; i++) {
            for (j=0; j<(p->ncol)-1; j++) {
                cella=cell_to_char(p->w[i][j]);
                if (cella=='W') {
                MENOUNOMENOUNO(fprintf(f, CIANO"%c "RESET, cella));
                } else if (cella=='S') {
                    MENOUNOMENOUNO(fprintf(f, BIANCO"%c "RESET, cella));
                } else {
                    MENOUNOMENOUNO(fprintf(f, GIALLO"%c "RESET, cella));
                }
            }
            cella=cell_to_char(p->w[i][j]);
            if (cella=='W') {
                MENOUNOMENOUNO(fprintf(f, CIANO"%c"RESET"\n", cella));
            } else if (cella=='S') {
                MENOUNOMENOUNO(fprintf(f, BIANCO"%c"RESET"\n", cella));
            } else {
                MENOUNOMENOUNO(fprintf(f, GIALLO"%c"RESET"\n", cella));
            }
        }
        return 0;
    }
    errno=EINVAL;
    return -1;
}

/** Carica un pianeta da file leggendo prima il numero di righe e di colonne, poi legge
    la matrice carattere per carattere costruendo progressivamente il pianeta.
*/
planet_t* load_planet (FILE* f) {
    int i, j;
    unsigned int righe, colonne;
    char cella;
    planet_t* pianeta;
    if (f!=NULL) {
        errno=0;
        EOFNULL(fscanf(f, "%ud", &righe));
        EOFNULL(fscanf(f, "%ud", &colonne));
        EQNULLNULL((pianeta=new_planet(righe, colonne)));
        MENOUNONULL(fseek(f, sizeof(char), SEEK_CUR));
        for (i=0; i<righe; i++) {
            for (j=0; j<colonne; j++) {
                EOFNULL(fscanf(f, "%c", &cella));
                if (char_to_cell(cella)==-1) {
                    errno=ERANGE;
                    return NULL;
                }
                pianeta->w[i][j]=char_to_cell(cella);
                MENOUNONULL(fseek(f, sizeof(char), SEEK_CUR));
            }
        }
        return pianeta;
    }
    errno=EINVAL;
    return NULL;
}

/** Crea una nuova istanza di wator chiamando la funzione load_planet per caricare
    il pianeta contenuto in fileplan, poi setta nwork e nchronon secondo i valori
    predefiniti e sd, sb e sb secondo i valori contenuti in CONFIGURATION_FILE.
*/
wator_t* new_wator (char* fileplan) {
    int i, sd, sb, fb;
    char stringa[3];
    FILE *pianeta, *configurazione;
    wator_t* wator;
    if (fileplan!=NULL) {
        errno=0;
        RANDOMIZE;
        EQNULLNULL((wator=malloc(sizeof(wator_t))));
        EQNULLNULL((pianeta=fopen(fileplan, "r")));
        NOCONF((configurazione=fopen(CONFIGURATION_FILE, "r")));
        EQNULLNULL((wator->plan=load_planet(pianeta)));
        MENOUNONULL((wator->ns=shark_count(wator->plan)));
        MENOUNONULL((wator->nf=fish_count(wator->plan)));
        wator->nwork=NWORK_DEF;
        wator->chronon=0;
        EQNULLNULL((wator->update=malloc(wator->plan->nrow*sizeof(int*))));
        for (i=0; i<wator->plan->nrow; i++) {
            EQNULLNULL((wator->update[i]=malloc(wator->plan->ncol*sizeof(int))));
        }
        for (i=0; i<3; i++) {
            EOFNULL(fscanf(configurazione, "%s", stringa));
            sd=strcmp(stringa, "sd");
            sb=strcmp(stringa, "sb");
            fb=strcmp(stringa, "fb");
            if (sd==0) {
                EOFNULL(fscanf(configurazione, "%d", &wator->sd));
            }
            else if (sb==0) {
                EOFNULL(fscanf(configurazione, "%d", &wator->sb));
            }
            else if (fb==0) {
                EOFNULL(fscanf(configurazione, "%d", &wator->fb));
            }
            else {
                errno=ERANGE;
                return NULL;
            }
        }
        EOFNULL(fclose(pianeta));
        EOFNULL(fclose(configurazione));
        return wator;
    }
    errno=EINVAL;
    return NULL;
}

/** Utilizza la funzione free per deallocare la memoria che era stata allocata
    dalla funzione new_wator.
*/
void free_wator(wator_t* pw) {
    int i;
    if (pw!=NULL) {
        for (i=0; i<pw->plan->nrow; i++) {
            free(pw->update[i]);
        }
        free(pw->update);
        free_planet(pw->plan);
        free(pw);
    }
}

/** Calcola il modulo di un numero; a differenza dell'operatore %
    questa funzione calcola il modulo positivo anche dei primi
    numeri negativi.
*/
int modulo (int dividendo, int divisore) {
    dividendo=dividendo+divisore;
    return dividendo%divisore;
}

/** Riempie un array di struct in cui si trovano le coordinate dei vicini
    della cella stessa.
*/
void crea_vicini (wator_t* pw, struct celle_adiacenti* vicini, int x, int y) {
    int righe, colonne;
    righe=pw->plan->nrow;
    colonne=pw->plan->ncol;
    vicini[0].tipo=pw->plan->w[modulo(x-1, righe)][y];
    vicini[0].riga=modulo(x-1, righe);
    vicini[0].colonna=y;
    vicini[1].tipo=pw->plan->w[x][modulo(y+1, colonne)];
    vicini[1].riga=x;
    vicini[1].colonna=modulo(y+1, colonne);
    vicini[2].tipo=pw->plan->w[modulo(x+1, righe)][y];
    vicini[2].riga=modulo(x+1, righe);
    vicini[2].colonna=y;
    vicini[3].tipo=pw->plan->w[x][modulo(y-1, colonne)];
    vicini[3].riga=x;
    vicini[3].colonna=modulo(y-1, colonne);
}

/** Chiama crea_vicini per creare un array con le informazioni sui vicini della
    cella di coordinate (x,y) e poi lo scorre, partendo da un indice scelto casualmente,
    per vedere se in una cella vicina si trova un pesce; se questo si verifica lo squalo 
    mangia il pesce, vengono aggiornate le matrici pianeta, btime e dtime, vengono settati 
    k e l e viene restituito EAT.
 
    Se non viene trovato nessun pesce, la funzione scorre di nuovo l'array per vedere se
    una delle celle vicine è libera; se questo si verifica lo squalo si sposta in quella
    posizione, vengono aggiornate le matrici pianeta, btime e dtime, vengono settati k e l
    e viene restituito MOVE.
 
    Se non viene trovata nessuna posizione libera viene restituito STOP.
*/
int shark_rule1 (wator_t* pw, int x, int y, int* k, int* l) {
    int i, inizio;
    struct celle_adiacenti vicini[4];
    *k=x;
    *l=y;
    if (pw!=NULL && x>=0 && y>=0 && k!=NULL && l!=NULL && pw->plan->w[x][y]==SHARK) {
        crea_vicini(pw, vicini, x, y);
        inizio=RANDOM(4);
        for (i=0; i<4; i++) {
            inizio=inizio%4;
            if (vicini[inizio].tipo==FISH) {
                pw->plan->w[vicini[inizio].riga][vicini[inizio].colonna]=SHARK;
                pw->plan->w[x][y]=WATER;
                swap(&pw->plan->btime[x][y], &pw->plan->btime[vicini[inizio].riga][vicini[inizio].colonna]);
                pw->plan->dtime[x][y]=0;
                pw->plan->dtime[vicini[inizio].riga][vicini[inizio].colonna]=0;
                *k=vicini[inizio].riga;
                *l=vicini[inizio].colonna;
                return EAT;
            }
            inizio++;
        }
        for (i=0; i<4; i++) {
            inizio=inizio%4;
            if (vicini[inizio].tipo==WATER) {
                pw->plan->w[vicini[inizio].riga][vicini[inizio].colonna]=SHARK;
                pw->plan->w[x][y]=WATER;
                swap(&pw->plan->btime[x][y], &pw->plan->btime[vicini[inizio].riga][vicini[inizio].colonna]);
                swap(&pw->plan->dtime[x][y], &pw->plan->dtime[vicini[inizio].riga][vicini[inizio].colonna]);
                *k=vicini[inizio].riga;
                *l=vicini[inizio].colonna;
                return MOVE;
            }
            inizio++;
        }
        return STOP;
    }
    errno=EINVAL;
    return -1;
}

/** Controlla se btime (x,y) è minore di pw->sb; se questo si verifica incrementa btime (x,y),
    altrimenti chiama crea_vicini per creare un array con le informazioni sui vicini
    della cella di coordinate (x,y) e poi lo scorre, partendo da un indice scelto casualmente,
    per vedere se una delle celle vicine è libera; se questo si verifica lo squalo si riproduce,
    vengono aggiornate le matrici pianeta, btime e dtime evengono settati k e l.

    A questo punto la funzione controlla se dtime (x,y) è minore di pw->sd; se questo si
    verifica incrementa dtime (x,y) e restituisce ALIVE, altrimenti lo squalo muore, la
    cella (x,y) diventa libera e viene restituito DEAD.
*/
int shark_rule2 (wator_t* pw, int x, int y, int* k, int* l) {
    int i, inizio;
    struct celle_adiacenti vicini[4];
    *k=-1;
    *l=-1;
    if (pw!=NULL && x>=0 && y>=0 && k!=NULL && l!=NULL && pw->plan->w[x][y]==SHARK) {
        if (pw->plan->btime[x][y]<pw->sb) {
            pw->plan->btime[x][y]++;
        } else {
            crea_vicini(pw, vicini, x, y);
            inizio=RANDOM(4);
            for (i=0; i<4; i++) {
                inizio=inizio%4;
                if (vicini[inizio].tipo==WATER) {
                    pw->plan->w[vicini[inizio].riga][vicini[inizio].colonna]=SHARK;
                    pw->plan->btime[vicini[inizio].riga][vicini[inizio].colonna]=0;
                    pw->plan->dtime[vicini[inizio].riga][vicini[inizio].colonna]=0;
                    *k=vicini[inizio].riga;
                    *l=vicini[inizio].colonna;
                    break;
                }
                inizio++;
            }
            pw->plan->btime[x][y]=0;
        }
        if (pw->plan->dtime[x][y]<pw->sd) {
            pw->plan->dtime[x][y]++;
            return ALIVE;
        } else {
            pw->plan->w[x][y]=WATER;
            pw->plan->dtime[x][y]=0;
            return DEAD;
        }
    }
    errno=EINVAL;
    return -1;
}

/** Chiama crea_vicini per creare un array con le informazioni sui vicini della
    cella di coordinate (x,y) e poi lo scorre, partendo da un indice scelto casualmente,
    per vedere se una delle celle vicine è libera; se questo si verifica il pesce si sposta,
    vengono aggiornate le matrici pianeta, btime e dtime, vengono settati k e l,
    viene restituito MOVE.
 
    Se non viene trovata nessuna posizione libera viene retituito STOP.
*/
int fish_rule3 (wator_t* pw, int x, int y, int* k, int* l) {
    int i, inizio;
    struct celle_adiacenti vicini[4];
    *k=x;
    *l=y;
    if (pw!=NULL && x>=0 && y>=0 && k!=NULL && l!=NULL && pw->plan->w[x][y]==FISH) {
        crea_vicini(pw, vicini, x, y);
        inizio=RANDOM(4);
        for (i=0; i<4; i++) {
            inizio=inizio%4;
            if (vicini[inizio].tipo==WATER) {
                pw->plan->w[x][y]=WATER;
                pw->plan->w[vicini[inizio].riga][vicini[inizio].colonna]=FISH;
                swap(&pw->plan->btime[x][y], &pw->plan->btime[vicini[inizio].riga][vicini[inizio].colonna]);
                swap(&pw->plan->dtime[x][y], &pw->plan->dtime[vicini[inizio].riga][vicini[inizio].colonna]);
                *k=vicini[inizio].riga;
                *l=vicini[inizio].colonna;
                return MOVE;
            }
            inizio++;
        }
        return STOP;
    }
    errno=EINVAL;
    return -1;
}

/** Controlla se btime (x,y) è minore di pw->fb; se questo si verifica incrementa btime (x,y),
    altrimenti chiama crea_vicini per creare un array con le informazioni sui vicini
    della cella di coordinate (x,y) e poi lo scorre, partendo da un indice scelto casualmente,
    per vedere se una delle celle vicine è libera; in questo caso il pesce si riproduce,
    vengono aggiornate le matrici pianeta, btime e dtime e vengono settati k e l.
 
    Se non ci sono stati errori la funzione restituisce 0, altrimenti restituisce -1.
*/

int fish_rule4 (wator_t* pw, int x, int y, int* k, int* l) {
    int i, inizio;
    struct celle_adiacenti vicini[4];
    *k=-1;
    *l=-1;
    if (pw!=NULL && x>=0 && y>=0 && k!=NULL && l!=NULL && pw->plan->w[x][y]==FISH) {
        if (pw->plan->btime[x][y]<pw->fb) {
            pw->plan->btime[x][y]++;
        } else {
            crea_vicini(pw, vicini, x, y);
            inizio=RANDOM(4);
            for (i=0; i<4; i++) {
                inizio=inizio%4;
                if (vicini[inizio].tipo==WATER) {
                    pw->plan->w[vicini[inizio].riga][vicini[inizio].colonna]=FISH;
                    pw->plan->btime[vicini[inizio].riga][vicini[inizio].colonna]=0;
                    pw->plan->dtime[vicini[inizio].riga][vicini[inizio].colonna]=0;
                    *k=vicini[inizio].riga;
                    *l=vicini[inizio].colonna;
                    break;
                }
                inizio++;
            }
            pw->plan->btime[x][y]=0;
        }
        return 0;
    }
    errno=EINVAL;
    return -1;
}

/** Scorre la matrice aumentando di una unità la variabile nfish ogni volta che trova
    una cella con un pesce e alla fine restituisce questa variabile.
*/
int fish_count (planet_t* p) {
    int i, j, numero_pesci=0;
    if (p!=NULL) {
        for (i=0; i<p->nrow; i++) {
            for (j=0; j<p->ncol; j++) {
                if (p->w[i][j]==FISH) {
                    numero_pesci++;
                }
            }
        }
        return numero_pesci;
    }
    errno=EINVAL;
    return -1;
}

/** Scorre la matrice aumentando di una unità la variabile nshark ogni volta che trova
    una cella con uno squalo e alla fine restituisce questa variabile.
*/
int shark_count (planet_t* p) {
    int i, j, numero_squali=0;
    if (p!=NULL) {
        for (i=0; i<p->nrow; i++) {
            for (j=0; j<p->ncol; j++) {
                if (p->w[i][j]==SHARK) {
                    numero_squali++;
                }
            }
        }
        return numero_squali;
    }
    errno=EINVAL;
    return -1;

}

/** Azzera la matrice update, in seguito scorre la matrice pianeta e per ogni cella decide
    le azioni da eseguire distinguendo i seguenti due casi:
 
    Caso 1: w(i,j)==SHARK && update(i,j)==0 -> la funzione applica la Regola 2 alla cella (i,j)
                                               e setta ad 1 update(k,l), dove k e l sono le
                                               eventuali coordinate dello squalo figlio ricavate 
                                               dalla Regola 2. In seguito, se lo squalo è ancora
                                               vivo, applica la Regola 1 alla cella (i,j) e setta
                                               ad 1 update(k,l), dove k e l sono le nuove coordinate
                                               dello squalo ricavate dalla Regola 1.
                                               Per finire aggiorna il numero degli squali e dei pesci.
 
    Caso 2: w(i,j)==FISH && update(i,j)==0  -> la funzione applica la Regola 4 alla cella (i,)
                                               e setta ad 1 update(k,l), dove k e l sono le
                                               eventuali coordinate del pesce figlio ricavate
                                               dalla Regola 4. In seguito applica la Regola 3
                                               alla cella (i,j) e setta ad 1 update(k,l), dove
                                               k e l sono le nuove coordinate del pesce ricavate
                                               dalla Regola 3.
                                               Per finire aggiorna il numero degli squali e dei pesci.
 
   In tutti gli altri casi la funzione non esegue nessuna azione e passa direttamente all'iterazione
   seguente.
*/
int update_wator (wator_t * pw) {
    int i, j, k, l, azione;
    if (pw!=NULL) {
        errno=0;
        for (i=0; i<pw->plan->nrow; i++) {
            for (j=0; j<pw->plan->ncol; j++) {
                pw->update[i][j]=0;
            }
        }
        for (i=0; i<pw->plan->nrow; i++) {
            for (j=0; j<pw->plan->ncol; j++) {
                if (pw->plan->w[i][j]==SHARK && pw->update[i][j]==0) {
                    MENOUNOMENOUNO(shark_rule2(pw, i, j, &k, &l));
                    if (k!=-1 && l!=-1)
                        pw->update[k][l]=1;
                    if (pw->plan->w[i][j]==SHARK) {
                        MENOUNOMENOUNO((azione=shark_rule1(pw, i, j, &k, &l)));
                        if (azione==EAT || azione==MOVE)
                            pw->update[k][l]=1;
                    }
                }
                if (pw->plan->w[i][j]==FISH && pw->update[i][j]==0) {
                    MENOUNOMENOUNO(fish_rule4(pw, i, j, &k, &l));
                    if (k!=-1 && l!=-1)
                        pw->update[k][l]=1;
                    azione=fish_rule3(pw, i, j, &k, &l);
                    if (azione==MOVE)
                        pw->update[k][l]=1;
                }
            }
        }
        MENOUNOMENOUNO((pw->nf=fish_count(pw->plan)));
        MENOUNOMENOUNO((pw->ns=shark_count(pw->plan)));
        return 0;
    }
    errno=EINVAL;
    return -1;
}
