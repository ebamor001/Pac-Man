#include "movement.h"
#include <SDL2/SDL.h>
#include "types.h"
#define MAX_X 100
#define MAX_Y 100

static int visited[MAX_Y][MAX_X];
static Direction parent_direction[MAX_Y][MAX_X]; //pour utiliser dans find_optimal_direction : c mieux que des allocations dynamiques et c'est plus rapide

Direction inverse_direction(Direction direction) {
    switch (direction) {
        case DIR_UP:
            return DIR_DOWN;
        case DIR_DOWN:
            return DIR_UP;
        case DIR_LEFT:
            return DIR_RIGHT;
        case DIR_RIGHT:
            return DIR_LEFT;
        default:
            return direction; 
    }
}

int distance_euclidienne(SDL_Rect ghost, SDL_Rect pacman, Direction dir_ghost) {
    SDL_Rect cp_ghost = ghost;

    switch (dir_ghost)
    {
        case DIR_UP:    cp_ghost.y--; break;
        case DIR_DOWN:  cp_ghost.y++; break;
        case DIR_LEFT:  cp_ghost.x--; break;
        case DIR_RIGHT: cp_ghost.x++; break;
        default: break;
    }

    return (int)sqrt(pow(cp_ghost.x - pacman.x, 2) + pow(cp_ghost.y - pacman.y, 2));
}

int distance_manhattan(SDL_Rect ghost, SDL_Rect pacman, Direction dir_ghost) {
    SDL_Rect cp_ghost = ghost;

    switch (dir_ghost) {
        case DIR_UP:    cp_ghost.y--; break;
        case DIR_DOWN:  cp_ghost.y++; break;
        case DIR_LEFT:  cp_ghost.x--; break;
        case DIR_RIGHT: cp_ghost.x++; break;
        default: break;
    }

    return abs(cp_ghost.x - pacman.x) + abs(cp_ghost.y - pacman.y);
}


int can_move(enum tile_type_e** map, SDL_Rect pos, int tile_size, int shift, int entity_size, Direction dir, int N_X_TILES, int N_Y_TILES)
{
    int tile_x = pos.x / tile_size;
    int tile_y = pos.y / tile_size;

    switch (dir)
    {
        case DIR_UP:

            //condition pour la téléportation en haut
            if (pos.y <= 0) {
                return 1;
            }
            
            if ((map[(pos.y - shift - 1) / tile_size][tile_x] == PATH || map[(pos.y - shift - 1) / tile_size][tile_x] == SECRET_WALL) &&
                ((pos.x - shift) % tile_size == 0))
                return 1;
            break;

        case DIR_DOWN:
            //condition pour la téléportation en bas
            if (pos.y >= (N_Y_TILES - 1) * tile_size) {
                return 1;
            }
            if ((map[(pos.y + entity_size + shift) / tile_size][tile_x] == PATH || map[(pos.y + entity_size + shift) / tile_size][tile_x] == SECRET_WALL ) &&
                ((pos.x - shift) % tile_size == 0))
                return 1;
            break;

        case DIR_LEFT:
            //condition pour la téléportation à gauche
            if (pos.x <= 0) {
                return 1;
            }
            if ((map[tile_y][(pos.x - shift - 1) / tile_size] == PATH || map[tile_y][(pos.x - shift - 1) / tile_size] == SECRET_WALL )  &&
                ((pos.y - shift) % tile_size == 0))
                return 1;
            break;

        case DIR_RIGHT:
            //condition pour la téléportation à droite
            if (pos.x >= (N_X_TILES - 1) * tile_size) {
                return 1;
            }
            if ((map[tile_y][(pos.x + entity_size + shift) / tile_size] == PATH || map[tile_y][(pos.x + entity_size + shift) / tile_size] == SECRET_WALL ) &&
                ((pos.y - shift) % tile_size == 0))
                return 1;
            break;
        case DIR_NONE:
        // Rien à faire, Pac-Man ou le fantôme ne bouge pas. on l'ajoute juste pour ne pas avoir d'erreur
        break;
    }
    return 0;
}
// Version temporaire simplifiée pour tester

