#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "segdef.h"

void calculer_moyenne_locale(segment *segptr, double *moyenne_locale);
void afficher_resultats(segment *segptr, double moyenne_locale);
double moyenne_locale_arrondie;

int main() {
    int shmid, semid;  
    segment *segptr;   
    double moyenne_locale;
    int req;

    
    if ((shmid = shmget(cle, segsize, 0666)) == -1) {
        perror("Erreur : impossible de récupérer l'identifiant du segment mémoire");
        exit(EXIT_FAILURE);
    }

    if ((segptr = (segment *)shmat(shmid, NULL, 0)) == (segment *)-1) {
        perror("Erreur : impossible d'attacher le segment mémoire");
        exit(EXIT_FAILURE);
    }

    if ((semid = semget(cle, 3, 0666)) == -1) {
        perror("Erreur : impossible de récupérer l'identifiant des sémaphores");
        exit(EXIT_FAILURE);
    }

    init_rand();


    for (req = 1; req <= 5; req++) {
        acq_sem(semid, seg_dispo);

        segptr->pid = getpid();
        segptr->req = req;
        for (int i = 0; i < maxval; i++) {
            segptr->tab[i] = getrand() %100;
        }
        calculer_moyenne_locale(segptr, &moyenne_locale);

        acq_sem(semid, seg_init);

        wait_sem(semid, res_ok);

        long resultat_serveur = segptr->result;
        lib_sem(semid, seg_init);

        lib_sem(semid, seg_dispo);

        afficher_resultats(segptr, moyenne_locale);
    }

    if (shmdt(segptr) == -1) {
        perror("Erreur : impossible de détacher le segment mémoire");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}

void calculer_moyenne_locale(segment *segptr, double *moyenne_locale) {
    long somme = 0;
    for (int i = 0; i < maxval; i++) {
        somme += segptr->tab[i];
    }
    *moyenne_locale = (double)somme / maxval;
}

void afficher_resultats(segment *segptr, double moyenne_locale) {
    double moyenne_locale_arrondie = round(moyenne_locale);
    printf("PID : %d, Requête : %d\n", segptr->pid, segptr->req);
    printf("Moyenne locale (arrondie) : %.2f\n", moyenne_locale_arrondie);
    printf("Moyenne serveur : %.2f\n", (double)segptr->result);
    printf("-----------------------------\n");
}

