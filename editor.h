#ifndef EDITOR_H
#define EDITOR_H
#include "types.h"

// Fonction pour convertir un type de tuile en caractère
char tile_type_to_char(tile_type_e t);
// Lance le mode éditeur interactif SDL
int run_editor(char *filename_out, int *rows, int *cols, int *NB_GHOSTS,int TILE_SIZE);
// Sauvegarde une carte dans un fichier
void save_map(tile_type_e **map, int rows, int cols, const char *filename);

#endif // EDITOR_H
