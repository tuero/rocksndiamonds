/**
 * @file: engine_helper.cpp
 *
 * @brief: Interface for controllers to access information about the engine state.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */

#include "engine_helper.h"

extern int spriteIDs[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int spriteIDCounter;

namespace enginehelper {

// Hashing data structures
uint64_t zobristElement[MAX_LEV_FIELDX * MAX_LEV_FIELDY][MAX_NUM_ELEMENTS];
uint64_t zobristDir[MAX_LEV_FIELDX * MAX_LEV_FIELDY][MAX_DIR];

auto rng = std::default_random_engine {};



/*
 * Get the controller type defined by user command line argument.
 */
enginetype::ControllerType getControllerType() {
    return static_cast<enginetype::ControllerType>(options.controller_type);
}

/*
 * Get the replay file name.
 */
std::string getReplayFileName() {
    std::string replay_file(options.replay_file);
    return replay_file;
}


// -------------------------------------------------------
// -------------------Level Information-------------------
// -------------------------------------------------------


/*
 * Call engine functions to automatically load the levelset.
 */
void setLevelSet() {
    // Need to load levelset before openall
    // Maybe get setLevelset to check if replay, then get the levelset from there?
    // Kind of messy, need a better

    // No levelset given
    if (options.level_set == NULL) {return;}

    std::string level_set(options.level_set);

    try{
        // Initialize leveldir_current and related objects
        LoadLevelInfo();

        PLOGI_(logwrap::FileLogger) << "Setting levelset " << level_set;

        // Set levelset to save
        leveldir_current->fullpath = options.level_set;
        leveldir_current->subdir = options.level_set;
        leveldir_current->identifier = options.level_set;

        // Save the levelset
        // We save because on startup, the previously saved levelset is loaded
        SaveLevelSetup_LastSeries();
        OpenAll(); 
    }
    catch (...){
        PLOGE_(logwrap::FileLogger) << "Something went wrong trying to load levelset " << level_set;
        PLOGE_(logwrap::ConsolLogger) << "Something went wrong trying to load levelset " << level_set;
        CloseAllAndExit(1);
    }
}

/*
 * Get the levelset currently set in the engine.
 */
std::string getLevelSet() {
    std::string level_set(leveldir_current->subdir);
    return level_set;
}


/*
 * Call engine function to load the given level.
 */
void loadLevel(int level_num) {
    options.level_number = level_num;
    PLOGI_(logwrap::FileLogger) << "Loading level " << level_num;
    LoadLevel(level_num);
}


/*
 * Get the level number from command line argument.
 */
int getLevelNumber() {
    return options.level_number;
}


/*
 * Get the level height of the level currently loaded in the engine.
 */
int getLevelHeight() {
    return level.fieldy;
}


/*
 * Get the level width of the level currently loaded in the engine.
 */
int getLevelWidth() {
    return level.fieldx;
}


// -------------------------------------------------------
// -----------------Map Item Information------------------
// -------------------------------------------------------


void initSpriteIDs() {
    spriteIDCounter = 0;
    enginetype::GridCell playerCell = getPlayerPosition();

    for (int x = 0; x < level.fieldx; x++) {
        for (int y = 0; y < level.fieldy; y++) {
            spriteIDs[x][y] = -1;
            if (Feld[x][y] > 1 && Feld[x][y] != enginetype::FIELD_TEMP &&
                !(x == playerCell.x && y == playerCell.y)) 
            {
                spriteIDs[x][y] = spriteIDCounter++;
                // spriteIDCounter += 1;
            }
        }
    }
}


int getSpriteID(enginetype::GridCell cell) {
    if (cell.x < 0 || cell.x >= level.fieldx || cell.y < 0 || cell.y >= level.fieldy) {
        PLOGE_(logwrap::FileLogger) << "Position out of bounds.";
        return -1;
    }
    return spriteIDs[cell.x][cell.y];
}


enginetype::GridCell getSpriteGridCell(int spriteID) {
    enginetype::GridCell cell = {-1, -1};
    for (int x = 0; x < level.fieldx; x++) {
        for (int y = 0; y < level.fieldy; y++) {
            if (spriteIDs[x][y] == spriteID) {
                cell.x = x;
                cell.y = y;
                break;
            }
        }
    }
    return cell;
}


bool isSpriteActive(int spriteID) {
    if (spriteID == -1) {return true;}
    for (int x = 0; x < level.fieldx; x++) {
        for (int y = 0; y < level.fieldy; y++) {
            if (spriteIDs[x][y] == spriteID) {return true;}
        }
    }
    return false;
}


/*
 * Get the item located at the given GridCell (x,y) location.
 */
int getGridItem(enginetype::GridCell cell) {
    if (cell.x < 0 || cell.x >= level.fieldx || cell.y < 0 || cell.y >= level.fieldy) {
        PLOGE_(logwrap::FileLogger) << "Position out of bounds.";
        return enginetype::FIELD_EMPTY;
    }

    return Feld[cell.x][cell.y];
}


/*
 * Get the item MovPos at the given GridCell (x,y) location.
 */
int getGridMovPos(enginetype::GridCell cell) {
    if (cell.x < 0 || cell.x >= level.fieldx || cell.y < 0 || cell.y >= level.fieldy) {
        PLOGE_(logwrap::FileLogger) << "Position out of bounds.";
        return 0;
    }

    return MovPos[cell.x][cell.y];
}


/*
 * Get the grid cell that the player is currently located in.
 */
enginetype::GridCell getPlayerPosition() {
    enginetype::GridCell gridCell{stored_player[0].jx, stored_player[0].jy};
    return gridCell;
}

/*
 * Get the sprite locations on the map.
 */
std::vector<enginetype::GridCell> getMapSprites() {
    std::vector<enginetype::GridCell> mapSprites;
    enginetype::GridCell playerCell = getPlayerPosition();

    for (int x = 0; x < level.fieldx; x++) {
        for (int y = 0; y < level.fieldy; y++) {
            if (Feld[x][y] > 1 && Feld[x][y] != enginetype::FIELD_TEMP &&
                !(x == playerCell.x && y == playerCell.y)) 
            {
                mapSprites.push_back({x,y});
            }
        }
    }

    return mapSprites;
}


/*
 * Get the current goal location (defined by distance of 0).
 */
enginetype::GridCell getCurrentGoalLocation() {
    enginetype::GridCell gridCell{-1, -1};
    for (int x = 0; x < level.fieldx; x++) {
        for (int y = 0; y < level.fieldy; y++) {
            if (distances[x][y] == 0) {gridCell.x = x; gridCell.y = y;}
        }
    }
    return gridCell;
}


/*
 * Check if the grid cell at location (x,y) is empty.
 */
bool isGridEmpty(enginetype::GridCell cell) {
    // Check bounds (this shouldn't happen but best to be safe)
    if (cell.x < 0 || cell.x >= level.fieldx || cell.y < 0 || cell.y >= level.fieldy) {
        PLOGE_(logwrap::FileLogger) << "Location (" << cell.x << "," << cell.y << ") is out of bounds.";
        return false;
    }

    // Player position is not indicated in Feld, so check first
    if (stored_player[0].jx == cell.x && stored_player[0].jy == cell.y) {
        return false;
    }

    // Element is currently attempting to move to grid (x,y)
    if (MovDir[cell.x][cell.y] != 0) {
        return false;
    }

    // Grid now empty if Feld is empty
    return Feld[cell.x][cell.y] == enginetype::FIELD_EMPTY;
}


/*
 * Checks if the direction the player wants to move in is a wall.
 */
bool isWall(Action action, enginetype::GridCell playerCell) {
    int playerX = playerCell.x;
    int playerY = playerCell.y;

    if (playerX == -1 || playerY == -1) {
        playerX = stored_player[0].jx;
        playerY = stored_player[0].jy;
    }

    if (action == Action::down && (Feld[playerX][playerY+1] == enginetype::FIELD_WALL || 
        playerY+1 == level.fieldy))
    {
        return true;
    }
    else if (action == Action::right && (Feld[playerX+1][playerY] == enginetype::FIELD_WALL || 
        playerX+1 == level.fieldx))
    {
        return true;
    }
    else if (action == Action::up && (Feld[playerX][playerY-1] == enginetype::FIELD_WALL || 
        playerY == 0))
    {
        return true;
    }
    else if (action == Action::left && (Feld[playerX-1][playerY] == enginetype::FIELD_WALL || 
        playerX == 0)) 
    {
        return true;
    }
    return false;
}


/*
 * Checks if the direction the player wants to move in is walkable.
 */
bool isWalkable(Action action, enginetype::GridCell playerCell) {
    int playerX = playerCell.x;
    int playerY = playerCell.y;

    if (playerX == -1 || playerY == -1) {
        playerX = stored_player[0].jx;
        playerY = stored_player[0].jy;
    }

    if (action == Action::down && playerY+1 < level.fieldy) {
        if (enginetype::TYPE_IS_WALKABLE.find(Feld[playerX][playerY+1]) != enginetype::TYPE_IS_WALKABLE.end()) {
            return enginetype::TYPE_IS_WALKABLE[Feld[playerX][playerY+1]];
        }
        // return (Feld[playerX][playerY+1] == enginetype::FIELD_EMPTY || Feld[playerX][playerY+1] == enginetype::FIELD_DIRT || Feld[playerX][playerY+1] == enginetype::FIELD_DIAMOND);
    }
    else if (action == Action::right && playerX+1 < level.fieldx) {
        if (enginetype::TYPE_IS_WALKABLE.find(Feld[playerX+1][playerY]) != enginetype::TYPE_IS_WALKABLE.end()) {
            return enginetype::TYPE_IS_WALKABLE[Feld[playerX+1][playerY]];
        }
        // return (Feld[playerX+1][playerY] == enginetype::FIELD_EMPTY || Feld[playerX+1][playerY] == enginetype::FIELD_DIRT || Feld[playerX+1][playerY] == enginetype::FIELD_DIAMOND);
    }
    else if (action == Action::up && playerY-1 >= 0) {
        if (enginetype::TYPE_IS_WALKABLE.find(Feld[playerX][playerY-1]) != enginetype::TYPE_IS_WALKABLE.end()) {
            return enginetype::TYPE_IS_WALKABLE[Feld[playerX][playerY-1]];
        }
        // return (Feld[playerX][playerY-1] == enginetype::FIELD_EMPTY || Feld[playerX][playerY-1] == enginetype::FIELD_DIRT || Feld[playerX][playerY-1] == enginetype::FIELD_DIAMOND);
    }
    else if (action == Action::left && playerX-1 >= 0) {
        if (enginetype::TYPE_IS_WALKABLE.find(Feld[playerX-1][playerY]) != enginetype::TYPE_IS_WALKABLE.end()) {
            return enginetype::TYPE_IS_WALKABLE[Feld[playerX-1][playerY]];
        }
        // return (Feld[playerX-1][playerY] == enginetype::FIELD_EMPTY || Feld[playerX-1][playerY] == enginetype::FIELD_DIRT || Feld[playerX-1][playerY] == enginetype::FIELD_DIAMOND);
    }
    return false;
}


/*
 * Helper function to check if cell is in bounds and in allowed area.
 *
 * @param playerCell The GridCell which the agent wants to be in
 * @param playerBounds The map bounds for the agent given its direction
 * @param mapBounds The 
 */
// bool _canExpandDirection(enginetype::GridCell playerCell, int playerBounds, int mapBounds, 
//     std::vector<enginetype::GridCell> &allowedCells) 
bool _canExpandDirection(enginetype::GridCell playerCell, std::vector<enginetype::GridCell> &allowedCells) 
{
    // Check if direction is blocked by map mounds/wall
    // if (Feld[playerCell.x][playerCell.y] == enginetype::FIELD_WALL || playerBounds == mapBounds) {
    //     return false;
    // }
    if (Feld[playerCell.x][playerCell.y] == enginetype::FIELD_WALL) {
        return false;
    }

    // Check if direction is in allowed cells
    for (auto const & cell : allowedCells) {
        if (cell.x == playerCell.x && cell.y == playerCell.y) {
            return true;
        }
    }
    return false;
}


/*
 * Checks if action is valid given restricted GridCells player is allowed in.
 */
bool canExpand(Action action, std::vector<enginetype::GridCell> &allowedCells) {
    int playerX = stored_player[0].jx;
    int playerY = stored_player[0].jy;

    if (action == Action::down) {
        // return _canExpandDirection({playerX, playerY+1}, playerY+1, level.fieldy, allowedCells);
        return _canExpandDirection({playerX, playerY+1}, allowedCells);
    }
    else if (action == Action::up) {
        // return _canExpandDirection({playerX, playerY-1}, playerY, 0, allowedCells);
        return _canExpandDirection({playerX, playerY-1}, allowedCells);
    }
    else if (action == Action::right) {
        // return _canExpandDirection({playerX+1, playerY}, playerX+1, level.fieldx, allowedCells);
        return _canExpandDirection({playerX+1, playerY}, allowedCells);
    }
    else if (action == Action::left) {
        // return _canExpandDirection({playerX-1, playerY}, playerX, 0, allowedCells);
        return _canExpandDirection({playerX-1, playerY}, allowedCells);
    }

    // Otherwise, we have a noop which is always allowed
    return true;
}


/*
 * Check if two grid cells are neighbours, defined by being separated by Euclidean distance of 1.
 */
bool checkIfNeighbours(enginetype::GridCell left, enginetype::GridCell right) {
    if (left.x == right.x && std::abs(left.y - right.y) == 1) {return true;}
    if (left.y == right.y && std::abs(left.x - right.x) == 1) {return true;}
    return false;
}


/*
 * Get the action which moves from the first given grid cell to the second. 
 *
 * Note: The gridcells must be neighbours.
 */
Action getResultingAction(enginetype::GridCell from, enginetype::GridCell to) {
    if (!checkIfNeighbours(from, to)) {
        PLOGE_(logwrap::FileLogger) << "Cells are not neighbours.";
    }

    Action action = Action::noop;
    if (from.y == to.y && (from.x - to.x) == -1) {
        action = Action::right;
    }
    else if (from.y == to.y && (from.x - to.x) == 1) {
        action = Action::left;
    }
    else if (from.x == to.x && (from.y - to.y) == -1) {
        action = Action::down;
    }
    else if (from.x == to.x && (from.y - to.y) == 1) {
        action = Action::up;
    }

    return action;
}


// -------------------------------------------------------
// ---------------Custom Level Programming----------------
// -------------------------------------------------------

/*
 * Count how many of a specified element in the game.
 */
int countNumOfElement(int element) {
    int elementCount = 0;
    for (int y = 0; y < level.fieldy; y++) {
        for (int x = 0; x < level.fieldx; x++) {
            if (Feld[x][y] == element) {
                elementCount += 1;
            }
        }
    }
    return elementCount;
}


/*
 * Add the specified element to the level.
 */
void spawnElement(int element, int dir, enginetype::GridCell gridCell) {
    Feld[gridCell.x][gridCell.y] = element;
    MovDir[gridCell.x][gridCell.y] = dir;
}


/*
 * Get all grid cells which are empty.
 */
std::vector<enginetype::GridCell> getEmptyGridCells() {
    std::vector<enginetype::GridCell> emptyGridCells;
    for (int y = 0; y < level.fieldy; y++) {
        for (int x = 0; x < level.fieldx; x++) {
            if (isGridEmpty({x, y})) {
                emptyGridCells.push_back(enginetype::GridCell{x, y});
            }
        }
    }

    return emptyGridCells;
}


// -------------------------------------------------------
// -----------------Game Engine State---------------------
// -------------------------------------------------------


/*
 * Check if the current status of the engine is loss of life.
 */
bool engineGameFailed() {
    return checkGameFailed();
}


/*
 * Check if the current status of the engine is level solved.
 */
bool engineGameSolved() {
    // return checkGameSolved();
    return (game.LevelSolved && !game.LevelSolved_GameEnd);
}


/*
 * Check if the current status of the engine is failed or solved
 */
bool engineGameOver() {
    return engineGameFailed() || engineGameSolved();
}


/*
 * Set the action for the engine to perform on behalf of the player on the next iteration.
 */
void setEnginePlayerAction(Action action) {
    stored_player[0].action = action;
}


/*
 * Set the stored player's action as a valid random action.
 */
void setEngineRandomPlayerAction() {
    std::vector<Action> availableActions;

    int playerX = stored_player[0].jx;
    int playerY = stored_player[0].jy;

    // Won't select actions which player won't be able to move due
    // to being blocked by walls
    availableActions.push_back(Action::noop);
    if (Feld[playerX][playerY+1] != enginetype::FIELD_WALL) {
        availableActions.push_back(Action::down);
    }
    else if (Feld[playerX+1][playerY] != enginetype::FIELD_WALL) {
        availableActions.push_back(Action::right);
    }
    else if (Feld[playerX][playerY-1] != enginetype::FIELD_WALL) {
        availableActions.push_back(Action::up);
    }
    else if (Feld[playerX-1][playerY] != enginetype::FIELD_WALL) {
        availableActions.push_back(Action::left);
    }

    // Linear complexity but guaranteed max size is 5
    std::shuffle(std::begin(availableActions), std::end(availableActions), rng);
    
    // There is always at least 1 action (NOOP), so should be safe.
    stored_player[0].action = availableActions[0];
}


/*
 * Get the currently stored player action.
 */
int getEnginePlayerAction() {
    return stored_player[0].action;
}


/*
 * Get the current score of the game.
 */
int getCurrentScore() {
    return game.score;
}


/*
 * Get the Time left in the game.
 */
int getTimeLeftScore() {
    return TimeLeft;
}


/*
 * Simulate the engine ahead a single tick.
 */
void engineSimulateSingle() {
    HandleGameActions();
}


/*
 * Simulate the engine ahead ENGINE_RESOLUTION game ticks.
 */
void engineSimulate() {
    for (int i = 0; i < enginetype::ENGINE_RESOLUTION; i++) {
        HandleGameActions();
    }
}

/*
 * Set flag for simulating.
 */
void setSimulatorFlag(bool simulatorFlag) {
    is_simulating = simulatorFlag;
}


/*
 * Get the simulator flag status.
 */
bool isSimulating() {
    return is_simulating;
}


// -------------------------------------------------------
// -------------------State Hashing-----------------------
// -------------------------------------------------------


/*
 * Initialize Zorbrist tables, used to hash game board states
 */
void initZorbristTables() {
    for (int i = 0; i < MAX_LEV_FIELDX*MAX_LEV_FIELDY; i++) {
        // K is item type
        for (int k = 0; k < MAX_NUM_ELEMENTS; k++) {
            zobristElement[i][k] = RNG::getRandomNumber();
        }
        for (int k = 0; k < MAX_DIR; k++) {
            zobristDir[i][k] = RNG::getRandomNumber();
        }
    }
}


/*
 * Get the hash representation of the current state in the engine
 */
uint64_t stateToHash() {
    int px = stored_player[0].jx;
    int py = stored_player[0].jy;
    int pMov = stored_player[0].MovDir;
    uint64_t hashValue = 0; 

    // Set initial hash
    for (int x = 0; x < level.fieldx; x++) {
        for (int y = 0; y < level.fieldy; y++) {
            hashValue ^= zobristElement[y*level.fieldx + x][Feld[x][y]];
            hashValue ^= zobristDir[y*level.fieldx + x][MovDir[x][y]];
        }
    }

    hashValue ^= zobristElement[py*level.fieldx + px][80];
    hashValue ^= zobristDir[py*level.fieldx + px][pMov];

    return hashValue;
}


// -------------------------------------------------------
// ----------------Distance Functions---------------------
// -------------------------------------------------------


/*
 * Get L1 distance between two gridcells
 */
int getL1Distance(enginetype::GridCell left, enginetype::GridCell right) {
    return (std::abs(left.x - right.x) + std::abs(left.y - right.y));
}


/*
 * Get the minimum player distance in reference to given list of gridcells
 */
int minDistanceToAllowedCells(std::vector<enginetype::GridCell> &goal_cells) {
    int playerX = stored_player[0].jx;
    int playerY = stored_player[0].jy;
    int min_distance = std::numeric_limits<int>::max();

    PLOGE_IF_(logwrap::FileLogger, goal_cells.empty()) << "No goal nodes.";
    assert(goal_cells.size() > 0);

    for (auto const & grid : goal_cells) {
        int distance = std::abs(grid.x - playerX) + std::abs(grid.y - playerY);
        if (distance < min_distance) {min_distance = distance;}
    }

    return min_distance;
}


// Data types/structures for Dijkstra
typedef std::array<int, 2> Point;
short INF = std::numeric_limits<short>::max();
short distances[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short abstract_node_distances[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short max_distance = -1;

/*
 * Get the players current shortest path distance to goal
 * This uses distance tile maps pre-calculated using Dijkstra algorithm,
 * NOT Euclidean distance.
 */
float getPlayerDistanceToGoal() {
    int playerX = stored_player[0].jx;
    int playerY = stored_player[0].jy;
    return (float)distances[playerX][playerY];
}


/*
 * Get the distance to goal from given gridcell
 * This is set by distance metric, usually L1
 */
int getGridDistanceToGoal(enginetype::GridCell gridCell) {
    return distances[gridCell.x][gridCell.y];
}


/*
 * Get the player distance to the next abstract node
 * Internal abstract node distances are set by Dijsktra, used to help
 * player get around corners that fails by L1 shortest distance
 */
int getPlayerDistanceToNextNode() {
    int playerX = stored_player[0].jx;
    int playerY = stored_player[0].jy;
    return abstract_node_distances[playerX][playerY];
}


/*
 * Find the next min index point from point queue Q
 */
void _getMinDistanceIndex(std::vector<Point> &Q, int &index) {
    int min_index = -1;
    short min_value = INF;
    for (int i = 0; i < (int)Q.size(); i++) {
        int x = Q[i][0]; 
        int y = Q[i][1];
        if (distances[x][y] <= min_value) {
            min_index = i;
            min_value = distances[x][y];
        }
    }
    index = min_index;
}

void _getMinDistanceIndexAbstract(std::vector<Point> &Q, int &index) {
    int min_index = -1;
    short min_value = INF;
    for (int i = 0; i < (int)Q.size(); i++) {
        int x = Q[i][0]; 
        int y = Q[i][1];
        if (abstract_node_distances[x][y] <= min_value) {
            min_index = i;
            min_value = abstract_node_distances[x][y];
        }
    }
    index = min_index;
}


/*
 * Get the neighbours of point u from queue Q and insert into neighbours
 */
void _getNeighboursDijkstra(Point u, std::vector<Point> &neighbours, std::vector<Point> &Q) {
    int x = u[0];
    int y = u[1];

    if (x-1 >= 0 && Feld[x-1][y] != enginetype::FIELD_WALL 
        && std::find(Q.begin(), Q.end(), (Point){x-1,y}) != Q.end()) 
    {
        neighbours.push_back((Point){x-1,y});
    }
    if (x+1 < level.fieldx && Feld[x+1][y] != enginetype::FIELD_WALL && 
        std::find(Q.begin(), Q.end(), (Point){x+1,y}) != Q.end()) 
    {
        neighbours.push_back((Point){x+1,y});
    }

    if (y-1 >= 0 && Feld[x][y-1] != enginetype::FIELD_WALL 
        && std::find(Q.begin(), Q.end(), (Point){x,y-1}) != Q.end()) 
    {
        neighbours.push_back((Point){x,y-1});
    }
    if (y+1 < level.fieldy && Feld[x][y+1] != enginetype::FIELD_WALL && 
        std::find(Q.begin(), Q.end(), (Point){x,y+1}) != Q.end()) 
    {
        neighbours.push_back((Point){x,y+1});
    }
}


/*
 * Find the grid location of the goal, given by enginetype::FIELD_EXIT
 */
enginetype::GridCell findExitLocation() {
    enginetype::GridCell goal_cell = {-1, -1};
    for (int y = 0; y < level.fieldy; y++) {
        for (int x = 0; x < level.fieldx; x++) {
            if (Feld[x][y] == enginetype::FIELD_EXIT) {
                goal_cell.x = x;
                goal_cell.y = y;
                PLOGI_(logwrap::FileLogger) << "Found goal at x=" << x << ", y=" << y << ".";
                return goal_cell;
            }
        }
    }

    return goal_cell;
}


/*
 * Set the grid distances to goal using Dijkstra's algorithm (shortest path)
 */
void setBoardDistancesDijkstra(enginetype::GridCell goal_cell) {
    int x, y;
    std::vector<Point> Q;       // Queue of points 

    PLOGI_(logwrap::FileLogger) << "Setting board distances.";

    // Initialize distances
    for (y = 0; y < level.fieldy; y++) {
        for (x = 0; x < level.fieldx; x++) {
            distances[x][y] = INF;
            if (Feld[x][y] != enginetype::FIELD_WALL) {Q.push_back({x, y});}
        }
    }

    // If no goal, then break
    if (goal_cell.x == -1 || goal_cell.y == -1) {
        PLOGI_(logwrap::FileLogger) << "Level has no goal.";
        return;
    }

    // Check goal in bounds
    if (goal_cell.x < 0 || goal_cell.x >= level.fieldx || goal_cell.y < 0 || goal_cell.y >= level.fieldy) {
        PLOGI_(logwrap::FileLogger) << "Provided goal is out of level bounds.";
        return;
    }

    // Set goal distance
    distances[goal_cell.x][goal_cell.y] = 0;

    // Calc distances
    Point u;
    int index;
    while (!Q.empty()) {
        // Get min distance from vertex set and remove
        _getMinDistanceIndex(Q, index);
        u = Q[index];
        Q.erase(Q.begin() + index);

        // Get neighbours
        std::vector<Point> neighbours;
        _getNeighboursDijkstra(u, neighbours, Q);

        // For each neighbour, update distance
        for (Point v : neighbours) {
            int alt = distances[u[0]][u[1]] + 1;
            if (alt < distances[v[0]][v[1]]) {
                distances[v[0]][v[1]] = alt;
            }
        }
    }

    // Set max distances to neg
    for (y = 0; y < level.fieldy; y++) {
        for (x = 0; x < level.fieldx; x++) {
            distances[x][y] = (distances[x][y] == INF ? -1 : distances[x][y]);
            max_distance = (max_distance < distances[x][y]) ? distances[x][y] : max_distance;
        }
    }
}

/*
 * Set the grid distances to goal using L1 distance
 */
void setBoardDistancesL1(enginetype::GridCell goal_cell) {
    PLOGI_(logwrap::FileLogger) << "Setting board distances.";

    if (goal_cell.x == -1 && goal_cell.y == -1) {
        goal_cell = findExitLocation();
    }

    // Initialize distances
    max_distance = -1;
    for (int y = 0; y < level.fieldy; y++) {
        for (int x = 0; x < level.fieldx; x++) {
            distances[x][y] = -1;
        }
    }

    // If no goal, then break
    if (goal_cell.x == -1 || goal_cell.y == -1) {
        PLOGI_(logwrap::FileLogger) << "Level has no goal.";
        return;
    }

    // Check goal in bounds
    if (goal_cell.x < 0 || goal_cell.x >= level.fieldx || goal_cell.y < 0 || goal_cell.y >= level.fieldy) {
        PLOGI_(logwrap::FileLogger) << "Provided goal is out of level bounds.";
        return;
    }

    // Set goal distance
    distances[goal_cell.x][goal_cell.y] = 0;

    // Set other grid distances
    for (int y = 0; y < level.fieldy; y++) {
        for (int x = 0; x < level.fieldx; x++) {
            if (Feld[x][y] != enginetype::FIELD_WALL) {
                distances[x][y] = std::abs(goal_cell.x - x) + std::abs(goal_cell.y - y);
                max_distance = (max_distance < distances[x][y]) ? distances[x][y] : max_distance;
            }
        }
    }
}


/*
 * Used in PFA_MCST
 * Set the internal grid cell distances to goal in abstract node
 * This helps MCTS get around corners that fails with just L1
 */
void setAbstractNodeDistances(std::vector<enginetype::GridCell> goal_cells,
    std::vector<enginetype::GridCell> allowedCells) 
{
    int x, y;
    std::vector<Point> Q;       // Queue of points 

    // Initialize distances
    for (y = 0; y < level.fieldy; y++) {
        for (x = 0; x < level.fieldx; x++) {
            abstract_node_distances[x][y] = INF;
        }
    }
    for (auto const & allowed_cell : allowedCells) {
        if (std::find(goal_cells.begin(), goal_cells.end(), allowed_cell) != goal_cells.end()) {
            continue;
        }
        Q.push_back({allowed_cell.x, allowed_cell.y});
    }

    // If no goal, then break
    if (goal_cells.empty()) {
        PLOGI_(logwrap::FileLogger) << "Level has no goal.";
        return;
    }

    // Set goal distance
    for (auto const & goal_cell : goal_cells) {
        abstract_node_distances[goal_cell.x][goal_cell.y] = 0;
        Q.push_back({goal_cell.x, goal_cell.y});
    }

    // Calc distances
    Point u;
    int index;
    while (!Q.empty()) {
        // Get min distance from vertex set and remove
        _getMinDistanceIndexAbstract(Q, index);
        u = Q[index];
        Q.erase(Q.begin() + index);

        // Get neighbours
        std::vector<Point> neighbours;
        _getNeighboursDijkstra(u, neighbours, Q);

        // For each neighbour, update distance
        for (Point v : neighbours) {
            int alt = abstract_node_distances[u[0]][u[1]] + 1;
            if (alt < abstract_node_distances[v[0]][v[1]]) {
                abstract_node_distances[v[0]][v[1]] = alt;
            }
        }
    }

    // Set max distances to neg
    for (y = 0; y < level.fieldy; y++) {
        for (x = 0; x < level.fieldx; x++) {
            abstract_node_distances[x][y] = (abstract_node_distances[x][y] == INF ? -1 : abstract_node_distances[x][y]);
            max_distance = (max_distance < abstract_node_distances[x][y]) ? abstract_node_distances[x][y] : max_distance;
        }
    }
}

}