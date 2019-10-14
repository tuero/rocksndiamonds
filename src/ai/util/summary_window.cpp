/**
 * @file: summary_window.cpp
 *
 * @brief: Second window to display summarized information of planner and objects in game.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */

#include "summary_window.h"

#include <iostream>
#include <string>
#include <map>
#include <array>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// Engine access
#include "../engine/engine_types.h"
#include "../engine/engine_helper.h"

//Logging
#include "../util/logging_wrapper.h"
#include <plog/Log.h>  


namespace summarywindow {

// Grid sizes/metrics
int TILE_SIZE = 20;                                     // Tile size in pixels
int TILES_ROWS;                                         // Number of tile rows
int TILES_COLS;                                         // Number of tile columns
int TOTAL_TILES;                                        // Total number of tiles


bool setFlag = false;                                   // Flag to check for proper setup
SDL_Window* window_;                                    // SDL window
SDL_Renderer* renderer_;                                // SDL renderer for window
std::map<int, SDL_Texture*> texturesMap_;               // Texture maps for each image
std::vector<SDL_Rect> textureRects_;                    // Texture for each grid

// Graphics
const std::string IMG_DIR = "./src/custom/graphics/";   // Graphics directory
std::map<int, std::string> graphicsMap_ = {             // Feld type to image map
        {-1, "avatar.png"}, 
        {enginetype::FIELD_DIAMOND, "diamond.png"}, 
        {enginetype::FIELD_CUSTOM_1, "diamond.png"}, 
        {enginetype::FIELD_YAMYAM, "yamyam.png"}, 
        {enginetype::FIELD_CUSTOM_2, "yamyam.png"}, 
        {enginetype::FIELD_EXIT, "door.png"},
        {enginetype::FIELD_WALL, "wall.png"}    
};
std::vector<std::vector<int>> gridRepresentation_;      // Grid ids 
std::vector<std::array<int, 3>> gridColors_ = {         // Colour for abstract grids
    {0, 102, 102},           // turquoise 
    {153, 76, 0},            // green
    {102, 0, 102},           // majenta
    {0, 0, 102}              // deep blue
};


/*
 * Initialize the summary window.
 *
 * SDL functions called to initialize window and renderer. Textures are created for the
 * images representing objects on the summary window.
 */
bool init() {
    TILES_ROWS = enginehelper::getLevelHeight();
    TILES_COLS = enginehelper::getLevelWidth();
    TOTAL_TILES = TILES_ROWS * TILES_COLS;

    int WIDTH = TILES_COLS * TILE_SIZE;
    int HEIGHT = TILES_ROWS * TILE_SIZE;

    // Create window
    window_ = SDL_CreateWindow("summary window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (window_ == NULL) {
        LOGE_(logwrap::FileLogger) << "Could not create window: " << SDL_GetError();
        LOGE_(logwrap::ConsolLogger) << "Could not create window: " << SDL_GetError();
        return false;
    }

    // Attach render to window
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
    if(renderer_ == NULL) {
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
    texturesMap_.clear();
    for (auto it = graphicsMap_.begin(); it != graphicsMap_.end(); ++it) {
        texturesMap_[it->first] = IMG_LoadTexture(renderer_, (IMG_DIR + it->second).c_str());
    }

    // Set texture rectangles
    textureRects_.clear();
    for (int row = 0; row < TILES_ROWS; row++) {
        for (int col = 0; col < TILES_COLS; col++) {
            SDL_Rect texture_rect;
            texture_rect.x = col*TILE_SIZE;
            texture_rect.y = row*TILE_SIZE;
            texture_rect.w = TILE_SIZE;
            texture_rect.h = TILE_SIZE;
            textureRects_.push_back(texture_rect);
        }
    }

    SDL_RenderPresent(renderer_);
    setFlag = true;

    return true;
}


/*
 * Close the summary window and cleanup renderer.
 */
void close() {
    SDL_DestroyRenderer(renderer_); 
    SDL_DestroyWindow(window_);
}


/*
 * Update the stored grid positions of abstract nodes/objects
 *
 * @param gridRepresentation Vector array of abstract nodes/objects
 */
void updateGridRepresentation(std::vector<std::vector<int>> &gridRepresentation) {
    gridRepresentation_ = gridRepresentation;
}


/*
 * Draw the summary information.
 * 
 * Simplified board is drawn, along with planned abstractions and path. Images for objects
 * which we care about are drawn. Abstract nodes have their represented grid cells in 
 * the same colour.
 */
void draw() {
    if (!setFlag) {return;}

    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);

    int counter = 0;
    enginetype::GridCell player_pos = enginehelper::getPlayerPosition();

    for (int row = 0; row < TILES_ROWS; row++) {
        for (int col = 0; col < TILES_COLS; col++) {
            // Abstracted nodes representing grid cells
            if ((int)gridRepresentation_.size() > col && (int)gridRepresentation_[col].size() > row) {
                int id = gridRepresentation_[col][row];
                
                if (id == -1) {
                    SDL_SetRenderDrawColor(renderer_, 255, 0, 0, 255);       // r, g, b, a
                    SDL_RenderFillRect(renderer_, &textureRects_[counter]);
                }
                else if (id == -2) {
                    SDL_SetRenderDrawColor(renderer_, 0, 255, 0, 255);       // r, g, b, a
                    SDL_RenderFillRect(renderer_, &textureRects_[counter]);
                }
                else if (id == -3) {
                    SDL_SetRenderDrawColor(renderer_, 128, 128, 128, 255);       // r, g, b, a
                    SDL_RenderFillRect(renderer_, &textureRects_[counter]);
                }
                else {
                    SDL_SetRenderDrawColor(renderer_, gridColors_[id][0], gridColors_[id][1], gridColors_[id][2], 255);       // r, g, b, a
                    SDL_RenderDrawRect(renderer_, &textureRects_[counter]);
                }
                SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
            }


            // Draw player
            if (col == player_pos.x && row == player_pos.y) {
                SDL_RenderCopy(renderer_, texturesMap_[-1], NULL, &textureRects_[counter]);
            }

            // Draw texture representing item in map
            int item_id = enginehelper::getGridItem({col, row});
            if (graphicsMap_.find(item_id) != graphicsMap_.end()) {
                SDL_RenderCopy(renderer_, texturesMap_[item_id], NULL, &textureRects_[counter]);
            }
            counter += 1;
        }
    }

    SDL_RenderPresent(renderer_);
}

} // namespace summarywindow