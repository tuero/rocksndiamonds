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

// Game headers
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
     * Number of player actions 
     */
    const int NUM_MV = 5;

    /**
     * Array of all actions.
     */
    extern const Action ALL_ACTIONS[NUM_MV];

    /**
     * Array of all actions without NOOP
     * This is useful when running pathfinding at the grid level where we don't want to stand still.
     */
    extern const Action ALL_ACTIONS_NO_NOOP[NUM_MV - 1];


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


    /**
     * Information for observed elements on the map.
     */
    struct ElementFullObservation {
        GridCell currentCell;           // Cell the element is currently in
        GridCell reservedCell;          // Cell the element has reserved if it is moving into that cell
        int spriteId;                   // Unique identification for that sprite (differentiate multiple elements of same type)
        int element;                    // Element type
        Action directionMoving;            // Direction the element is moving in
    };

    /**
     * Information for partially observed elements on the map.
     */
    struct ElementPartialObservation {
        GridCell currentCell;           // Cell the element is currently in
        GridCell reservedCell;          // Cell the element has reserved if it is moving into that cell
        int element;                    // Element type
        Action directionMoving;            // Direction the element is moving in
    };


    // ------------- Action values -------------
    static const int ENGINE_MV_NONE    = MV_NONE;
    static const int ENGINE_MV_LEFT    = MV_LEFT;
    static const int ENGINE_MV_RIGHT   = MV_RIGHT; 
    static const int ENGINE_MV_UP      = MV_UP;
    static const int ENGINE_MV_DOWN    = MV_DOWN;
    static const int NUM_ACTIONS       = 5;
    static const int DOOR_CHANGE_DELAY = 29;


    static const int NUM_ELEMENTS = MAX_NUM_ELEMENTS;


    // ------------- Element Values -------------
    static const int ELEMENT_EXIT_OPENING                   = EL_EXIT_OPENING;
    static const int ELEMENT_EXIT_CLOSING                   = EL_EXIT_CLOSING;
    static const int ELEMENT_BLOCKED                        = EL_BLOCKED;
    static const int ELEMENT_EXPLOSION                      = EL_EXPLOSION;
    static const int ELEMENT_NUT_BREAKING                   = EL_NUT_BREAKING;
    static const int ELEMENT_DIAMOND_BREAKING               = EL_DIAMOND_BREAKING;
    static const int ELEMENT_AMOEBA_GROWING                 = EL_AMOEBA_GROWING;
    static const int ELEMENT_AMOEBA_SHRINKING               = EL_AMOEBA_SHRINKING;
    static const int ELEMENT_EXPANDABLE_WALL_GROWING        = EL_EXPANDABLE_WALL_GROWING;
    static const int ELEMENT_EXPANDABLE_STEELWALL_GROWING   = EL_EXPANDABLE_STEELWALL_GROWING;
    static const int ELEMENT_PLAYER_IS_LEAVING              = EL_PLAYER_IS_LEAVING;
    static const int ELEMENT_PLAYER_IS_EXPLODING            = EL_PLAYER_IS_EXPLODING_1;

    static const int ELEMENT_EMPTY                          = EL_EMPTY;
    static const int ELEMENT_SAND                           = EL_SAND;
    static const int ELEMENT_WALL                           = EL_WALL;
    static const int ELEMENT_WALL_SLIPPERY                  = EL_WALL_SLIPPERY;
    static const int ELEMENT_ROCK                           = EL_ROCK;
    static const int ELEMENT_EMERALD                        = EL_EMERALD;
    static const int ELEMENT_EXIT_CLOSED                    = EL_EXIT_CLOSED;
    static const int ELEMENT_BUG                            = EL_BUG;
    static const int ELEMENT_SPACESHIP                      = EL_SPACESHIP;
    static const int ELEMENT_YAMYAM                         = EL_YAMYAM;
    static const int ELEMENT_ROBOT                          = EL_ROBOT;
    static const int ELEMENT_STEELWALL                      = EL_STEELWALL;
    static const int ELEMENT_DIAMOND                        = EL_DIAMOND;
    static const int ELEMENT_AMOEBA_DEAD                    = EL_AMOEBA_DEAD;
    static const int ELEMENT_QUICKSAND_EMPTY                = EL_QUICKSAND_EMPTY;
    static const int ELEMENT_QUICKSAND_FULL                 = EL_QUICKSAND_FULL;
    static const int ELEMENT_AMOEBA_DROP                    = EL_AMOEBA_DROP;
    static const int ELEMENT_BOMB                           = EL_BOMB;
    static const int ELEMENT_MAGIC_WALL                     = EL_MAGIC_WALL;
    static const int ELEMENT_SPEED_PILL                     = EL_SPEED_PILL;
    static const int ELEMENT_ACID                           = EL_ACID;
    static const int ELEMENT_AMOEBA_WET                     = EL_AMOEBA_WET;
    static const int ELEMENT_AMOEBA_DRY                     = EL_AMOEBA_DRY;
    static const int ELEMENT_NUT                            = EL_NUT;
    static const int ELEMENT_GAME_OF_LIFE                   = EL_GAME_OF_LIFE;
    static const int ELEMENT_BIOMAZE                        = EL_BIOMAZE;
    static const int ELEMENT_DYNAMITE_ACTIVE                = EL_DYNAMITE_ACTIVE;
    static const int ELEMENT_STONEBLOCK                     = EL_STONEBLOCK;
    static const int ELEMENT_ROBOT_WHEEL                    = EL_ROBOT_WHEEL;
    static const int ELEMENT_ROBOT_WHEEL_ACTIVE             = EL_ROBOT_WHEEL_ACTIVE;
    static const int ELEMENT_KEY_1                          = EL_KEY_1;
    static const int ELEMENT_KEY_2                          = EL_KEY_2;
    static const int ELEMENT_KEY_3                          = EL_KEY_3;
    static const int ELEMENT_KEY_4                          = EL_KEY_4;
    static const int ELEMENT_GATE_1                         = EL_GATE_1;
    static const int ELEMENT_GATE_2                         = EL_GATE_2;
    static const int ELEMENT_GATE_3                         = EL_GATE_3;
    static const int ELEMENT_GATE_4                         = EL_GATE_4;
    static const int ELEMENT_GATE_1_GRAY                    = EL_GATE_1_GRAY;
    static const int ELEMENT_GATE_2_GRAY                    = EL_GATE_2_GRAY;
    static const int ELEMENT_GATE_3_GRAY                    = EL_GATE_3_GRAY;
    static const int ELEMENT_GATE_4_GRAY                    = EL_GATE_4_GRAY;
    static const int ELEMENT_DYNAMITE                       = EL_DYNAMITE;
    static const int ELEMENT_PACMAN                         = EL_PACMAN;
    static const int ELEMENT_INVISIBLE_WALL                 = EL_INVISIBLE_WALL;
    static const int ELEMENT_LAMP                           = EL_LAMP;
    static const int ELEMENT_LAMP_ACTIVE                    = EL_LAMP_ACTIVE;
    static const int ELEMENT_WALL_EMERALD                   = EL_WALL_EMERALD;
    static const int ELEMENT_WALL_DIAMOND                   = EL_WALL_DIAMOND;
    static const int ELEMENT_AMOEBA_FULL                    = EL_AMOEBA_FULL;
    static const int ELEMENT_BD_AMOEBA                      = EL_BD_AMOEBA;
    static const int ELEMENT_TIME_ORB_FULL                  = EL_TIME_ORB_FULL;
    static const int ELEMENT_TIME_ORB_EMPTY                 = EL_TIME_ORB_EMPTY;
    static const int ELEMENT_EXPANDABLE_WALL                = EL_EXPANDABLE_WALL;
    static const int ELEMENT_BD_DIAMOND                     = EL_BD_DIAMOND;
    static const int ELEMENT_EMERALD_YELLOW                 = EL_EMERALD_YELLOW;
    static const int ELEMENT_WALL_BD_DIAMOND                = EL_WALL_BD_DIAMOND;
    static const int ELEMENT_WALL_EMERALD_YELLOW            = EL_WALL_EMERALD_YELLOW;
    static const int ELEMENT_DARK_YAMYAM                    = EL_DARK_YAMYAM;
    static const int ELEMENT_BD_MAGIC_WALL                  = EL_BD_MAGIC_WALL;
    static const int ELEMENT_INVISIBLE_STEELWALL            = EL_INVISIBLE_STEELWALL;
    static const int ELEMENT_SOKOBAN_FIELD_PLAYER           = EL_SOKOBAN_FIELD_PLAYER;
    static const int ELEMENT_DYNABOMB_INCREASE_NUMBER       = EL_DYNABOMB_INCREASE_NUMBER;
    static const int ELEMENT_DYNABOMB_INCREASE_SIZE         = EL_DYNABOMB_INCREASE_SIZE;
    static const int ELEMENT_DYNABOMB_INCREASE_POWER        = EL_DYNABOMB_INCREASE_POWER;
    static const int ELEMENT_SOKOBAN_OBJECT                 = EL_SOKOBAN_OBJECT;
    static const int ELEMENT_SOKOBAN_FIELD_EMPTY            = EL_SOKOBAN_FIELD_EMPTY;
    static const int ELEMENT_SOKOBAN_FIELD_FULL             = EL_SOKOBAN_FIELD_FULL;
    static const int ELEMENT_BD_BUTTERFLY_RIGHT             = EL_BD_BUTTERFLY_RIGHT;
    static const int ELEMENT_BD_BUTTERFLY_UP                = EL_BD_BUTTERFLY_UP;
    static const int ELEMENT_BD_BUTTERFLY_LEFT              = EL_BD_BUTTERFLY_LEFT;
    static const int ELEMENT_BD_BUTTERFLY_DOWN              = EL_BD_BUTTERFLY_DOWN;
    static const int ELEMENT_BD_FIREFLY_RIGHT               = EL_BD_FIREFLY_RIGHT;
    static const int ELEMENT_BD_FIREFLY_UP                  = EL_BD_FIREFLY_UP;
    static const int ELEMENT_BD_FIREFLY_LEFT                = EL_BD_FIREFLY_LEFT;
    static const int ELEMENT_BD_FIREFLY_DOWN                = EL_BD_FIREFLY_DOWN;
    static const int ELEMENT_BD_BUTTERFLY_1                 = EL_BD_BUTTERFLY_1;
    static const int ELEMENT_BD_BUTTERFLY_2                 = EL_BD_BUTTERFLY_2;
    static const int ELEMENT_BD_BUTTERFLY_3                 = EL_BD_BUTTERFLY_3;
    static const int ELEMENT_BD_BUTTERFLY_4                 = EL_BD_BUTTERFLY_4;
    static const int ELEMENT_BD_FIREFLY_1                   = EL_BD_FIREFLY_1;
    static const int ELEMENT_BD_FIREFLY_2                   = EL_BD_FIREFLY_2;
    static const int ELEMENT_BD_FIREFLY_3                   = EL_BD_FIREFLY_3;
    static const int ELEMENT_BD_FIREFLY_4                   = EL_BD_FIREFLY_4;
    static const int ELEMENT_BD_BUTTERFLY                   = EL_BD_BUTTERFLY;
    static const int ELEMENT_BD_FIREFLY                     = EL_BD_FIREFLY;
    static const int ELEMENT_PLAYER_1                       = EL_PLAYER_1;
    static const int ELEMENT_PLAYER_2                       = EL_PLAYER_2;
    static const int ELEMENT_PLAYER_3                       = EL_PLAYER_3;
    static const int ELEMENT_PLAYER_4                       = EL_PLAYER_4;
    static const int ELEMENT_BUG_RIGHT                      = EL_BUG_RIGHT;
    static const int ELEMENT_BUG_UP                         = EL_BUG_UP;
    static const int ELEMENT_BUG_LEFT                       = EL_BUG_LEFT;
    static const int ELEMENT_BUG_DOWN                       = EL_BUG_DOWN;
    static const int ELEMENT_SPACESHIP_RIGHT                = EL_SPACESHIP_RIGHT;
    static const int ELEMENT_SPACESHIP_UP                   = EL_SPACESHIP_UP;
    static const int ELEMENT_SPACESHIP_LEFT                 = EL_SPACESHIP_LEFT;
    static const int ELEMENT_SPACESHIP_DOWN                 = EL_SPACESHIP_DOWN;
    static const int ELEMENT_PACMAN_RIGHT                   = EL_PACMAN_RIGHT;
    static const int ELEMENT_PACMAN_UP                      = EL_PACMAN_UP;
    static const int ELEMENT_PACMAN_LEFT                    = EL_PACMAN_LEFT;
    static const int ELEMENT_PACMAN_DOWN                    = EL_PACMAN_DOWN;
    static const int ELEMENT_EMERALD_RED                    = EL_EMERALD_RED;
    static const int ELEMENT_EMERALD_PURPLE                 = EL_EMERALD_PURPLE;
    static const int ELEMENT_WALL_EMERALD_RED               = EL_WALL_EMERALD_RED;
    static const int ELEMENT_WALL_EMERALD_PURPLE            = EL_WALL_EMERALD_PURPLE;
    static const int ELEMENT_ACID_POOL_TOPLEFT              = EL_ACID_POOL_TOPLEFT;
    static const int ELEMENT_ACID_POOL_TOPRIGHT             = EL_ACID_POOL_TOPRIGHT;
    static const int ELEMENT_ACID_POOL_BOTTOMLEFT           = EL_ACID_POOL_BOTTOMLEFT;
    static const int ELEMENT_ACID_POOL_BOTTOM               = EL_ACID_POOL_BOTTOM;
    static const int ELEMENT_ACID_POOL_BOTTOMRIGHT          = EL_ACID_POOL_BOTTOMRIGHT;
    static const int ELEMENT_BD_WALL                        = EL_BD_WALL;
    static const int ELEMENT_BD_ROCK                        = EL_BD_ROCK;
    static const int ELEMENT_EXIT_OPEN                      = EL_EXIT_OPEN;
    static const int ELEMENT_BLACK_ORB                      = EL_BLACK_ORB;
    static const int ELEMENT_AMOEBA_TO_DIAMOND              = EL_AMOEBA_TO_DIAMOND;
    static const int ELEMENT_MOLE                           = EL_MOLE;
    static const int ELEMENT_PENGUIN                        = EL_PENGUIN;
    static const int ELEMENT_SATELLITE                      = EL_SATELLITE;
    static const int ELEMENT_ARROW_LEFT                     = EL_ARROW_LEFT;
    static const int ELEMENT_ARROW_RIGHT                    = EL_ARROW_RIGHT;
    static const int ELEMENT_ARROW_UP                       = EL_ARROW_UP;
    static const int ELEMENT_ARROW_DOWN                     = EL_ARROW_DOWN;
    static const int ELEMENT_PIG                            = EL_PIG;
    static const int ELEMENT_DRAGON                         = EL_DRAGON;
    
    static const int ELEMENT_EXPANDABLE_WALL_HORIZONTAL     = EL_EXPANDABLE_WALL_HORIZONTAL;
    static const int ELEMENT_EXPANDABLE_WALL_VERTICAL       = EL_EXPANDABLE_WALL_VERTICAL;
    static const int ELEMENT_EXPANDABLE_WALL_ANY            = EL_EXPANDABLE_WALL_ANY;
    static const int ELEMENT_EM_GATE_1                      = EL_EM_GATE_1;
    static const int ELEMENT_EM_GATE_2                      = EL_EM_GATE_2;
    static const int ELEMENT_EM_GATE_3                      = EL_EM_GATE_3;
    static const int ELEMENT_EM_GATE_4                      = EL_EM_GATE_4;
    
    static const int ELEMENT_SP_START                       = EL_SP_START;
    static const int ELEMENT_SP_EMPTY_SPACE                 = EL_SP_EMPTY_SPACE;
    static const int ELEMENT_SP_EMPTY                       = EL_SP_EMPTY;
    static const int ELEMENT_SP_ZONK                        = EL_SP_ZONK;
    static const int ELEMENT_SP_BASE                        = EL_SP_BASE;
    static const int ELEMENT_SP_MURPHY                      = EL_SP_MURPHY;
    static const int ELEMENT_SP_INFOTRON                    = EL_SP_INFOTRON;
    static const int ELEMENT_SP_CHIP_SINGLE                 = EL_SP_CHIP_SINGLE;
    static const int ELEMENT_SP_HARDWARE_GRAY               = EL_SP_HARDWARE_GRAY;
    static const int ELEMENT_SP_EXIT_CLOSED                 = EL_SP_EXIT_CLOSED;
    static const int ELEMENT_SP_DISK_ORANGE                 = EL_SP_DISK_ORANGE;
    static const int ELEMENT_SP_PORT_RIGHT                  = EL_SP_PORT_RIGHT;
    static const int ELEMENT_SP_PORT_DOWN                   = EL_SP_PORT_DOWN;
    static const int ELEMENT_SP_PORT_LEFT                   = EL_SP_PORT_LEFT;
    static const int ELEMENT_SP_PORT_UP                     = EL_SP_PORT_UP;
    static const int ELEMENT_SP_GRAVITY_PORT_RIGHT          = EL_SP_GRAVITY_PORT_RIGHT;
    static const int ELEMENT_SP_GRAVITY_PORT_DOWN           = EL_SP_GRAVITY_PORT_DOWN;
    static const int ELEMENT_SP_GRAVITY_PORT_LEFT           = EL_SP_GRAVITY_PORT_LEFT;
    static const int ELEMENT_SP_GRAVITY_PORT_UP             = EL_SP_GRAVITY_PORT_UP;
    static const int ELEMENT_SP_SNIKSNAK                    = EL_SP_SNIKSNAK;
    static const int ELEMENT_SP_DISK_YELLOW                 = EL_SP_DISK_YELLOW;
    static const int ELEMENT_SP_TERMINAL                    = EL_SP_TERMINAL;
    static const int ELEMENT_SP_DISK_RED                    = EL_SP_DISK_RED;
    static const int ELEMENT_SP_PORT_VERTICAL               = EL_SP_PORT_VERTICAL;
    static const int ELEMENT_SP_PORT_HORIZONTAL             = EL_SP_PORT_HORIZONTAL;
    static const int ELEMENT_SP_PORT_ANY                    = EL_SP_PORT_ANY;
    static const int ELEMENT_SP_ELECTRON                    = EL_SP_ELECTRON;
    static const int ELEMENT_SP_BUGGY_BASE                  = EL_SP_BUGGY_BASE;
    static const int ELEMENT_SP_CHIP_LEFT                   = EL_SP_CHIP_LEFT;
    static const int ELEMENT_SP_CHIP_RIGHT                  = EL_SP_CHIP_RIGHT;
    static const int ELEMENT_SP_HARDWARE_BASE_1             = EL_SP_HARDWARE_BASE_1;
    static const int ELEMENT_SP_HARDWARE_GREEN              = EL_SP_HARDWARE_GREEN;
    static const int ELEMENT_SP_HARDWARE_BLUE               = EL_SP_HARDWARE_BLUE;
    static const int ELEMENT_SP_HARDWARE_RED                = EL_SP_HARDWARE_RED;
    static const int ELEMENT_SP_HARDWARE_YELLOW             = EL_SP_HARDWARE_YELLOW;
    static const int ELEMENT_SP_HARDWARE_BASE_2             = EL_SP_HARDWARE_BASE_2;
    static const int ELEMENT_SP_HARDWARE_BASE_3             = EL_SP_HARDWARE_BASE_3;
    static const int ELEMENT_SP_HARDWARE_BASE_4             = EL_SP_HARDWARE_BASE_4;
    static const int ELEMENT_SP_HARDWARE_BASE_5             = EL_SP_HARDWARE_BASE_5;
    static const int ELEMENT_SP_HARDWARE_BASE_6             = EL_SP_HARDWARE_BASE_6;
    static const int ELEMENT_SP_CHIP_TOP                    = EL_SP_CHIP_TOP;
    static const int ELEMENT_SP_CHIP_BOTTOM                 = EL_SP_CHIP_BOTTOM;
    static const int ELEMENT_SP_END                         = EL_SP_END;
    
    static const int ELEMENT_EM_GATE_1_GRAY                 = EL_EM_GATE_1_GRAY;
    static const int ELEMENT_EM_GATE_2_GRAY                 = EL_EM_GATE_2_GRAY;
    static const int ELEMENT_EM_GATE_3_GRAY                 = EL_EM_GATE_3_GRAY;
    static const int ELEMENT_EM_GATE_4_GRAY                 = EL_EM_GATE_4_GRAY;
    static const int ELEMENT_EM_DYNAMITE                    = EL_EM_DYNAMITE;
    static const int ELEMENT_EM_DYNAMITE_ACTIVE             = EL_EM_DYNAMITE_ACTIVE;
    static const int ELEMENT_PEARL                          = EL_PEARL;
    static const int ELEMENT_CRYSTAL                        = EL_CRYSTAL;
    static const int ELEMENT_WALL_PEARL                     = EL_WALL_PEARL;
    static const int ELEMENT_WALL_CRYSTAL                   = EL_WALL_CRYSTAL;
    static const int ELEMENT_DC_GATE_WHITE                  = EL_DC_GATE_WHITE;
    static const int ELEMENT_DC_GATE_WHITE_GRAY             = EL_DC_GATE_WHITE_GRAY;
    static const int ELEMENT_DC_KEY_WHITE                   = EL_DC_KEY_WHITE;
    static const int ELEMENT_SHIELD_NORMAL                  = EL_SHIELD_NORMAL;
    static const int ELEMENT_EXTRA_TIME                     = EL_EXTRA_TIME;
    static const int ELEMENT_SWITCHGATE_OPEN                = EL_SWITCHGATE_OPEN;
    static const int ELEMENT_SWITCHGATE_CLOSED              = EL_SWITCHGATE_CLOSED;
    static const int ELEMENT_SWITCHGATE_SWITCH_UP           = EL_SWITCHGATE_SWITCH_UP;
    static const int ELEMENT_SWITCHGATE_SWITCH_DOWN         = EL_SWITCHGATE_SWITCH_DOWN;
    static const int ELEMENT_CONVEYOR_BELT_1_LEFT           = EL_CONVEYOR_BELT_1_LEFT;
    static const int ELEMENT_CONVEYOR_BELT_1_MIDDLE         = EL_CONVEYOR_BELT_1_MIDDLE;
    static const int ELEMENT_CONVEYOR_BELT_1_RIGHT          = EL_CONVEYOR_BELT_1_RIGHT;
    static const int ELEMENT_CONVEYOR_BELT_1_SWITCH_LEFT    = EL_CONVEYOR_BELT_1_SWITCH_LEFT;
    static const int ELEMENT_CONVEYOR_BELT_1_SWITCH_MIDDLE  = EL_CONVEYOR_BELT_1_SWITCH_MIDDLE;
    static const int ELEMENT_CONVEYOR_BELT_1_SWITCH_RIGHT   = EL_CONVEYOR_BELT_1_SWITCH_RIGHT;
    static const int ELEMENT_CONVEYOR_BELT_2_LEFT           = EL_CONVEYOR_BELT_2_LEFT;
    static const int ELEMENT_CONVEYOR_BELT_2_MIDDLE         = EL_CONVEYOR_BELT_2_MIDDLE;
    static const int ELEMENT_CONVEYOR_BELT_2_RIGHT          = EL_CONVEYOR_BELT_2_RIGHT;
    static const int ELEMENT_CONVEYOR_BELT_2_SWITCH_LEFT    = EL_CONVEYOR_BELT_2_SWITCH_LEFT;
    static const int ELEMENT_CONVEYOR_BELT_2_SWITCH_MIDDLE  = EL_CONVEYOR_BELT_2_SWITCH_MIDDLE;
    static const int ELEMENT_CONVEYOR_BELT_2_SWITCH_RIGHT   = EL_CONVEYOR_BELT_2_SWITCH_RIGHT;
    static const int ELEMENT_CONVEYOR_BELT_3_LEFT           = EL_CONVEYOR_BELT_3_LEFT;
    static const int ELEMENT_CONVEYOR_BELT_3_MIDDLE         = EL_CONVEYOR_BELT_3_MIDDLE;
    static const int ELEMENT_CONVEYOR_BELT_3_RIGHT          = EL_CONVEYOR_BELT_3_RIGHT;
    static const int ELEMENT_CONVEYOR_BELT_3_SWITCH_LEFT    = EL_CONVEYOR_BELT_3_SWITCH_LEFT;
    static const int ELEMENT_CONVEYOR_BELT_3_SWITCH_MIDDLE  = EL_CONVEYOR_BELT_3_SWITCH_MIDDLE;
    static const int ELEMENT_CONVEYOR_BELT_3_SWITCH_RIGHT   = EL_CONVEYOR_BELT_3_SWITCH_RIGHT;
    static const int ELEMENT_CONVEYOR_BELT_4_LEFT           = EL_CONVEYOR_BELT_4_LEFT;
    static const int ELEMENT_CONVEYOR_BELT_4_MIDDLE         = EL_CONVEYOR_BELT_4_MIDDLE;
    static const int ELEMENT_CONVEYOR_BELT_4_RIGHT          = EL_CONVEYOR_BELT_4_RIGHT;
    static const int ELEMENT_CONVEYOR_BELT_4_SWITCH_LEFT    = EL_CONVEYOR_BELT_4_SWITCH_LEFT;
    static const int ELEMENT_CONVEYOR_BELT_4_SWITCH_MIDDLE  = EL_CONVEYOR_BELT_4_SWITCH_MIDDLE;
    static const int ELEMENT_CONVEYOR_BELT_4_SWITCH_RIGHT   = EL_CONVEYOR_BELT_4_SWITCH_RIGHT;
    static const int ELEMENT_LANDMINE                       = EL_LANDMINE;
    static const int ELEMENT_LIGHT_SWITCH                   = EL_LIGHT_SWITCH;
    static const int ELEMENT_LIGHT_SWITCH_ACTIVE            = EL_LIGHT_SWITCH_ACTIVE;
    static const int ELEMENT_SIGN_EXCLAMATION               = EL_SIGN_EXCLAMATION;
    static const int ELEMENT_SIGN_RADIOACTIVITY             = EL_SIGN_RADIOACTIVITY;
    static const int ELEMENT_SIGN_STOP                      = EL_SIGN_STOP;
    static const int ELEMENT_SIGN_WHEELCHAIR                = EL_SIGN_WHEELCHAIR;
    static const int ELEMENT_SIGN_PARKING                   = EL_SIGN_PARKING;
    static const int ELEMENT_SIGN_NO_ENTRY                  = EL_SIGN_NO_ENTRY;
    static const int ELEMENT_SIGN_UNUSED_1                  = EL_SIGN_UNUSED_1;
    static const int ELEMENT_SIGN_GIVE_WAY                  = EL_SIGN_GIVE_WAY;
    static const int ELEMENT_SIGN_ENTRY_FORBIDDEN           = EL_SIGN_ENTRY_FORBIDDEN;
    static const int ELEMENT_SIGN_EMERGENCY_EXIT            = EL_SIGN_EMERGENCY_EXIT;
    static const int ELEMENT_SIGN_YIN_YANG                  = EL_SIGN_YIN_YANG;
    static const int ELEMENT_MOLE_LEFT                      = EL_MOLE_LEFT;
    static const int ELEMENT_MOLE_RIGHT                     = EL_MOLE_RIGHT;
    static const int ELEMENT_MOLE_UP                        = EL_MOLE_UP;
    static const int ELEMENT_MOLE_DOWN                      = EL_MOLE_DOWN;
    static const int ELEMENT_STEELWALL_SLIPPERY             = EL_STEELWALL_SLIPPERY;
    static const int ELEMENT_INVISIBLE_SAND                 = EL_INVISIBLE_SAND;
    static const int ELEMENT_SHIELD_DEADLY                  = EL_SHIELD_DEADLY;
    static const int ELEMENT_TIMEGATE_OPEN                  = EL_TIMEGATE_OPEN;
    static const int ELEMENT_TIMEGATE_CLOSED                = EL_TIMEGATE_CLOSED;
    static const int ELEMENT_TIMEGATE_SWITCH_ACTIVE         = EL_TIMEGATE_SWITCH_ACTIVE;
    static const int ELEMENT_TIMEGATE_SWITCH                = EL_TIMEGATE_SWITCH;
    static const int ELEMENT_BALLOON                        = EL_BALLOON;
    static const int ELEMENT_BALLOON_SWITCH_LEFT            = EL_BALLOON_SWITCH_LEFT;
    static const int ELEMENT_BALLOON_SWITCH_RIGHT           = EL_BALLOON_SWITCH_RIGHT;
    static const int ELEMENT_BALLOON_SWITCH_UP              = EL_BALLOON_SWITCH_UP;
    static const int ELEMENT_BALLOON_SWITCH_DOWN            = EL_BALLOON_SWITCH_DOWN;
    static const int ELEMENT_BALLOON_SWITCH_ANY             = EL_BALLOON_SWITCH_ANY;
    static const int ELEMENT_EMC_STEELWALL_1                = EL_EMC_STEELWALL_1;
    static const int ELEMENT_EMC_STEELWALL_2                = EL_EMC_STEELWALL_2;
    static const int ELEMENT_EMC_STEELWALL_3                = EL_EMC_STEELWALL_3;
    static const int ELEMENT_EMC_STEELWALL_4                = EL_EMC_STEELWALL_4;
    static const int ELEMENT_EMC_WALL_1                     = EL_EMC_WALL_1;
    static const int ELEMENT_EMC_WALL_2                     = EL_EMC_WALL_2;
    static const int ELEMENT_EMC_WALL_3                     = EL_EMC_WALL_3;
    static const int ELEMENT_EMC_WALL_4                     = EL_EMC_WALL_4;
    static const int ELEMENT_EMC_WALL_5                     = EL_EMC_WALL_5;
    static const int ELEMENT_EMC_WALL_6                     = EL_EMC_WALL_6;
    static const int ELEMENT_EMC_WALL_7                     = EL_EMC_WALL_7;
    static const int ELEMENT_EMC_WALL_8                     = EL_EMC_WALL_8;
    static const int ELEMENT_TUBE_ANY                       = EL_TUBE_ANY;
    static const int ELEMENT_TUBE_VERTICAL                  = EL_TUBE_VERTICAL;
    static const int ELEMENT_TUBE_HORIZONTAL                = EL_TUBE_HORIZONTAL;
    static const int ELEMENT_TUBE_VERTICAL_LEFT             = EL_TUBE_VERTICAL_LEFT;
    static const int ELEMENT_TUBE_VERTICAL_RIGHT            = EL_TUBE_VERTICAL_RIGHT;
    static const int ELEMENT_TUBE_HORIZONTAL_UP             = EL_TUBE_HORIZONTAL_UP;
    static const int ELEMENT_TUBE_HORIZONTAL_DOWN           = EL_TUBE_HORIZONTAL_DOWN;
    static const int ELEMENT_TUBE_LEFT_UP                   = EL_TUBE_LEFT_UP;
    static const int ELEMENT_TUBE_LEFT_DOWN                 = EL_TUBE_LEFT_DOWN;
    static const int ELEMENT_TUBE_RIGHT_UP                  = EL_TUBE_RIGHT_UP;
    static const int ELEMENT_TUBE_RIGHT_DOWN                = EL_TUBE_RIGHT_DOWN;
    static const int ELEMENT_SPRING                         = EL_SPRING;
    static const int ELEMENT_TRAP                           = EL_TRAP;
    static const int ELEMENT_DX_SUPABOMB                    = EL_DX_SUPABOMB;

    // ---- Custom elements
    static const int ELEMENT_CUSTOM_1                       = EL_CUSTOM_START; 
    static const int ELEMENT_CUSTOM_2                       = (EL_CUSTOM_START + 1); 
    static const int ELEMENT_CUSTOM_3                       = (EL_CUSTOM_START + 2); 
    static const int ELEMENT_CUSTOM_4                       = (EL_CUSTOM_START + 3); 
    static const int ELEMENT_CUSTOM_5                       = (EL_CUSTOM_START + 4); 
    static const int ELEMENT_CUSTOM_6                       = (EL_CUSTOM_START + 5); 
    static const int ELEMENT_CUSTOM_7                       = (EL_CUSTOM_START + 6); 
    static const int ELEMENT_CUSTOM_8                       = (EL_CUSTOM_START + 7); 
    static const int ELEMENT_CUSTOM_9                       = (EL_CUSTOM_START + 8); 
    static const int ELEMENT_CUSTOM_10                      = (EL_CUSTOM_START + 9); 
    static const int ELEMENT_CUSTOM_11                      = (EL_CUSTOM_START + 10); 
    static const int ELEMENT_CUSTOM_12                      = (EL_CUSTOM_START + 11); 
    static const int ELEMENT_CUSTOM_13                      = (EL_CUSTOM_START + 12); 
    static const int ELEMENT_CUSTOM_14                      = (EL_CUSTOM_START + 13); 
    static const int ELEMENT_CUSTOM_15                      = (EL_CUSTOM_START + 14); 
    static const int ELEMENT_CUSTOM_16                      = (EL_CUSTOM_START + 15); 
    static const int ELEMENT_CUSTOM_17                      = (EL_CUSTOM_START + 16); 
    static const int ELEMENT_CUSTOM_18                      = (EL_CUSTOM_START + 17); 
    static const int ELEMENT_CUSTOM_19                      = (EL_CUSTOM_START + 18); 
    static const int ELEMENT_CUSTOM_20                      = (EL_CUSTOM_START + 19); 

    static const int ELEMENT_EM_KEY_1                       = EL_EM_KEY_1;
    static const int ELEMENT_EM_KEY_2                       = EL_EM_KEY_2;
    static const int ELEMENT_EM_KEY_3                       = EL_EM_KEY_3;
    static const int ELEMENT_EM_KEY_4                       = EL_EM_KEY_4;
    static const int ELEMENT_ENVELOPE_1                     = EL_ENVELOPE_1;
    static const int ELEMENT_ENVELOPE_2                     = EL_ENVELOPE_2;
    static const int ELEMENT_ENVELOPE_3	                    = EL_ENVELOPE_3;
    static const int ELEMENT_ENVELOPE_4                     = EL_ENVELOPE_4;

    static const int ELEMENT_TRIGGER_ELEMENT                = EL_TRIGGER_ELEMENT;
    static const int ELEMENT_TRIGGER_PLAYER                 = EL_TRIGGER_PLAYER;
    // SP style elements
    static const int ELEMENT_SP_GRAVITY_ON_PORT_RIGHT       = EL_SP_GRAVITY_ON_PORT_RIGHT;
    static const int ELEMENT_SP_GRAVITY_ON_PORT_DOWN        = EL_SP_GRAVITY_ON_PORT_DOWN;
    static const int ELEMENT_SP_GRAVITY_ON_PORT_LEFT        = EL_SP_GRAVITY_ON_PORT_LEFT;
    static const int ELEMENT_SP_GRAVITY_ON_PORT_UP          = EL_SP_GRAVITY_ON_PORT_UP;
    static const int ELEMENT_SP_GRAVITY_OFF_PORT_RIGHT      = EL_SP_GRAVITY_OFF_PORT_RIGHT;
    static const int ELEMENT_SP_GRAVITY_OFF_PORT_DOWN       = EL_SP_GRAVITY_OFF_PORT_DOWN;
    static const int ELEMENT_SP_GRAVITY_OFF_PORT_LEFT       = EL_SP_GRAVITY_OFF_PORT_LEFT;
    static const int ELEMENT_SP_GRAVITY_OFF_PORT_UP         = EL_SP_GRAVITY_OFF_PORT_UP;
    // EMC style elements
    static const int ELEMENT_BALLOON_SWITCH_NONE            = EL_BALLOON_SWITCH_NONE;
    static const int ELEMENT_EMC_GATE_5                     = EL_EMC_GATE_5;
    static const int ELEMENT_EMC_GATE_6                     = EL_EMC_GATE_6;
    static const int ELEMENT_EMC_GATE_7                     = EL_EMC_GATE_7;
    static const int ELEMENT_EMC_GATE_8                     = EL_EMC_GATE_8;
    static const int ELEMENT_EMC_GATE_5_GRAY                = EL_EMC_GATE_5_GRAY;
    static const int ELEMENT_EMC_GATE_6_GRAY                = EL_EMC_GATE_6_GRAY;
    static const int ELEMENT_EMC_GATE_7_GRAY                = EL_EMC_GATE_7_GRAY;
    static const int ELEMENT_EMC_GATE_8_GRAY                = EL_EMC_GATE_8_GRAY;
    static const int ELEMENT_EMC_KEY_5                      = EL_EMC_KEY_5;
    static const int ELEMENT_EMC_KEY_6                      = EL_EMC_KEY_6;
    static const int ELEMENT_EMC_KEY_7                      = EL_EMC_KEY_7;
    static const int ELEMENT_EMC_KEY_8                      = EL_EMC_KEY_8;
    static const int ELEMENT_EMC_ANDROID                    = EL_EMC_ANDROID;
    static const int ELEMENT_EMC_GRASS                      = EL_EMC_GRASS;
    static const int ELEMENT_EMC_MAGIC_BALL                 = EL_EMC_MAGIC_BALL;
    static const int ELEMENT_EMC_MAGIC_BALL_ACTIVE          = EL_EMC_MAGIC_BALL_ACTIVE;
    static const int ELEMENT_EMC_MAGIC_BALL_SWITCH          = EL_EMC_MAGIC_BALL_SWITCH;
    static const int ELEMENT_EMC_MAGIC_BALL_SWITCH_ACTIVE   = EL_EMC_MAGIC_BALL_SWITCH_ACTIVE;
    static const int ELEMENT_EMC_SPRING_BUMPER              = EL_EMC_SPRING_BUMPER;
    static const int ELEMENT_EMC_PLANT                      = EL_EMC_PLANT;
    static const int ELEMENT_EMC_LENSES                     = EL_EMC_LENSES;
    static const int ELEMENT_EMC_MAGNIFIER                  = EL_EMC_MAGNIFIER;
    static const int ELEMENT_EMC_WALL_9                     = EL_EMC_WALL_9;
    static const int ELEMENT_EMC_WALL_10                    = EL_EMC_WALL_10;
    static const int ELEMENT_EMC_WALL_11                    = EL_EMC_WALL_11;
    static const int ELEMENT_EMC_WALL_12                    = EL_EMC_WALL_12;
    static const int ELEMENT_EMC_WALL_13                    = EL_EMC_WALL_13;
    static const int ELEMENT_EMC_WALL_14                    = EL_EMC_WALL_14;
    static const int ELEMENT_EMC_WALL_15                    = EL_EMC_WALL_15;
    static const int ELEMENT_EMC_WALL_16                    = EL_EMC_WALL_16;
    static const int ELEMENT_EMC_WALL_SLIPPERY_1            = EL_EMC_WALL_SLIPPERY_1;
    static const int ELEMENT_EMC_WALL_SLIPPERY_2            = EL_EMC_WALL_SLIPPERY_2;
    static const int ELEMENT_EMC_WALL_SLIPPERY_3            = EL_EMC_WALL_SLIPPERY_3;
    static const int ELEMENT_EMC_WALL_SLIPPERY_4            = EL_EMC_WALL_SLIPPERY_4;
    static const int ELEMENT_EMC_FAKE_GRASS                 = EL_EMC_FAKE_GRASS;
    static const int ELEMENT_EMC_FAKE_ACID                  = EL_EMC_FAKE_ACID;
    static const int ELEMENT_EMC_DRIPPER                    = EL_EMC_DRIPPER;
    static const int ELEMENT_TRIGGER_CE_VALUE               = EL_TRIGGER_CE_VALUE;
    static const int ELEMENT_TRIGGER_CE_SCORE               = EL_TRIGGER_CE_SCORE;
    static const int ELEMENT_CURRENT_CE_VALUE               = EL_CURRENT_CE_VALUE;
    static const int ELEMENT_CURRENT_CE_SCORE               = EL_CURRENT_CE_SCORE;
    static const int ELEMENT_YAMYAM_LEFT                    = EL_YAMYAM_LEFT;
    static const int ELEMENT_YAMYAM_RIGHT                   = EL_YAMYAM_RIGHT;
    static const int ELEMENT_YAMYAM_UP                      = EL_YAMYAM_UP;
    static const int ELEMENT_YAMYAM_DOWN                    = EL_YAMYAM_DOWN;
    static const int ELEMENT_BD_EXPANDABLE_WALL             = EL_BD_EXPANDABLE_WALL;
    static const int ELEMENT_PREV_CE_8                      = EL_PREV_CE_8;
    static const int ELEMENT_PREV_CE_7                      = EL_PREV_CE_7;
    static const int ELEMENT_PREV_CE_6                      = EL_PREV_CE_6;
    static const int ELEMENT_PREV_CE_5                      = EL_PREV_CE_5;
    static const int ELEMENT_PREV_CE_4                      = EL_PREV_CE_4;
    static const int ELEMENT_PREV_CE_3                      = EL_PREV_CE_3;
    static const int ELEMENT_PREV_CE_2                      = EL_PREV_CE_2;
    static const int ELEMENT_PREV_CE_1                      = EL_PREV_CE_1;
    static const int ELEMENT_SELF                           = EL_SELF;
    static const int ELEMENT_NEXT_CE_1                      = EL_NEXT_CE_1;
    static const int ELEMENT_NEXT_CE_2                      = EL_NEXT_CE_2;
    static const int ELEMENT_NEXT_CE_3                      = EL_NEXT_CE_3;
    static const int ELEMENT_NEXT_CE_4                      = EL_NEXT_CE_4;
    static const int ELEMENT_NEXT_CE_5                      = EL_NEXT_CE_5;
    static const int ELEMENT_NEXT_CE_6                      = EL_NEXT_CE_6;
    static const int ELEMENT_NEXT_CE_7                      = EL_NEXT_CE_7;
    static const int ELEMENT_NEXT_CE_8                      = EL_NEXT_CE_8;
    static const int ELEMENT_ANY_ELEMENT                    = EL_ANY_ELEMENT;



} // namespace enginetype



#endif  //ENGINE_TYPES_H