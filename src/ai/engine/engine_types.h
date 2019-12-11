/**
 * @file: engine_types.h
 *
 * @brief: Readable typedefs and enumerations for commonly accessed engine datatypes.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */


#ifndef ENGINE_TYPES_H
#define ENGINE_TYPES_H


// ------------- Includes  -------------
#include <array>
#include <string>
#include <unordered_map>
#include <set>

extern "C" {
    #include "../../main.h"
    #include "../../libgame/system.h"
}


/**
 * Action definition, based on internal engine definitions.
 */
enum Action {noop=MV_NONE, left=MV_LEFT, right=MV_RIGHT, up=MV_UP, down=MV_DOWN};


namespace enginetype {

    // ------------- Action sets -------------

    /**
     * Array of all actions.
     */
    extern const Action ALL_ACTIONS[5];

    /**
     * Array of all actions without NOOP
     * This is useful when running pathfinding at the grid level where we don't want to stand still.
     */
    extern const Action ALL_ACTIONS_NO_NOOP[4];

    // ------------- Short naming -------------
    typedef std::array<short, MAX_LEV_FIELDX * MAX_LEV_FIELDY> board_short;
    typedef std::array<int, MAX_LEV_FIELDX * MAX_LEV_FIELDY> board_int;
    typedef std::array<boolean, MAX_LEV_FIELDX * MAX_LEV_FIELDY> board_bool;
    typedef std::array<short, MAX_NUM_AMOEBA> amoeba_short;


    /*
     * Grid cell along with comparison methods for required hashing methods
     */
    struct GridCell {
        int x; 
        int y;

        bool operator<(const GridCell& rhs) const {
            return rhs.x < x || (rhs.x == x && rhs.y < y);
        }

        bool operator==(const GridCell& rhs) const {
            return (x == rhs.x && y == rhs.y);
        }

        bool operator!=(const GridCell& rhs) const {
            return (x != rhs.x || y != rhs.y);
        }
    };

    class GridCellHash {
    public:
        std::size_t operator()(const GridCell& grid_cell) const {
            return grid_cell.y * MAX_LEV_FIELDY + grid_cell.x;
        }
    };


    // ------------- Action values -------------
    static const int ENGINE_NOOP    = MV_NONE;
    static const int ENGINE_LEFT    = MV_LEFT;
    static const int ENGINE_RIGHT   = MV_RIGHT; 
    static const int ENGINE_UP      = MV_UP;
    static const int ENGINE_DOWN    = MV_DOWN;
    static const int NUM_ACTIONS    = 5;
    static const int ENGINE_RESOLUTION = 8;     // Engine ticks per time step the agent considers


    // ------------- Element Values -------------
    // Get rid of this eventually
    static const int FIELD_EMPTY            = 0;
    static const int FIELD_DIRT             = 1;
    static const int FIELD_BOULDER          = 106;
    static const int FIELD_DOOR_CLOSED      = 7;
    static const int FIELD_BD_STEEL_WALL    = 13;
    static const int FIELD_BD_NORMAL_WALL   = 105;
    static const int FIELD_EM_STEEL_WALL    = 13;
    static const int FIELD_EM_NORMAL_WALL   = 2;
    static const int FIELD_YAMYAM           = 11;
    static const int FIELD_DIAMOND          = 56;
    static const int FIELD_WALL             = 105;
    static const int FIELD_EXIT             = 107;
    static const int FIELD_CUSTOM_1         = 360;      // Diamond, no gravity
    static const int FIELD_CUSTOM_2         = 361;      // Yamyam, normal speed, deterministic delay
    static const int FIELD_CUSTOM_3         = 362;      // Yamyam, normal speed, stochastic delay
    static const int FIELD_CUSTOM_4         = 363;
    static const int FIELD_CUSTOM_5         = 364;
    static const int FIELD_CUSTOM_6         = 365;
    static const int FIELD_CUSTOM_7         = 366;
    static const int FIELD_CUSTOM_8         = 367;
    static const int FIELD_CUSTOM_9         = 368;
    static const int FIELD_CUSTOM_10        = 369;
    static const int FIELD_CUSTOM_11        = 370;
    static const int FIELD_CUSTOM_12        = 371;
    static const int FIELD_DOOR_OPENING     = 1241;
    static const int FIELD_TEMP             = 1296;
    static const int FIELD_EXPLOSION        = 1297;


} // namespace enginetype



#endif  //ENGINE_TYPES_H