void move_entity(enum tile_type_e** map, SDL_Rect *position, Direction direction,int tile_size, int shift, int entity_size, SDL_Texture **texture, SDL_Texture *textures[], int N_X_TILES, int N_Y_TILES)
{
    if (can_move(map, *position, tile_size, shift, entity_size, direction,N_X_TILES, N_Y_TILES))
    {
        switch (direction)
        {
            case DIR_UP:
                *texture = textures[0];
                position->y--;
                break;
            case DIR_DOWN:
                *texture = textures[1];
                position->y++;
                break;
            case DIR_LEFT:
                *texture = textures[2];
                position->x--;
                break;
            case DIR_RIGHT:
                *texture = textures[3];
                position->x++;
                break;
            case DIR_NONE:  
                break;

        }
        // Téléportation horizontale (gauche/droite)
        if (position->x < 0 ) {
            if (map[position->y / tile_size][N_X_TILES - 1] == PATH) {
                position->x = (N_X_TILES - 1) * tile_size;
            } else {
                position->x = 0; // Bloqué à gauche
            }
        } 
        else if (position->x > (N_X_TILES-1) * tile_size) {
            if (map[position->y / tile_size][0] == PATH) {
                position->x = 0;
            } else {
                position->x = (N_X_TILES - 1) * tile_size; // Bloqué à droite
            }
        }
    

        // Téléportation verticale (haut/bas)
        if (position->y < 0) {
            if (map[N_Y_TILES - 1][position->x / tile_size] == PATH) {
                position->y = (N_Y_TILES-1) * tile_size;
            } else {
                position->y = 0; // Bloqué en haut
            }
        } 
        else if (position->y > (N_Y_TILES-1) * tile_size) {
            if (map[0][position->x / tile_size] == PATH) {
                position->y = 0;
            } else {
                position->y = (N_Y_TILES-1) * tile_size; // Bloqué en bas
            }
        }
    }
}
int collision_with_pacman(SDL_Rect ghost_position, SDL_Rect pacman_position)
{
    return SDL_HasIntersection(&ghost_position, &pacman_position);
}


// Fonction pour choisir une direction aléatoire parmi les directions possibles
Direction find_direction_aleatoire(Direction* possible_directions, int nb_choices){
    if (nb_choices == 0) {
        return DIR_NONE; // Aucune direction possible
    }
    int random_index = rand() % nb_choices;
    return possible_directions[random_index];
}

// Fonction pour calculer la distance entre deux positions en utilisant la distance euclidienne
Direction find_optimal_direction_euclidienne(SDL_Rect ghost, SDL_Rect pacman, Direction* possible_directions, int nb_choices) {
    int min_dist = 100000;  // Distance initiale très grande
    int best_index = -1;

    for (int i = 0; i < nb_choices; i++) {
        int dist = distance_euclidienne(ghost, pacman, possible_directions[i]);

        if (dist < min_dist) {
            min_dist = dist;
            best_index = i;
        }
    }

    if (best_index != -1) {
        return possible_directions[best_index];
    }

    return DIR_NONE;
}


// Fonction pour calculer la distance entre deux positions en utilisant la distance de Manhattan
Direction find_optimal_direction_manhattan(SDL_Rect ghost, SDL_Rect pacman, Direction* possible_directions, int nb_choices) {
    int min_dist = 100000;
    int best_index = -1;

    for (int i = 0; i < nb_choices; i++) {
        int dist = distance_manhattan(ghost, pacman, possible_directions[i]);

        if (dist < min_dist) {
            min_dist = dist;
            best_index = i;
        }
    }

    if (best_index != -1) {
        return possible_directions[best_index];
    }

    return DIR_NONE;
}



