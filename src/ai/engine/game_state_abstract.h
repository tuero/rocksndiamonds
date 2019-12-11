/**
 * @file: game_state_abstract.h
 *
 * @brief: Contains minimal information of the game state from the engine.
 * 
 * @author: Jake Tuero
 * Date: November 2019
 * Contact: tuero@ualberta.ca
 */

#ifndef GAMESTATE_ABSTRACT_H
#define GAMESTATE_ABSTRACT_H


#include <array>

// Engine
#include "engine_types.h"

extern "C" {
    #include "../../main.h"
    #include "../../events.h"
    #include "../../screens.h"
    #include "../../game.h"
}


/**
 * Holds minimal information of the game state from the engine.
 * 
 * Useful for planning at the grid level, where all that matters is the position of items
 * and the player on the game field.
 */
class AbstractGameState {
private:
    short Feld_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];            // Engine Feld state
    int spriteIDs_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];         // Sprite IDs and their locations
    struct PlayerInfo player_;                              // Player information

public:

    AbstractGameState();

    /*
     * Set the stored state to the current game engine state.
     */
    void setFromEngineState();

    /*
     * Restore the game engine state to the stored state.
     */
    void restoreEngineState();
};



#endif  //GAMESTATE_ABSTRACT_H