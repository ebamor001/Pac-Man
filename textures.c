#include "textures.h"
#include <stdio.h>
#include <SDL2/SDL_image.h>

SDL_Texture* load_texture(SDL_Renderer *renderer, const char *path) {
    SDL_Texture *texture = IMG_LoadTexture(renderer, path);
    if (!texture) {
        fprintf(stderr, "Could not load texture: %s\n", IMG_GetError());
        return NULL;
    }
    return texture;
}
void load_pacman_textures(SDL_Renderer *renderer, SDL_Texture **pacman_textures) {
    const char *pacman_files[4] = {
        "images/pacman-up.png",    // Up
        "images/pacman-down.png",  // Down
        "images/pacman-left.png",  // Left
        "images/pacman-right.png"  // Right
    };
    for (int i = 0; i < 4; i++) {
        pacman_textures[i] = load_texture(renderer, pacman_files[i]);
    }
}
void load_ghost_dead_textures(SDL_Renderer *renderer, SDL_Texture **ghost_dead_textures) {
    const char *ghost_dead_files[4] = {
        "images/ghost-eyes-up.png",
        "images/ghost-eyes-down.png",
        "images/ghost-eyes-left.png",
        "images/ghost-eyes-right.png"
    };
    for (int i = 0; i < 4; i++) {
        ghost_dead_textures[i] = load_texture(renderer, ghost_dead_files[i]);
    }
}

void load_ghost_textures(SDL_Renderer *renderer, SDL_Texture ***ghost_textures,int NB_GHOSTS) {
    const char *ghost_colors[4] = { "red", "blue", "pink", "orange" };
    const char *ghost_directions[4] = { "up", "down", "left", "right" };

    for (int i = 0; i < NB_GHOSTS; i++) {
        for (int j = 0; j < 4; j++) {
            char path[100];
            sprintf(path, "images/ghost-%s-%s.png", ghost_colors[i%4], ghost_directions[j]);
            ghost_textures[i][j] = load_texture(renderer, path);
        }
    }
}