Direction find_optimal_direction_BFS(tile_type_e** map, SDL_Rect position1, SDL_Rect position2, int tile_size, int shift, int entity_size, int N_X_TILES, int N_Y_TILES) {
    int dx[] = {0, 0, -1, 1};  // Pour DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT
    int dy[] = {-1, 1, 0, 0};

    if (N_X_TILES > MAX_X || N_Y_TILES > MAX_Y) {
        printf("Erreur : dimensions de la carte trop grandes\n");
        return DIR_NONE;
    }

    // Réinitialiser les tableaux
    for (int i = 0; i < N_Y_TILES; i++) {
        for (int j = 0; j < N_X_TILES; j++) {
            visited[i][j] = 0;
            parent_direction[i][j] = DIR_NONE;
        }
    }

    int pile[MAX_X * MAX_Y][2];
    int front = 0, rear = 0;

    // Convertir les positions en indices de tuiles
    int start_tile_x = position1.x / tile_size ;
    int start_tile_y = position1.y/ tile_size ;
    int end_tile_x = position2.x / tile_size ;
    int end_tile_y = position2.y / tile_size ;

    // Vérifier si nous sommes déjà à destination
    if (start_tile_x == end_tile_x && start_tile_y == end_tile_y) {
        printf("Le fantôme est arrivé à sa case d'origine !\n");
        return DIR_NONE;
    }

    // Initialisation de la pile
    pile[rear][0] = start_tile_x;
    pile[rear][1] = start_tile_y;
    rear++;
    visited[start_tile_y][start_tile_x] = 1;

    while (front < rear) {
        int current_tile_x = pile[front][0];
        int current_tile_y = pile[front][1];
        front++;
    
        if (current_tile_x == end_tile_x && current_tile_y == end_tile_y) {
            return parent_direction[current_tile_y][current_tile_x];
        }
       
        for (Direction dir = 0; dir < 4; dir++) {
            int next_tile_x = current_tile_x + dx[dir];
            int next_tile_y = current_tile_y + dy[dir];
            // Vérifier les limites de la carte
            if (next_tile_x < 0 || next_tile_x >= N_X_TILES || next_tile_y < 0 || next_tile_y >= N_Y_TILES) {
                continue;
            }
            if (!visited[next_tile_y][next_tile_x]) {
    
                if (map[next_tile_y][next_tile_x] == PATH || map[next_tile_y][next_tile_x] == SECRET_WALL) {
                    visited[next_tile_y][next_tile_x] = 1;
    
                    if (current_tile_x == start_tile_x && current_tile_y == start_tile_y) {
                        parent_direction[next_tile_y][next_tile_x] = dir;
                    } else {
                        parent_direction[next_tile_y][next_tile_x] = parent_direction[current_tile_y][current_tile_x];
                    }
    
                    pile[rear][0] = next_tile_x;
                    pile[rear][1] = next_tile_y;
                    rear++;
    
                    if (rear >= MAX_X * MAX_Y) {
                        printf("Pile overflow !\n");
                        return DIR_NONE;
                    }
                }
            }
        }
        
    }
    
    printf("Aucun chemin trouvé vers la destination.\n");
    return DIR_NONE;
}

// Déclaration de la fonction pour trouver la direction optimale selon la méthode choisie
Direction find_direction_method(Method method, tile_type_e** map, SDL_Rect  initial_position,  SDL_Rect final_position,
    int tile_size, int shift, int ghost_size, int n_x_tiles, int n_y_tiles, 
    Direction*tab, int nb_choices) {
Direction optimal_direction;

    // Choisir la fonction de calcul en fonction de la méthode
    switch (method) {
        case BFS:
            // Appel de la fonction BFS pour trouver la direction optimale
            optimal_direction= find_optimal_direction_BFS( map, initial_position,final_position, tile_size,  shift, ghost_size, n_x_tiles, n_y_tiles);
            break;

        case MANHATTAN:
            // Appel de la fonction Manhattan pour trouver la direction optimale
            optimal_direction = find_optimal_direction_manhattan(initial_position,final_position, tab, nb_choices);
            break;

        case EUCLIDIENNE:
            // Appel de la fonction Euclidienne pour trouver la direction optimale
            optimal_direction = find_optimal_direction_euclidienne(initial_position,final_position, tab, nb_choices);
            break;

        case RANDOM:
            // Appel de la fonction Random pour trouver la direction optimale
            optimal_direction = find_direction_aleatoire(tab, nb_choices);
            break;

        default:
            // Si la méthode est inconnue, retourner la direction par défaut: BFS
            optimal_direction =find_optimal_direction_BFS( map, initial_position,final_position, tile_size,  shift, ghost_size, n_x_tiles, n_y_tiles);
            break;
    }

    return optimal_direction;
    }

