# Pac-Man
BEN AMOR EYA 

projet de programmation ayant pour objectif de recréer le jeu Pac-Man en utilisant la bibliothèque SDL


UTILISATION :
./pacman <fichier_labyrinthe>: charge un fichier de carte (au format texte) et lance le jeu avec l'algorithme de recherche BFS par défaut.   
./pacman <fichier_labyrinthe> manhattan: lance le jeu avec l’algorithme A* utilisant la distance de Manhattan comme heuristique.
./pacman <fichier_labyrinthe> euclidienne: lance le jeu avec A* utilisant la distance euclidienne.
./pacman <fichier_labyrinthe> aleatoire: utilise un chemin aléatoire pour le déplacement des fantômes.
./pacman aleatoire: génère automatiquement un labyrinthe aléatoire en mémoire, grâce à l’algorithme de fusion de cellules et le sauvegarde dans un fichier, puis lance immédiatement le jeu avec cette carte.
./pacman editor: lance un éditeur de labyrinthe visuel permettant de créer manuellement des cartes.

