

#include "engine_helper.h"


/*
 * Check if the current status of the engine is loss of life
 */
bool engineGameFailed() {
    return checkGameFailed();
}


/*
 * Check if the current status of the engine is level solved
 */
bool engineGameSolved() {
    return (game.LevelSolved && !game.LevelSolved_GameEnd);
}


/*
 * Set the action for the engine to perform on behalf of the player on
 * the next iteration
 *
 * @param action -> Action to perform (may be noop)
 */
void setEnginePlayerAction(Action action) {
    stored_player[0].action = action;
}


/*
 * Simulate the engine ahead a single tick
 */
void engineSimulateSingle() {
    HandleGameActions();
}


/*
 * Simulate the engine ahead
 * This performs ENGINE_RESOLUTION ticks
 */
void engineSimulate() {
    for (int i = 0; i < 8; i++) {
        HandleGameActions();
    }
}


/*
 * Returns true if a wall is on the direction the player wants to move
 * Assumes simulator is in the current state to check
 */
bool isWall(Action action) {
    int playerX = stored_player[0].jx;
    int playerY = stored_player[0].jy;

    if (action == Action::down && Feld[playerX][playerY+1] == _FIELD_WALL) {
        return true;
    }
    else if (action == Action::right && Feld[playerX+1][playerY] == _FIELD_WALL) {
        return true;
    }
    else if (action == Action::up && Feld[playerX][playerY-1] == _FIELD_WALL) {
        return true;
    }
    else if (action == Action::left && Feld[playerX-1][playerY] == _FIELD_WALL) {
        return true;
    }
    return false;

    // if (action == Action::down && (Feld[playerX][playerY+1] != _FIELD_EMPTY && Feld[playerX][playerY+1] != _FIELD_GOAL)) {
    //     return true;
    // }
    // else if (action == Action::right && (Feld[playerX+1][playerY] != _FIELD_EMPTY && Feld[playerX+1][playerY] != _FIELD_GOAL)) {
    //     return true;
    // }
    // else if (action == Action::up && (Feld[playerX][playerY-1] != _FIELD_EMPTY && Feld[playerX][playerY-1] != _FIELD_GOAL)) {
    //     return true;
    // }
    // else if (action == Action::left && (Feld[playerX-1][playerY] != _FIELD_EMPTY && Feld[playerX-1][playerY] != _FIELD_GOAL)) {
    //     return true;
    // }
    // return false;

}


/*
 * Print current engine status
 */
void debug_print() {
    int x, y;
    std::cout << "Player x : " << stored_player[0].jx;
    std::cout << " y : " << stored_player[0].jy;
    std::cout << "Player MovPos: " << stored_player[0].MovPos << std::endl;
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