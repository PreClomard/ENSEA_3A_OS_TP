# TP4 - Réalisation d'un système de fichiers simple 01/10/25
> Note pour le TP4, que cela soit compréhension du document ou des notes pour  
> mieux comprendre certaine fonction.

## Objectif
Utiliser les mécanisme IPC pour synchroniser un ensemble de programme producer/consumer.

## Principe
Client fournit stableau de val aléatoire dont la moyenne est calculé au serveur. Client plusieur requete au serv et entrer en concurence pour ressource.
Pour gérer les accès on utilises 3 sémaphores.  
> **Sémaphores:**  
> C'est une vairable partagé par différents "acteurs" et constitue la méthode utilisé pour restreindre l'accès à des ressources partagées dans un environnement de programmation concurrente.

Le client doit faire:
- sa propre moyenne du tableau
- l'envoie de son pide
- l'envoie du numér de requête
- la vérif des résultat


## Commande terminal
Dans le cas de l'utilisation d'un fichier exécutable télécharger, afin de pouvoir faire 
``` ./[Nom de l'exécutable] ```
Il faut faire 
```chmod +x ./[Nom de l'exécutable] ```