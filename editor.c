#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "editor.h"

const char *tileNames[] = {"Mur", "Chemin", "Pac-Man", "Fantome"," Mur secret"};
char tile_type_to_char(tile_type_e t) {
    switch (t) {
        case WALL:         return 'W';
        case PATH:         return ' ';
        case PACMAN_START: return 'S';
        case GHOST_START:  return 'G';
        case SECRET_WALL:  return 'M';
        default:           return '?';  // Caractère pour valeur inconnue
    }
}

// Fonction pour sauvegarder la carte dans un fichier
void save_map(tile_type_e **map, int rows, int cols, const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Erreur d'ouverture fichier");
        return;
    }
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            tile_type_e val = map[y][x];
            char c = tile_type_to_char(val);
            fputc(c, f);
        }
        fputc('\n', f);
    }
    fclose(f);
    printf("Carte sauvegardee dans %s\n", filename);
}

int run_editor(char *filename_out, int *rows, int *cols , int *NB_GHOSTS, int TILE_SIZE) {
    char filename[100];
    int N_X_TILES, N_Y_TILES;
    int nb_ghosts=0;

    printf("Nom du fichier pour sauvegarder (ex: labyrinthe.txt) : ");
    scanf("%99s", filename);
    printf("Nombre de colonnes (X) : ");
    scanf("%d", &N_X_TILES);
    printf("Nombre de lignes (Y) : ");
    scanf("%d", &N_Y_TILES);

    if (N_X_TILES <= 0 || N_Y_TILES <= 0) {
        printf("Dimensions invalides.\n");
        return 1;
    }
    // Copier les valeurs dans les variables passées par pointeur
    strcpy(filename_out, filename);
    *rows = N_Y_TILES;
    *cols = N_X_TILES;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *win = SDL_CreateWindow("Editeur de labyrinthe",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        TILE_SIZE * N_X_TILES, TILE_SIZE * N_Y_TILES, 0);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, 0);

    tile_type_e **map = malloc(N_Y_TILES * sizeof(tile_type_e *));
    for (int y = 0; y < N_Y_TILES; y++) {
        map[y] = malloc(N_X_TILES * sizeof(tile_type_e));
        for (int x = 0; x < N_X_TILES; x++) {
            map[y][x] = PATH;
        }
    }

    tile_type_e current_tile = WALL;
    printf("Tuile actuelle : %s\n", tileNames[current_tile]);

    int running = 1;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
            else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_0: current_tile = PATH; break;
                    case SDLK_1: current_tile = WALL; break;
                    case SDLK_2: current_tile = PACMAN_START; break;
                    case SDLK_3: current_tile = GHOST_START; nb_ghosts++; break;
                    case SDLK_4: current_tile=SECRET_WALL ;break;
                    case SDLK_s: save_map(map, N_Y_TILES, N_X_TILES, filename); break;
                }
                printf("Tuile actuelle : %s\n", tileNames[current_tile]);
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int x = e.button.x / TILE_SIZE;
                int y = e.button.y / TILE_SIZE;
                if (x < N_X_TILES && y < N_Y_TILES) {
                    map[y][x] = current_tile;
                }
            }
        }

        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);

        for (int y = 0; y < N_Y_TILES; y++) {
            for (int x = 0; x < N_X_TILES; x++) {
                switch (map[y][x]) {
                    case WALL: SDL_SetRenderDrawColor(ren, 0, 0, 255, 255); break;
                    case PATH: SDL_SetRenderDrawColor(ren, 0, 0, 0, 255); break;
                    case PACMAN_START: SDL_SetRenderDrawColor(ren, 255, 255, 0, 255); break;
                    case GHOST_START: SDL_SetRenderDrawColor(ren, 255, 0, 0, 255); break;
                    case SECRET_WALL: SDL_SetRenderDrawColor(ren, 100, 0, 100, 255); break;
                }
                SDL_Rect rect = { x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE };
                SDL_RenderFillRect(ren, &rect);
                SDL_SetRenderDrawColor(ren, 80, 80, 80, 255);
                SDL_RenderDrawRect(ren, &rect);
            }
        }

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }
    *NB_GHOSTS=nb_ghosts;
    for (int y = 0; y < N_Y_TILES; y++) {
        free(map[y]);
    }
    free(map);

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
