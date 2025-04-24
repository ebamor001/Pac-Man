#ifndef TEXTURES_H
#define TEXTURES_H

#include <SDL2/SDL.h>

SDL_Texture* load_texture(SDL_Renderer *renderer, const char *path);
void load_pacman_textures(SDL_Renderer *renderer, SDL_Texture **pacman_textures);
void load_ghost_textures(SDL_Renderer *renderer, SDL_Texture ***ghost_textures, int NB_GHOSTS);
void load_ghost_dead_textures(SDL_Renderer *renderer, SDL_Texture **ghost_dead_textures);

#endif
