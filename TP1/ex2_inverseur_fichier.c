
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h> 

int main() {

    int fichier = open("test.txt", O_RDWR);
    if (fichier == -1) {
        perror("Erreur : impossible d'ouvrir le fichier");
        return 1;
    }


    struct stat infos_fichier;
    fstat(fichier, &infos_fichier);
    int taille = infos_fichier.st_size;

 
    char *contenu = mmap(
        NULL,
        taille,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fichier,
        0
    );
    if (contenu == MAP_FAILED) {
        perror("Erreur : impossible de mapper le fichier");
        close(fichier);
        return 1;
    }


    for (int i = 0; i < taille / 2; i++) {
        char temporaire = contenu[i];
        contenu[i] = contenu[taille - 1 - i];
        contenu[taille - 1 - i] = temporaire;
    }


    munmap(contenu, taille);
    close(fichier);


    printf("Le fichier a été inversé ! Vérifiez avec : cat test.txt\n");
    return 0;
}
