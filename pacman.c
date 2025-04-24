#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>
#include "constants.h"
#include "types.h"
#include "textures.h"
#include "movement.h"
#include "map.h"
#include "window_utils.h"
#include "editor.h" 


int main(int argc, char* argv[]) {
    Method method=4; // 0: BFS, 1: Manhattan, 2: Euclidienne, 3: Random; 4: Unknown
    int N_X_TILES = 0, N_Y_TILES = 0, NB_GHOSTS = 0;
    int tile_size =PACMAN_SIZE + 8;
    int shift = (tile_size - PACMAN_SIZE)/2;
    char filename[100];

    if (argc < 2) {
        printf("Utilisation : %s <fichier_labyrinthe>/<aleatoire> ou %s editor\n\n", argv[0],argv[0]);
         return 1;
    }else if (argc == 2) {
        method=0; //par défaut BFS
        if (strcmp(argv[1], "aleatoire") == 0)
        {
            // On génère un labyrinthe aléatoire
            generate_maze_from_fusion(filename,  &N_Y_TILES, &N_X_TILES, &NB_GHOSTS);
            
            
        }
        else if  (strcmp(argv[1], "editor") == 0) {
            // On lance l'éditeur de labyrinthe
            run_editor(filename, &N_Y_TILES, &N_X_TILES, &NB_GHOSTS, tile_size);
        }else{
            // On charge comme avant depuis un fichier .txt
            if (!get_map_dimensions(argv[1], &N_Y_TILES, &N_X_TILES,&NB_GHOSTS)) {
                return 1; // erreur ouverture fichier
            }
            else{
                strcpy(filename, argv[1]);
            }            
        }
    } else if (argc == 3) {
        if (!get_map_dimensions(argv[1], &N_Y_TILES, &N_X_TILES,&NB_GHOSTS)) {
            return 1; // erreur ouverture fichier
        }
        else{
            strcpy(filename, argv[1]);
            // Si une méthode est spécifiée, on la choisit
            if (strcmp(argv[2], "manhattan") == 0) {
                method=1;
            } else if (strcmp(argv[2], "euclidienne") == 0) {
                method=2;
            } else if (strcmp(argv[2], "aleatoire") == 0) {
                method=3;
            } else {
                // Si la méthode n'est pas reconnue, on utilise BFS
                printf("Méthode non reconnue. Utilisation de BFS par défaut.\n");
                method=0;
            }
        }
    } else {
        printf("Nombre d'arguments incorrect.\n");
        return 1;
    }


    // Allocation dynamique
    enum tile_type_e** map = malloc(N_Y_TILES * sizeof(enum tile_type_e*));
    enum tile_type_e** secret_map = malloc(N_Y_TILES * sizeof(enum tile_type_e*));
    int** pacgommes = malloc(N_Y_TILES * sizeof(int*));
    int** path_detected = malloc(N_Y_TILES * sizeof(int*));

    for (int i = 0; i < N_Y_TILES; i++) {
        map[i] = malloc(N_X_TILES * sizeof(enum tile_type_e));
        secret_map[i] = malloc(N_X_TILES * sizeof(enum tile_type_e));
        pacgommes[i] = malloc(N_X_TILES * sizeof(int));
        path_detected[i] = calloc(N_X_TILES, sizeof(int));  // initialise à 0 direct
    }
    if (!loadMapFromFile(filename, map, pacgommes, secret_map, N_X_TILES, N_Y_TILES))
    {
        return 1;
    }
    

    int ret;
    int is_running = 1;
    
    
    SDL_Rect window_rect = {
        .w = N_X_TILES*tile_size,
        .h = N_Y_TILES*tile_size,
        .x = SDL_WINDOWPOS_UNDEFINED, //the code is instructing SDL to let the operating system decide the initial position of the window
        .y = SDL_WINDOWPOS_UNDEFINED,
    };

    // SDL initialisation with video support
    ret = SDL_Init(SDL_INIT_VIDEO);
    if (ret != 0)
    {
        fprintf(stderr, "Could not init SDL: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    // Create the window
    set_window_position_coords(&window_rect);
    SDL_Window *screen = SDL_CreateWindow(
        "Pacman",
        window_rect.x, window_rect.y,
        window_rect.w*RENDER_SCALE, window_rect.h*RENDER_SCALE,
        0
    );
    if (!screen)
    {
        fprintf(stderr, "Could not create SDL screen: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    // Create the renderer, can be seen as a paint brush
    SDL_Renderer *renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer)
    {
        fprintf(stderr, "Could not create SDL renderer: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    // Will apply a scale of RENDER_SCALE to all coordinates and dimensions handled by renderer
    ret = SDL_RenderSetScale(renderer, RENDER_SCALE, RENDER_SCALE);
    if (ret < 0)
    {
        fprintf(stderr, "Could not scale SDL renderer: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Rect pacman_position = {
        .w = PACMAN_SIZE,
        .h = PACMAN_SIZE,
    };
    SDL_Rect ghosts_positions[NB_GHOSTS];
    for (int i = 0; i < NB_GHOSTS; i++)
    {
        ghosts_positions[i].w = GHOST_SIZE;
        ghosts_positions[i].h = GHOST_SIZE;
    }
    //enregistrer la position initiale de pacman et des ghosts pour le restart
    SDL_Rect initial_pacman_position = {
        .w = PACMAN_SIZE,
        .h = PACMAN_SIZE,
    };

    SDL_Rect initial_ghosts_positions[NB_GHOSTS];
    for (int i = 0; i < NB_GHOSTS; i++)
    {
        ghosts_positions[i].w = GHOST_SIZE;
        ghosts_positions[i].h = GHOST_SIZE;
    }

    SDL_Rect map_position = {
        .x = 0,
        .y = 0,
        .w = window_rect.w,
        .h = window_rect.h,
    };
    SDL_Rect echec_rect = {
        .w = map_position.w / 3,
        .h = map_position.h / 2,
        .x = (map_position.w - map_position.w / 3) / 2,
        .y = (map_position.h - map_position.h / 2) / 2
    };

    SDL_Rect victoire_rect = {
        .w = map_position.w /3,
        .h = map_position.h /2,
        .x = (map_position.w - map_position.w /3) / 2,
        .y = (map_position.h - map_position.h /2) / 2
    };

    // Create a texture for the map, can be seen as a layer
    SDL_Texture* map_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        window_rect.w,
        window_rect.h
    );

    // Now, the paint brush `renderer` will paint on the layer `map_texture`
    SDL_SetRenderTarget(renderer, map_texture);

    // Select color of the paint brush (R, G, B, alpha)
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    // Paint the the whole target of the paint brush (`map_texture` now)
    SDL_RenderClear(renderer);

    // Now, let's paint the maze walls in blue:
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);


    int actual_ghost_number=0;
    for (int y = 0; y < N_Y_TILES; y++)
    {
        for (int x = 0; x < N_X_TILES; x++)
        {
            if (map[y][x] == WALL)
            {
                SDL_Rect rectangle = {
                    .x = x*tile_size,
                    .y = y*tile_size,
                    .w = tile_size,
                    .h = tile_size,
                };
                SDL_RenderFillRect(renderer, &rectangle);
            }
            else if (map[y][x] == PACMAN_START)
            {
                // While we are at it, save what is the initial position of Pac-Man
                pacman_position.x = x*tile_size + (tile_size-PACMAN_SIZE)/2;
                pacman_position.y = y*tile_size + (tile_size-PACMAN_SIZE)/2;

                /* Don't forget to change the map tile type to consider it as a
                 * path (can be done probably in a better way, for instance
                 * with a bit fields) */
                map[y][x] = PATH;
            }
            else if (map[y][x] == GHOST_START)
            {
                if (actual_ghost_number >= NB_GHOSTS)  
                {
                    fprintf(stderr, " Too many ghosts in the map!\n");
                    return EXIT_FAILURE;
                }    
                // While we are at it, save what is the initial position of THE GHOSTS
                ghosts_positions[actual_ghost_number].x = x * tile_size + (tile_size - GHOST_SIZE) / 2;
                ghosts_positions[actual_ghost_number].y = y * tile_size + (tile_size - GHOST_SIZE) / 2;
                map[y][x] = PATH;
                actual_ghost_number++;   // Select color of the paint brush (R, G, B, alpha)
            }
        }
    }
    //vérifier que les ghosts déclarés sont bien NB_GHOSTS
    if (actual_ghost_number!=NB_GHOSTS)
    {
        fprintf(stderr, "The number of ghosts is not correct\n");
        return EXIT_FAILURE;
    }
    //enregistrer la position initiale de pacman et des ghosts pour le restart
    initial_pacman_position = pacman_position;
    for (int i = 0; i < NB_GHOSTS; i++)
    {
        initial_ghosts_positions[i] = ghosts_positions[i];
    }
    // Draw back to window's renderer (ie the paint brush draws on the window now):
    SDL_SetRenderTarget(renderer, NULL);

    
    

    // Charger toutes les textures du pacman et fantomes dans les differentes directions et couleurs
    SDL_Texture* pacman_textures[4];
    SDL_Texture ***ghosts_textures = malloc(NB_GHOSTS * sizeof(SDL_Texture**));
    for (int i = 0; i < NB_GHOSTS; i++) {
        ghosts_textures[i] = malloc(4 * sizeof(SDL_Texture*));
    }

    load_pacman_textures(renderer, pacman_textures);
    for (int i = 0; i < 4; i++) {
        if (pacman_textures[i] == NULL) {
            fprintf(stderr, "Could not load pacman texture: %s\n", IMG_GetError());
            return EXIT_FAILURE;
        }
    }
    load_ghost_textures(renderer, ghosts_textures,NB_GHOSTS);

    
    //texture pour les ghosts vulnérables
    SDL_Texture* ghost_vulnerable_texture = load_texture(renderer, "images/ghost-blue-dizzy.png");

    //texture poue les ghosts morts
    SDL_Texture* ghost_dead_textures[4];
    load_ghost_dead_textures(renderer, ghost_dead_textures);
    for (int i = 0; i < 4; i++) {
        if (ghost_dead_textures[i] == NULL) {
            fprintf(stderr, "Could not load pacman texture: %s\n", IMG_GetError());
            return EXIT_FAILURE;
        }
    }

    //  texture echec et victoire
    SDL_Texture* echec_texture = load_texture(renderer, "images/echec.jpg");
    SDL_Texture* victoire_texture = load_texture(renderer, "images/victoire.jpg");
    //The texture of Pac-Man facing right is selected as the default texture and those for ghosts are random 
    SDL_Texture* pacman_texture = pacman_textures[3];
    SDL_Texture* actual_ghosts_textures[NB_GHOSTS];
    int actual_direction[NB_GHOSTS];
    for (int i = 0; i < NB_GHOSTS; i++){
        actual_direction[i]=rand()%4;//aléatoire
        actual_ghosts_textures[i]=ghosts_textures[i][actual_direction[i]];
    }
    int ghost_is_alive[NB_GHOSTS];
    for (int i = 0; i < NB_GHOSTS; i++){
        ghost_is_alive[i]=1;
    }

    
    Direction move_direction = DIR_NONE;
    int wanted_direction = 0;

    int pacman_is_alive=1;
    int ghosts_can_move=0;
    int nb_pacgommes_manges = 1;//il a déja mangé 1 dans sa position initiale
    int nb_pacgommes_total=1;
    for (int y = 0; y < N_Y_TILES; y++) {
        for (int x = 0; x < N_X_TILES; x++) {
            nb_pacgommes_total+=pacgommes[y][x];
        }
    }
    int ghosts_vulnerable = 0;    // Vaut 1 si les fantômes sont vulnérables, 0 sinon
    Uint32 vulnerable_start_time; // Moment où la vulnérabilité a commencé

    while (is_running)
    {
        // Fetch event qq soit pacman alive ou pas, pour pouvoir figer l'ecran s'il meurt jusqu'à l'utilisateur utilise q ou ferme la fenêtre
        SDL_Event event;
        SDL_PollEvent(&event);

        switch (event.type)
        {
            case SDL_QUIT:
                is_running = 0;
                break;

            case SDL_KEYDOWN:
                // A keyboard key was pressed down
                switch (event.key.keysym.sym)
                {
                    // It was a `q`, quit the program by exiting this loop
                    case SDLK_q:
                        is_running = 0;
                        break;
                    case SDLK_r:
                        // Restart the game
                        if (pacman_is_alive == 0) {
                            pacman_is_alive = 1;
                            pacman_texture = pacman_textures[3];
                           
                            // Reset the game variables
                            nb_pacgommes_manges = 1;
                            ghosts_vulnerable = 0;
                            ghosts_can_move = 0;
                            
                            // Reset Pac-Man's position
                            pacman_position = initial_pacman_position;
                            move_direction = DIR_NONE;
                            wanted_direction = 0;

                            // Reset ghosts' positions and states
                            for (int i = 0; i < NB_GHOSTS; i++) {
                                ghosts_positions[i] = initial_ghosts_positions[i];
                                ghost_is_alive[i] = 1; 
                                /*pas besoin de reintialiser la direction car elle est aléatoire à chaque fois
                                actual_direction[i] = rand() % 4;       
                                actual_ghosts_textures[i] = ghosts_textures[i][actual_direction[i]];*/
                            }

                            // Reset pac-gommes
                            for (int y = 0; y < N_Y_TILES; y++) {
                                for (int x = 0; x < N_X_TILES; x++) {
                                    if (map[y][x] == PATH) {
                                        pacgommes[y][x] = 1;
                                    }
                                }
                            }

                            for (int i = 0; i < N_Y_TILES; i++) {
                                for (int j = 0; j < N_X_TILES; j++){
                                    if (path_detected[i][j]==1){
                                        path_detected[i][j]=0;
                                    }
                                }
                            }
                        }
                        break;
                    case SDLK_p:
                        // Pause the game
                        if (pacman_is_alive == 1) {
                            int paused = 1;
                            while (paused) {
                                SDL_Event pause_event;
                                SDL_PollEvent(&pause_event);
                                if (pause_event.type == SDL_KEYDOWN && pause_event.key.keysym.sym == SDLK_p) {
                                    paused = 0; // Unpause the game
                                } else if (pause_event.type == SDL_QUIT) {
                                    is_running = 0;
                                    paused = 0;
                                }
                            }
                        }
                        break;
                    // It was an arrow key, save which one
                    case SDLK_UP:
                    case SDLK_DOWN:
                    case SDLK_LEFT:
                    case SDLK_RIGHT:
                        wanted_direction = event.key.keysym.sym;
                        ghosts_can_move=1;
                        break;
                }
                break;
        }
        if (pacman_is_alive==1)
        {
            switch (wanted_direction)
            {
                case SDLK_UP:
                    if (can_move(secret_map, pacman_position, tile_size, shift, PACMAN_SIZE, DIR_UP,N_X_TILES, N_Y_TILES))
                        move_direction = DIR_UP;
                    break;

                case SDLK_DOWN:
                    if (can_move(secret_map, pacman_position, tile_size, shift, PACMAN_SIZE, DIR_DOWN,N_X_TILES, N_Y_TILES))
                        move_direction = DIR_DOWN;
                    break;

                case SDLK_LEFT:
                    if (can_move(secret_map, pacman_position, tile_size, shift, PACMAN_SIZE, DIR_LEFT,N_X_TILES, N_Y_TILES))
                        move_direction = DIR_LEFT;
                    break;

                case SDLK_RIGHT:
                    if (can_move(secret_map, pacman_position, tile_size, shift, PACMAN_SIZE, DIR_RIGHT,N_X_TILES, N_Y_TILES))
                        move_direction = DIR_RIGHT;
                    break;
            }

            // Move the pacman according to the requested direction
            //Update of Pac-Man's texture and position
            move_entity(secret_map, &pacman_position, move_direction, tile_size, shift, PACMAN_SIZE, &pacman_texture, pacman_textures,N_X_TILES, N_Y_TILES);
           
            //faire déplacer les ghosts aléatoirement
            if (ghosts_can_move==1){
                for (int i = 0; i < NB_GHOSTS; i++)
                {   
                    int nb_choices=0;
                    Direction tab[4];
                    for  (Direction j = DIR_UP; j <DIR_NONE; j++)
                    {   
                        if( j != inverse_direction(actual_direction[i]) && can_move(map, ghosts_positions[i], tile_size, shift, GHOST_SIZE, j,N_X_TILES, N_Y_TILES))
                        {
                            tab[nb_choices]=j;
                            nb_choices++;
                        } 
                    }
                    if (nb_choices==0)
                    {
                        actual_direction[i] = inverse_direction(actual_direction[i]);
                    }
                    if (nb_choices==1)
                    {
                        actual_direction[i] =tab[0];
                    }    
                        
                    if (ghost_is_alive[i]==1)
                    {
                        if (nb_choices>1)
                        {
                            Direction optimal_direction = find_direction_method(method, map, ghosts_positions[i], pacman_position, tile_size, shift, GHOST_SIZE, N_X_TILES, N_Y_TILES, tab, nb_choices);
                            //dés qu'il peut tourner maintenant, alors on change de direction
                            if (can_move(map, ghosts_positions[i], tile_size, shift, GHOST_SIZE, optimal_direction, N_X_TILES, N_Y_TILES)) {
                                actual_direction[i] = optimal_direction;
                            }
                        }
                        move_entity(map, &ghosts_positions[i], actual_direction[i], tile_size, shift, GHOST_SIZE, &actual_ghosts_textures[i], ghosts_textures[i],N_X_TILES, N_Y_TILES);
                        
                        // Vérification de la collision avec Pac-Man
                        if (collision_with_pacman(ghosts_positions[i], pacman_position)) {
                            if (ghosts_vulnerable) {
                                // Pac-Man mange le fantôme : le fantôme disparaît (on le déplace hors de l'aire de jeu)
                                ghost_is_alive[i]=0;
                            } else {
                                // Collision fatale pour Pac-Man
                                pacman_is_alive = 0;
                            }
                        }
                    }else{
                        if (nb_choices>1)
                        {
                            Direction optimal_direction = find_optimal_direction_BFS(map, ghosts_positions[i], initial_ghosts_positions[i], tile_size,  shift, GHOST_SIZE,N_X_TILES, N_Y_TILES);
                            //dés qu'il peut tourner maintenant, alors on change de direction
                            if (can_move(map, ghosts_positions[i], tile_size, shift, GHOST_SIZE, optimal_direction, N_X_TILES, N_Y_TILES)) {
                                actual_direction[i] = optimal_direction;
                            }
                        }
                        move_entity(map, &ghosts_positions[i], actual_direction[i], tile_size, shift, GHOST_SIZE, &actual_ghosts_textures[i], ghosts_textures[i],N_X_TILES, N_Y_TILES);
                        if (SDL_HasIntersection(&ghosts_positions[i], &initial_ghosts_positions[i])) {
                            ghost_is_alive[i]=1; // ressusciter le fantôme
                            printf("Le fantôme %d est ressuscité !\n", i);
                        }
                    }
                }
            }    
        }
        
    
        // Compter le nombre de fantômes morts , on le fait après la boucle de déplacement des fantômes 
        //pour éviter de compter plusieurs fois le même fantôme
        int dead_ghosts = 0;
        for (int i = 0; i < NB_GHOSTS; i++) {
            if (ghost_is_alive[i] == 0) {
                dead_ghosts++;
            }
        }
        //dessiner la fenêtre
        
        //gere la durree de vunerabilite
        if (ghosts_vulnerable) {
            Uint32 time = SDL_GetTicks() - vulnerable_start_time;
            if (time > DUREE_VULNERABILITE) {
                // La période de vulnérabilité est terminée
                ghosts_vulnerable = 0;
            }
        }
        
        // Clear the window: remove everything that was drawn
        SDL_RenderClear(renderer);
        // Draw the map texture in the window
        SDL_RenderCopy(renderer, map_texture, NULL, &map_position);
        
        //dessiner le chemin secret lorsqu'on le trouve
        int x =pacman_position.x/tile_size;
        int y =pacman_position.y/tile_size;
        if( map[y][x]==WALL ){
            path_detected[y][x]=1;   
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_Rect rectangle = {
                            .x = x*tile_size,
                            .y = y*tile_size,
                            .w = tile_size,
                            .h = tile_size,
            };
            SDL_RenderFillRect(renderer, &rectangle);                                    
        }


        // Dessiner les pac-gommes et vérifier si Pac-Man les mange
        for (int y = 0; y < N_Y_TILES; y++) {
            for (int x = 0; x < N_X_TILES; x++) {
                if (pacgommes[y][x] == 1) {
                    // Dessiner une pac-gomme
                    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Jaune
                    SDL_Rect pacgomme = {
                        .x = x * tile_size + tile_size / 2 - 8, // Positionner au centre du carreau
                        .y = y * tile_size + tile_size / 2 - 8,
                        .w = 14, // Taille arbitraire pour la pac-gomme
                        .h = 14
                    };
                    SDL_RenderFillRect(renderer, &pacgomme);
                }
                if (pacgommes[y][x] && pacman_position.x / tile_size == x && pacman_position.y / tile_size == y) {
                    pacgommes[y][x] = 0; // Supprimer la pac-gomme       
                    nb_pacgommes_manges++; 
                    // Par exemple, toutes les 10 pac-gommes, on rend les fantômes vulnérables.
                    if (nb_pacgommes_manges % 10 == 0) {
                        ghosts_vulnerable = 1;
                        vulnerable_start_time = SDL_GetTicks();
                    }           
                }
                //une fois le chemin secret est decouvert on continue à le dessiner, sachant que l'utilisation de path_detected était nécéssaire pour ne pas changer map et ainsi farder le mouvement des ghosts comme il était
                if(path_detected[y][x]==1) {
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    SDL_Rect rectangle = {
                                    .x = x*tile_size,
                                    .y = y*tile_size,
                                    .w = tile_size,
                                    .h = tile_size,
                    };
                    SDL_RenderFillRect(renderer, &rectangle);
                }
            }
        }

        // Draw the ghost i texture 
        for (int i = 0; i < NB_GHOSTS; i++){
            if (ghost_is_alive[i]==1)
            {
                if (ghosts_vulnerable) {
                    // Afficher le fantôme vulnérable
                    SDL_RenderCopy(renderer, ghost_vulnerable_texture, NULL, &ghosts_positions[i]);
                } else {
                    // Afficher le fantôme dans son apparence normale (selon sa direction)
                    SDL_RenderCopy(renderer, actual_ghosts_textures[i], NULL, &ghosts_positions[i]);
                }
            }
            else{
                // Afficher le fantôme mort
                SDL_RenderCopy(renderer, ghost_dead_textures[actual_direction[i]], NULL, &ghosts_positions[i]);
            }
        }
        
        
        // Draw the pacman texture on top of what was previously drawn
        if (pacman_is_alive==1){
            SDL_RenderCopy(renderer, pacman_texture, NULL, &pacman_position);
        }
        else if (dead_ghosts!=NB_GHOSTS && nb_pacgommes_manges!=nb_pacgommes_total) //sinon lorsque il gagne , pacman_is_alive=0 et echec_rect et victoire_rect vont etre affichés
        {
            SDL_RenderCopy(renderer, echec_texture, NULL, &echec_rect);
        }

        // Vérifier si tous les fantômes ou toutes les pacgommes sont mangés sont morts alors afficher victoire 
        if (dead_ghosts==NB_GHOSTS || nb_pacgommes_manges==nb_pacgommes_total)
        {
            SDL_RenderCopy(renderer, victoire_texture, NULL, &victoire_rect);
            pacman_is_alive=0;
        }
        // Really show on the screen what we drew so far
        SDL_RenderPresent(renderer);

        // Wait 2 ms (reduce a bit processor usage and regulates speed of Pac-Man)
        SDL_Delay(2);

    }
    // Libération de la mémoire allouée dynamiquement
    for (int i = 0; i < N_Y_TILES; i++) {
        free(map[i]);           // Libérer chaque ligne de map
        free(secret_map[i]);   
        free(pacgommes[i]);     
        free(path_detected[i]);
    }
    // Libérer les pointeurs des lignes eux-mêmes
    free(map);          // Libérer le tableau des pointeurs de map
    free(secret_map);    
    free(pacgommes);     
    free(path_detected); 
    // Free all created resources
    for (int j = 0; j < 4; j++) {
        SDL_DestroyTexture(pacman_textures[j]);
        SDL_DestroyTexture(ghost_dead_textures[j]);
    }
    
    for (int i = 0; i < NB_GHOSTS; i++) {
        for (int j = 0; j < 4; j++) {
            SDL_DestroyTexture(ghosts_textures[i][j]);
        }
        free(ghosts_textures[i]);
    }
    free(ghosts_textures);
    
    SDL_DestroyTexture(pacman_texture);
    SDL_DestroyTexture(map_texture);
    SDL_DestroyTexture(ghost_vulnerable_texture);
    SDL_DestroyTexture(echec_texture);
    SDL_DestroyTexture(victoire_texture);
    
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(screen);
    
    // Quit the SDL program
    SDL_Delay(1000);
    SDL_Quit();

    return EXIT_SUCCESS;
}
