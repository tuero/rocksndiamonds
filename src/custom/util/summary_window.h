
#ifndef SUMMARY_WINDOW
#define SUMMARY_WINDOW

#include <iostream>
#include <string>
#include <vector>
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


class SummaryWindow {
public:
    static SDL_Window* window;                          // SDL window for display
    static SDL_Renderer* renderer;                      // SDL Render
    static std::map<int, SDL_Texture*> textures_map;    // Texture for each game object
    static std::vector<SDL_Rect> texture_rects;         // Position on screen to render each texture
    static std::map<int, std::string> graphics_map;     // Graphic locations
    static std::vector<std::vector<int>> grid_representation;   // Abstract nodes for the grid cells
    static std::vector<std::array<int, 3>> grid_colors;         // Grid colours 

    static int TILE_SIZE;                              // Pixel dimension of tile
    static int TILES_ROWS;                             // Number of tile rows
    static int TILES_COLS;                             // Number of tile columns
    static int TOTAL_TILES;                            // Total number of tiles on screen


    SummaryWindow() {}

    /*
     * Initialize the summary window.
     */
    static bool init();

    /*
     * Close the summary window.
     */
    static void close();

    /*
     * Draw the summary information.
     * Simplified board is drawn, along with planned abstractions and path.
     */
    static void draw();
};

#endif  // SUMMARY_WINDOW