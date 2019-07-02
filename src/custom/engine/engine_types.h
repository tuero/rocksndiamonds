

#ifndef ENGINE_TYPES_H
#define ENGINE_TYPES_H


// ------------- Includes  -------------
#include <array>

extern "C" {
    #include "../../main.h"
}


// ------------- Short naming -------------
typedef std::array<short, MAX_LEV_FIELDX * MAX_LEV_FIELDY> board_short;
typedef std::array<int, MAX_LEV_FIELDX * MAX_LEV_FIELDY> board_int;
typedef std::array<boolean, MAX_LEV_FIELDX * MAX_LEV_FIELDY> board_bool;
typedef std::array<short, MAX_NUM_AMOEBA> amoeba_short;


// ------------- Action values -------------
static const int _ENGINE_NOOP    = 0;
static const int _ENGINE_LEFT    = 1;
static const int _ENGINE_RIGHT   = 2; 
static const int _ENGINE_UP      = 4;
static const int _ENGINE_DOWN    = 8;
static const int _NUM_ACTIONS    = 5;


// ------------- Field Values -------------
static const int _FIELD_EMPTY   = 0;
static const int _FIELD_WALL    = 105;
static const int _FIELD_GOAL    = 107;


// ------------- Misc -------------
static const int _ENGINE_RESOLUTION = 8;     // Engine ticks per time step the agent considers


#endif  //ENGINE_TYPES_H