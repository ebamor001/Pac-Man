#include <SDL2/SDL.h>
#include "constants.h"
#include "window_utils.h"
void set_window_position_coords(SDL_Rect* window_rect)
{
    // Find where to launch the window (at the center of the screen where is the cursor)
    int mouse_x, mouse_y;
    SDL_GetGlobalMouseState(&mouse_x, &mouse_y);

    int nb_displays = SDL_GetNumVideoDisplays();
    SDL_Rect display_bounds;
    for (int i = 0; i < nb_displays; i++)
    {
        SDL_GetDisplayBounds(i, &display_bounds);
        if (mouse_x >= display_bounds.x && mouse_x < (display_bounds.x + display_bounds.w) && mouse_y >= display_bounds.y && mouse_y <(display_bounds.y + display_bounds.h))
        {
            break;
        }
    }

    window_rect->x = display_bounds.x + (display_bounds.w - window_rect->w*RENDER_SCALE) / 2;
    window_rect->y = display_bounds.y + (display_bounds.h - window_rect->h*RENDER_SCALE) / 2;
}

