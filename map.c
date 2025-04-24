#include <stdio.h>
#include <string.h>   // pour strcpy
#include <stdlib.h>   // pour malloc, rand, srand
#include <time.h>     // pour time

#include "types.h"
#include "map.h"
#include "editor.h"
int get_map_dimensions(const char* filename, int* out_height, int* out_width,int* nb_ghosts) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Erreur : Impossible d'ouvrir %s\n", filename);
        return 0;
    }

    int width = 0;
    int max_width = 0;
    int height = 0;
    int ghosts=0;
    int espaces =0;
    char c;
    while ((c = fgetc(file)) != EOF) {
        if (c == '\n') {
            height++;
            if (width > max_width) {
                max_width = width;
            }
            width = 0;
            espaces =0;
        }else if (c == ' '){
            espaces++;
        }
        else{
            width += espaces; 
            width++;
            espaces=0;
        }
        if (c == 'G') {
            ghosts++;
        }
    }

    // Si le fichier ne se termine pas par un \n
    if (width > 0) {
        height++;
        if (width > max_width) {
            max_width = width;
        }
    }

    fclose(file);

    *out_height = height;
    *out_width = max_width;
    *nb_ghosts= ghosts;
    return 1;
}

// Fonction pour charger un labyrinthe depuis un fichier
int loadMapFromFile(const char* filename, enum tile_type_e** map, int** pacgommes,enum tile_type_e** secret_map , int N_X_TILES, int N_Y_TILES) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Erreur : Impossible d'ouvrir %s\n", filename);
        return 0;
    }

    int x = 0, y = 0;
    
    char c;

    while (y < N_Y_TILES && (c = fgetc(file)) != EOF) {
        if (c == '\n') {
            // remplir le reste de la ligne avec des espaces si c vide
            while (x < N_X_TILES) {
                map[y][x] = PATH;
                pacgommes[y][x] = 1;
                x++;
            }
            y++;
            x = 0;
        } else if (x < N_X_TILES) {
            switch (c) {
                case 'W': map[y][x] = WALL; pacgommes[y][x] = 0;secret_map[y][x]=WALL; break;
                case 'M': map[y][x] = WALL; pacgommes[y][x] = 0;secret_map[y][x]=SECRET_WALL; break;
                case ' ': map[y][x] = PATH; pacgommes[y][x] = 1;secret_map[y][x]=PATH; break;
                case 'S': map[y][x] = PACMAN_START; pacgommes[y][x] = 0;secret_map[y][x]=PATH; break;
                case 'G': map[y][x] = GHOST_START; pacgommes[y][x] = 1;secret_map[y][x]=PATH; break;
                
                default:
                    printf("Erreur : Caractère invalide '%c' à (%d, %d)\n", c, x, y);
                    fclose(file);
                    return 0;
            }
            x++;
        }
    }

    fclose(file);
    return 1;
}


