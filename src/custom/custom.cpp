
#include "custom.h"

#include <iostream>
#include <string>
#include <vector>

#include "engine/game_state.h"
#include "engine/action.h"
#include "engine/engine_helper.h"
#include "bfs/bfs.h"


boolean is_simulating;
std::vector<Action> solution;
int solution_index;

// Level info
char *level_bd = (char *)"classic_boulderdash";
char *level_em = (char *)"classic_emerald_mine";
char *level_custom = (char *)"tuero";


extern "C" void findPath() {
    GameState state;
    GameInfo saveGame;

    // Save current game state
    state.setFromSimulator();
    saveGame = game;

    // Find path
    is_simulating = TRUE;
    bfs(solution);
    solution_index = 0;
    is_simulating = FALSE;

    // Reset game state to before search
    game = saveGame;
    state.restoreSimulator();
}


extern "C" int getAction() {
    if ((int)solution.size() <= solution_index) {
        std::cout << "ERROR: Controller has no available action" << std::endl;
        return 0;
    }

    int action = solution[solution_index];
    solution_index += 1;
    std::cout << "Controller: " << actionToString(static_cast<Action>(action)) << std::endl;

    return action;
}


extern "C" void setLevel(int levelset) {
    char *levelset_dir;
    leveldir_current->basepath = (char *)"/home/tuero/.rocksndiamonds/levels";

    // Levelset subfolder names
    switch(levelset) {
        case 1: {levelset_dir = level_bd; break;}           // Boulder Dash
        case 2: {levelset_dir = level_em; break;}           // Emerald Mines
        default: {levelset_dir = level_custom; break;}      // Custom
    }

    leveldir_current->fullpath = levelset_dir;
    leveldir_current->subdir = levelset_dir;
    leveldir_current->identifier = levelset_dir;
} 


extern "C" void printBoardState() {
    std::cout << "DEBUG: Current State..." << std::endl;
    std::cout << "Engine Type: ";
    if (level.game_engine_type == GAME_ENGINE_TYPE_EM) {
        std::cout << "EM" << std::endl;
    }
    else if (level.game_engine_type == GAME_ENGINE_TYPE_UNKNOWN) {
        std::cout << "EM" << std::endl;
    }
    else if (level.game_engine_type == GAME_ENGINE_TYPE_RND) {
        std::cout << "EM" << std::endl;
    }

    int x, y;
    std::cout << "Player MovPos: " << stored_player[0].MovPos << std::endl;
    std::cout << "Player MovDir: " << stored_player[0].MovDir << std::endl;
    std::cout << "Player is_moving: " << stored_player[0].is_moving << std::endl;
    std::cout << "Player is_waiting: " << stored_player[0].is_waiting << std::endl;
    std::cout << "Player move_delay: " << stored_player[0].move_delay << std::endl;
    for (y = 0; y < level.fieldy; y++) {
        for (x = 0; x < level.fieldx; x++) {
            std::cout.width(5);
            std::cout << Feld[x][y] << " ";
        }
        std::cout << std::endl;
    }

    std::cout << "-----------------" << std::endl;
}