

#ifndef ENGINE_TYPES_H
#define ENGINE_TYPES_H


// ------------- Includes  -------------
#include <array>

extern "C" {
    #include "../../main.h"
    #include "../../libgame/system.h"
}

namespace enginetype{
    // ------------- Short naming -------------
    typedef std::array<short, MAX_LEV_FIELDX * MAX_LEV_FIELDY> board_short;
    typedef std::array<int, MAX_LEV_FIELDX * MAX_LEV_FIELDY> board_int;
    typedef std::array<boolean, MAX_LEV_FIELDX * MAX_LEV_FIELDY> board_bool;
    typedef std::array<short, MAX_NUM_AMOEBA> amoeba_short;


    struct GridCell{
        int x; 
        int y;

        bool operator<(const GridCell& rhs) const {
            return rhs.x < x || (rhs.x == x && rhs.y < y);
        }

        bool operator==(const GridCell& rhs) const {
            return (x == rhs.x && y == rhs.y);
        }
    };

    class GridCellHash {
    public:
        std::size_t operator()(const GridCell& grid_cell) const {
            return grid_cell.y * MAX_LEV_FIELDY + grid_cell.x;
        }
    };


    // ------------- Controller types -------------
    enum ControllerType{BFS=CONTROLLER_TYPE_BFS, MCTS=CONTROLLER_TYPE_MCTS, 
                        USER=CONTROLLER_TYPE_USER, REPLAY=CONTROLLER_TYPE_REPLAY,
                        PFA=CONTROLLER_TYPE_PFA};

    enum Statistics{RUN_TIME, COUNT_EXPANDED_NODES, COUNT_SIMULATED_NODES, MAX_DEPTH};


    // ------------- Action values -------------
    static const int ENGINE_NOOP    = 0;
    static const int ENGINE_LEFT    = 1;
    static const int ENGINE_RIGHT   = 2; 
    static const int ENGINE_UP      = 4;
    static const int ENGINE_DOWN    = 8;
    static const int NUM_ACTIONS    = 5;


    // ------------- Field Values -------------
    static const int FIELD_EMPTY        = 0;
    static const int FIELD_BD_STEEL_WALL = 13;
    static const int FIELD_BD_NORMAL_WALL = 105;
    static const int FIELD_EM_STEEL_WALL = 13;
    static const int FIELD_EM_NORMAL_WALL = 2;
    static const int FIELD_YAMYAM       = 11;
    static const int FELD_DIAMOND       = 56;
    static const int FIELD_WALL         = 105;
    static const int FIELD_GOAL         = 107;
    static const int FIELD_CUSTOM_1     = 360;      // Diamond, no gravity
    static const int FIELD_CUSTOM_2     = 361;      // Yamyam, normal speed, deterministic delay
    static const int FIELD_CUSTOM_3     = 362;      // Yamyam, normal speed, stochastic delay
    static const int FIELD_CUSTOM_4     = 363;
    static const int FIELD_CUSTOM_5     = 364;
    static const int FIELD_CUSTOM_6     = 365;
    static const int FIELD_CUSTOM_7     = 366;
    static const int FIELD_CUSTOM_8     = 367;
    static const int FIELD_CUSTOM_9     = 368;
    static const int FIELD_CUSTOM_10    = 369;
    static const int FIELD_CUSTOM_11    = 370;
    static const int FIELD_CUSTOM_12    = 371;
    static const int FIELD_TEMP         = 1296;


    // ------------- Misc -------------
    static const int ENGINE_RESOLUTION = 8;     // Engine ticks per time step the agent considers

}



#endif  //ENGINE_TYPES_H