void generate_maze_from_fusion(char *filename_out,int * rows, int* cols, int* nb_ghosts){
    int N_X_TILES, N_Y_TILES ,NB_GHOSTS; 
    char filename[100];

    printf("Nom du fichier pour sauvegarder (ex: labyrinthe.txt) : ");
    scanf("%99s", filename);
    printf("Nombre de colonnes (X) : ");
    scanf("%d", &N_X_TILES);
    printf("Nombre de lignes (Y) : ");
    scanf("%d", &N_Y_TILES);
    printf("Nombre de fantomes (G) : ");
    scanf("%d", &NB_GHOSTS);

    if (N_X_TILES <= 0 || N_Y_TILES <= 0) {
        printf("Dimensions invalides.\n");
    }
    // Copier les valeurs dans les variables passées par pointeur
    strcpy(filename_out, filename);
    *rows = N_Y_TILES;
    *cols = N_X_TILES;
    *nb_ghosts = NB_GHOSTS;
    tile_type_e **map = malloc(N_Y_TILES * sizeof(tile_type_e *));
    if (map == NULL) {
        perror("Erreur d'allocation mémoire pour la carte");
        return;
    }
    for (int i = 0; i < N_Y_TILES; i++) {
        map[i] = malloc(N_X_TILES * sizeof(tile_type_e));
        if (map[i] == NULL) {
            perror("Erreur d'allocation mémoire pour une ligne de la carte");
            return;
        }
    }

    int y, x;
    
    for (y = 0; y < N_Y_TILES; y++) {
        for (x = 0; x < N_X_TILES; x++) {
            if ( (y % 2 == 1) && (x % 2 == 1) ) {
                map[y][x] = PATH;
            } else {
                map[y][x] = WALL;
            }
        }
    }
    
    int **id = malloc(N_Y_TILES * sizeof(int *));
    for (y = 0; y < N_Y_TILES; y++) {
        id[y] = malloc(N_X_TILES * sizeof(int));
        for (x = 0; x < N_X_TILES; x++) {
            // Pour les cellules accessibles, on assigne un id unique
            if ((y % 2 == 1) && (x % 2 == 1)){
            id[y][x] = y * N_X_TILES + x;
            }else{
            id[y][x] = 0;  // pas utilisé pour les murs
            }
        }
    }
    int accessibleCount = ((N_Y_TILES)/2) * ((N_X_TILES)/2);
    int targetConnections = accessibleCount - 1;
    int connections = 0;
    
    srand(time(NULL));
    
    
    while (connections < targetConnections) {
        // Choisir une cellule accessible au hasard (indices impairs)
        int r_y = ((rand() % (N_Y_TILES/2)) * 2) + 1;
        int r_x = ((rand() % (N_X_TILES/2)) * 2) + 1;
        
        // Choisir une direction aléatoire parmi les 4 (0: haut, 1: droite, 2: bas, 3: gauche)
        int dir = rand() % 4;
        int nb_y = r_y, nb_x = r_x;
        if (dir == 0) nb_y = r_y - 2;        // haut
        else if (dir == 1) nb_x = r_x + 2;     // droite
        else if (dir == 2) nb_y = r_y + 2;     // bas
        else if (dir == 3) nb_x = r_x - 2;     // gauche
        
        // Vérifier que la cellule voisine est dans les bornes
        if (nb_y < 1 || nb_y >= N_Y_TILES || nb_x < 1 || nb_x >= N_X_TILES)
        continue;
        
        // Si les deux cellules appartiennent déjà au même ensemble, rien à faire
        if (id[r_y][r_x] == id[nb_y][nb_x])
        continue;
        
        // Sinon, on retire le mur entre les deux cellules :
        int wall_y = (r_y + nb_y) / 2;
        int wall_x = (r_x + nb_x) / 2;
        map[wall_y][wall_x] = PATH;
        
        // Fusionner les ensembles en mettant à jour tous les id correspondants.
        int oldId = id[nb_y][nb_x];
        int newId = id[r_y][r_x];
        for (y = 1; y < N_Y_TILES; y += 2) {
            for (x = 1; x < N_X_TILES; x += 2) {
                if (id[y][x] == oldId) 
                id[y][x] = newId;
            }
        }
        connections++;
    }
    
    // Placement du point de départ de Pacman et des fantômes
    int pacman_placed = 0;
    while (!pacman_placed) {
    int y = (rand() % (N_Y_TILES / 2)) * 2 + 1; // position impaire
    int x = (rand() % (N_X_TILES / 2)) * 2 + 1;
    if (map[y][x] == PATH) {
        map[y][x] = PACMAN_START;
        pacman_placed = 1;
      }
    }
    
    int ghosts_placed = 0;
    while (ghosts_placed < NB_GHOSTS) {
        int y = (rand() % (N_Y_TILES / 2)) * 2 + 1; // position impaire
        int x = (rand() % (N_X_TILES / 2)) * 2 + 1;

        if (map[y][x] == PATH && !(y == 1 && x == 1)) {
            map[y][x] = GHOST_START;
            ghosts_placed++;
        }
    }

    save_map(map, *rows, *cols, filename_out);
    
    // Libérer le tableau de fusion
    for (y = 0; y < N_Y_TILES; y++) {
        free(id[y]);
    }
    free(id);
}






   
    




