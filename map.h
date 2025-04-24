#ifndef MAP_H
#define MAP_H

#include "types.h"

int get_map_dimensions(const char* filename, int* out_height, int* out_width, int* nb_ghosts);
int loadMapFromFile(const char* filename, tile_type_e** map, int** pacgommes, tile_type_e** secret_map, int N_X_TILES, int N_Y_TILES);
void generate_maze_from_fusion(char *filename_out,int * rows, int* cols, int* NB_GHOSTS);


#endif
