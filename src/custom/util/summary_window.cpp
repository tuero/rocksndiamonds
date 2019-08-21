

#include "summary_window.h"

int SummaryWindow::TILE_SIZE = 20;   
int SummaryWindow::TILES_ROWS;
int SummaryWindow::TILES_COLS;
int SummaryWindow::TOTAL_TILES; 

SDL_Window* SummaryWindow::window;
SDL_Renderer* SummaryWindow::renderer;
std::map<int, SDL_Texture*> SummaryWindow::textures_map;
std::vector<SDL_Rect> SummaryWindow::texture_rects;
std::map<int, std::string> SummaryWindow::graphics_map = {
        {-1, "./src/custom/graphics/avatar.png"}, 
        {enginetype::FELD_DIAMOND, "./src/custom/graphics/diamond.png"}, 
        {enginetype::FIELD_CUSTOM_1, "./src/custom/graphics/diamond.png"}, 
        {enginetype::FIELD_YAMYAM, "./src/custom/graphics/yamyam.png"}, 
        {enginetype::FIELD_CUSTOM_2, "./src/custom/graphics/yamyam.png"}, 
        {enginetype::FIELD_GOAL, "./src/custom/graphics/door.png"},
        {enginetype::FIELD_WALL, "./src/custom/graphics/wall.png"}    
};

std::vector<std::vector<int>> SummaryWindow::grid_representation;
std::vector<std::array<int, 3>> SummaryWindow::grid_colors = {
    {0, 102, 102},           // turquoise 
    {153, 76, 0},            // green
    {102, 0, 102},           // majenta
    {0, 0, 102}              // deep blue
};


/*
 * Initialize the summary window.
 */
bool SummaryWindow::init() {
    TILES_ROWS = enginehelper::getLevelHeight();
    TILES_COLS = enginehelper::getLevelWidth();

    int WIDTH = TILES_COLS * TILE_SIZE;
    int HEIGHT = TILES_ROWS * TILE_SIZE;
    TOTAL_TILES = TILES_ROWS * TILES_COLS;

    // Create window
    window = SDL_CreateWindow("summary window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        LOGE_(logwrap::FileLogger) << "Could not create window: " << SDL_GetError();
        LOGE_(logwrap::ConsolLogger) << "Could not create window: " << SDL_GetError();
        return false;
    }

    // Attach render to window
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(renderer == NULL) {
        LOGE_(logwrap::FileLogger) << "Renderer could not be created: " << SDL_GetError();
        LOGE_(logwrap::ConsolLogger) << "Renderer could not be created: " << SDL_GetError();
        return false;
    }

    // Initialize SDL image for textures
    int imgFlags = IMG_INIT_PNG;
    if(!(IMG_Init(imgFlags) & imgFlags)) {
        LOGE_(logwrap::FileLogger) << "SDL_image could not initialize: " << IMG_GetError();
        LOGE_(logwrap::ConsolLogger) << "SDL_image could not initialize: " << IMG_GetError();
        return false;
    }

    // Load textures
    textures_map.clear();
    for (auto it = graphics_map.begin(); it != graphics_map.end(); ++it) {
        textures_map[it->first] = IMG_LoadTexture(renderer, it->second.c_str());
    }

    // Set texture rectangles
    texture_rects.clear();
    for (int row = 0; row < TILES_ROWS; row++) {
        for (int col = 0; col < TILES_COLS; col++) {
            SDL_Rect texture_rect;
            texture_rect.x = col*TILE_SIZE;
            texture_rect.y = row*TILE_SIZE;
            texture_rect.w = TILE_SIZE;
            texture_rect.h = TILE_SIZE;
            texture_rects.push_back(texture_rect);
        }
    }

    SDL_RenderPresent(renderer);

    return true;
}


/*
 * Close the summary window.
 */
void SummaryWindow::close() {
    SDL_DestroyRenderer(renderer); 
    SDL_DestroyWindow(window);
}


/*
 * Draw the summary information.
 * Simplified board is drawn, along with planned abstractions and path.
 */
void SummaryWindow::draw() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    int counter = 0;
    enginetype::GridCell player_pos = enginehelper::getPlayerPosition();

    for (int row = 0; row < TILES_ROWS; row++) {
        for (int col = 0; col < TILES_COLS; col++) {
            // Abstracted nodes representing grid cells
            if (grid_representation.size() > col && grid_representation[col].size() > row) {
                int id = grid_representation[col][row];
                
                if (id == -1) {
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);       // r, g, b, a
                    SDL_RenderFillRect(renderer, &texture_rects[counter]);
                }
                else if (id == -2) {
                    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);       // r, g, b, a
                    SDL_RenderFillRect(renderer, &texture_rects[counter]);
                }
                else if (id == -3) {
                    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);       // r, g, b, a
                    SDL_RenderFillRect(renderer, &texture_rects[counter]);
                }
                else {
                    SDL_SetRenderDrawColor(renderer, grid_colors[id][0], grid_colors[id][1], grid_colors[id][2], 255);       // r, g, b, a
                    SDL_RenderDrawRect(renderer, &texture_rects[counter]);
                }
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            }


            // Draw player
            if (col == player_pos.x && row == player_pos.y) {
                SDL_RenderCopy(renderer, textures_map[-1], NULL, &texture_rects[counter]);
            }

            // Draw texture representing item in map
            int item_id = enginehelper::getGridItem({col, row});
            if (graphics_map.find(item_id) != graphics_map.end()) {
                SDL_RenderCopy(renderer, textures_map[item_id], NULL, &texture_rects[counter]);
            }
            counter += 1;
        }
    }

    SDL_RenderPresent(renderer);
}