#ifndef MOVEMENT_H
#define MOVEMENT_H

#include <SDL2/SDL.h>
#include "types.h"

typedef enum {
    BFS,            // Méthode BFS
    MANHATTAN,      // Méthode Manhattan
    EUCLIDIENNE,    // Méthode Euclidienne
    RANDOM,        // Méthode aléatoire
    METHOD_UNKNOWN  // Méthode inconnue ou non spécifiée
} Method; 

Direction inverse_direction(Direction direction);
int can_move(tile_type_e** map, SDL_Rect pos, int tile_size, int shift, int entity_size, Direction dir, int N_X_TILES, int N_Y_TILES);
void move_entity(tile_type_e** map, SDL_Rect *position, Direction direction, int tile_size, int shift, int entity_size, SDL_Texture **texture, SDL_Texture *textures[], int N_X_TILES, int N_Y_TILES);
int collision_with_pacman(SDL_Rect ghost_position, SDL_Rect pacman_position);

// Fonctions de distance
int distance_euclidienne(SDL_Rect ghost, SDL_Rect pacman, Direction dir_ghost);
int distance_manhattan(SDL_Rect ghost, SDL_Rect pacman, Direction dir_ghost);

// Fonctions pour choisir la direction optimale
Direction find_direction_aleatoire(Direction* possible_directions, int nb_choices);
Direction find_optimal_direction_euclidienne(SDL_Rect ghost, SDL_Rect pacman, Direction* possible_directions, int nb_choices);
Direction find_optimal_direction_manhattan(SDL_Rect ghost, SDL_Rect pacman, Direction* possible_directions, int nb_choices);
Direction find_optimal_direction_BFS(tile_type_e** map, SDL_Rect position1, SDL_Rect position2, int tile_size, int shift,  int entity_size,int N_X_TILES, int N_Y_TILES);

//// Fonction pour choisir une direction optimale selon la méthode choisie
Direction find_direction_method(Method method, tile_type_e** map,  SDL_Rect  initial_position,  SDL_Rect final_position,int tile_size, int shift, int ghost_size, int n_x_tiles, int n_y_tiles, Direction*tab, int nb_choices);
#endif
