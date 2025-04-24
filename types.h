#ifndef TYPES_H
#define TYPES_H

typedef enum tile_type_e {
    WALL = 0,
    PATH,
    PACMAN_START,
    GHOST_START = 'G',
    SECRET_WALL = 'M'
} tile_type_e; 


typedef enum  {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT,
    DIR_NONE // Ajout d'une valeur par défaut pour éviter les erreurs et pour l'utiliser dans des boucles
} Direction;

#endif
