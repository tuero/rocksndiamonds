

#include "engine_helper.h"

namespace enginehelper {


// Hashing data structures
uint64_t zobristElement[MAX_LEV_FIELDX * MAX_LEV_FIELDY][MAX_NUM_ELEMENTS];
uint64_t zobristDir[MAX_LEV_FIELDX * MAX_LEV_FIELDY][MAX_DIR];



/*
 * Get the controller type defined by user CLA
 */
enginetype::ControllerType getControllerType() {
    return static_cast<enginetype::ControllerType>(options.controller_type);
}


/*
 * Call engine functions to load the levelset
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
    }
}


/*
 * Call engine function to load the given level
 */
void loadLevel(int level_num) {
    // Other functions replay on options being set
    options.level_number = level_num;
    PLOGI_(logwrap::FileLogger) << "Loading level " << level_num;
    LoadLevel(level_num);
}


/*
 * Get the level number from command line argument
 */
int getLevelNumber() {
    return options.level_number;
}


/*
 * Get the levelset used
 */
std::string getLevelSet() {
    std::string level_set(leveldir_current->subdir);
    return level_set;
}


/*
 * Get the level height
 */
int getLevelHeight() {
    return level.fieldy;
}


/*
 * Get the level width
 */
int getLevelWidth() {
    return level.fieldx;
}


/*
 * Get the player grid position
 */
enginetype::GridCell getPlayerPosition() {
    enginetype::GridCell grid_cell{stored_player[0].jx, stored_player[0].jy};
    return grid_cell;
}


/*
 * Get the current goal location (defined by distance of 0)
 */
enginetype::GridCell getCurrentGoalLocation() {
    enginetype::GridCell grid_cell{-1, -1};
    for (int x = 0; x < level.fieldx; x++) {
        for (int y = 0; y < level.fieldy; y++) {
            if (distances[x][y] == 0) {grid_cell.x = x; grid_cell.y = y;}
        }
    }
    return grid_cell;
}


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
 * Set the action for the engine to perform on behalf of the player on
 * the next iteration as a random action
 */
void setEngineRandomPlayerAction() {
    std::vector<Action> available_actions;

    int playerX = stored_player[0].jx;
    int playerY = stored_player[0].jy;

    // Won't select actions which player won't be able to move due
    // to being blocked by walls
    available_actions.push_back(Action::noop);
    if (Feld[playerX][playerY+1] != enginetype::FIELD_WALL) {
        available_actions.push_back(Action::down);
    }
    else if (Feld[playerX+1][playerY] != enginetype::FIELD_WALL) {
        available_actions.push_back(Action::right);
    }
    else if (Feld[playerX][playerY-1] != enginetype::FIELD_WALL) {
        available_actions.push_back(Action::up);
    }
    else if (Feld[playerX-1][playerY] != enginetype::FIELD_WALL) {
        available_actions.push_back(Action::left);
    }
    
    stored_player[0].action = available_actions[std::rand() % available_actions.size()];
}


/*
 * Get the currently stored player action
 */
int getEnginePlayerAction() {
    return stored_player[0].action;
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
    for (int i = 0; i < enginetype::ENGINE_RESOLUTION; i++) {
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
 * Check if the grid cell at location (x,y) is empty
 */
bool isGridEmpty(int x, int y) {
    // Check bounds (this shouldn't happen but best to be safe)
    if (x < 0 || x >= level.fieldx || y < 0 || y >= level.fieldy) {
        // PLOGE_(logwrap::FileLogger) << "Index out of bounds.";
        return false;
    }

    // Player position is not indicated in Feld, so check first
    if (stored_player[0].jx == x && stored_player[0].jy == y) {
        return false;
    }

    // Element is currently attempting to move to grid (x,y)
    if (MovDir[x][y] != 0) {
        return false;
    }

    // Grid now empty if Feld is empty
    return Feld[x][y] == enginetype::FIELD_EMPTY;
}


bool isNonWall(int x, int y) {
    // Check bounds (this shouldn't happen but best to be safe)
    if (x < 0 || x >= level.fieldx || y < 0 || y >= level.fieldy) {
        // PLOGE_(logwrap::FileLogger) << "Index out of bounds.";
        return false;
    }

    // Grid now empty if Feld is empty
    return Feld[x][y] != enginetype::FIELD_WALL;
}


/*
 * Get all empty grid cells
 */
void getEmptyGridCells(std::vector<enginetype::GridCell> &emptyGridCells) {
    emptyGridCells.clear();
    for (int y = 0; y < level.fieldy; y++) {
        for (int x = 0; x < level.fieldx; x++) {
            if (isGridEmpty(x, y)) {
                emptyGridCells.push_back(enginetype::GridCell{x, y});
            }
        }
    }
}


void setNeighbours() {
    for (int y = 0; y < level.fieldy; y++) {
        for (int x = 0; x < level.fieldx; x++) {
            if (isNonWall(x, y)) {
                enginetype::GridCell grid_cell = {x, y};

                // find neighbours
                std::vector<enginetype::GridCell> neighbours;
                if (isNonWall(x-1, y)) {neighbours.push_back(enginetype::GridCell{x-1, y});}
                if (isNonWall(x+1, y)) {neighbours.push_back(enginetype::GridCell{x+1, y});}
                if (isNonWall(x, y-1)) {neighbours.push_back(enginetype::GridCell{x, y-1});}
                if (isNonWall(x, y+1)) {neighbours.push_back(enginetype::GridCell{x, y+1});}

                grid_neighbours_[grid_cell] = neighbours;
            }
        }
    }
}

bool checkIfNeighbours(std::vector<enginetype::GridCell> &cells_left, std::vector<enginetype::GridCell> &cells_right) {
    for (auto const cell_left : cells_left) {
        for (auto const cell_right : cells_right) {
            std::vector<enginetype::GridCell> right_result = grid_neighbours_[cell_right];
            if (std::find(right_result.begin(), right_result.end(), cell_left) != right_result.end()) {
                return true;
            }
        }
    }
    return false;
}


/*
 * Count how many of a specified element in the game
 */
int countNumOfElement(int element) {
    int count_element = 0;
    for (int y = 0; y < level.fieldy; y++) {
        for (int x = 0; x < level.fieldx; x++) {
            if (Feld[x][y] == element) {
                count_element += 1;
            }
        }
    }
    return count_element;
}


/*
 * Add the specified element to the game
 */
void spawnElement(int element, int dir, enginetype::GridCell gridCell) {
    Feld[gridCell.x][gridCell.y] = element;
    MovDir[gridCell.x][gridCell.y] = dir;
}


/*
 * Set flag for simulating
 * This will cause blocking actions in engine such as not rending to screen
 * Profiling shows a 10x in speed with simulator_flag set
 */
void setSimulatorFlag(bool simulator_flag) {
    is_simulating = (simulator_flag ? TRUE : FALSE);
}


/*
 * Get the simulator flag status
 */
bool isSimulating() {
    return is_simulating;
}


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

/*
 * Get the replay game name
 */
std::string getReplayFileName() {
    std::string replay_file(options.replay_file);
    return replay_file;
}


/*
 * Get the players current shortest path distance to goal
 * This uses distance tile maps pre-calculated using Dijkstra algorithm,
 * NOT Euclidean distance.
 */
float getDistanceToGoal() {
    int playerX = stored_player[0].jx;
    int playerY = stored_player[0].jy;
    return (float)distances[playerX][playerY];
}


// Data types/structures for Dijkstra
typedef std::array<int, 2> Point;
short INF = std::numeric_limits<short>::max();
short distances[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short max_distance = -1;


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


/*
 * Get the neighbours of point u from queue Q and insert into neighbours
 */
void _getNeighbours(Point u, std::vector<Point> &neighbours, std::vector<Point> &Q) {
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
 * Find the grid location of the goal, given by enginetype::FIELD_GOAL
 */
void findGoalLocation(int &goal_x, int &goal_y) {
    goal_x = -1;
    goal_y = -1;
    for (int y = 0; y < level.fieldy; y++) {
        for (int x = 0; x < level.fieldx; x++) {
            if (Feld[x][y] == enginetype::FIELD_GOAL) {
                goal_x = x;
                goal_y = y;
                PLOGI_(logwrap::FileLogger) << "Found goal at x=" << x << ", y=" << y << ".";
                return;
            }
        }
    }
}


/*
 * Set the grid distances to goal using Dijkstra's algorithm (shortest path)
 */
void setBoardDistancesDijkstra(int goal_x, int goal_y) {
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
    if (goal_x == -1 || goal_y == -1) {
        PLOGI_(logwrap::FileLogger) << "Level has no goal.";
        return;
    }

    // Check goal in bounds
    if (goal_x < 0 || goal_x >= level.fieldx || goal_y < 0 || goal_y >= level.fieldy) {
        PLOGI_(logwrap::FileLogger) << "Provided goal is out of level bounds.";
        return;
    }

    // Set goal distance
    distances[goal_x][goal_y] = 0;

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
        _getNeighbours(u, neighbours, Q);

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
void setBoardDistancesL1(int goal_x, int goal_y) {
    PLOGI_(logwrap::FileLogger) << "Setting board distances.";

    if (goal_x == -1 && goal_y == -1) {findGoalLocation(goal_x, goal_y);}

    // Initialize distances
    for (int y = 0; y < level.fieldy; y++) {
        for (int x = 0; x < level.fieldx; x++) {
            distances[x][y] = -1;
        }
    }

    // If no goal, then break
    if (goal_x == -1 || goal_y == -1) {
        PLOGI_(logwrap::FileLogger) << "Level has no goal.";
        return;
    }

    // Check goal in bounds
    if (goal_x < 0 || goal_x >= level.fieldx || goal_y < 0 || goal_y >= level.fieldy) {
        PLOGI_(logwrap::FileLogger) << "Provided goal is out of level bounds.";
        return;
    }

    // Set goal distance
    distances[goal_x][goal_y] = 0;

    // Set other grid distances
    for (int y = 0; y < level.fieldy; y++) {
        for (int x = 0; x < level.fieldx; x++) {
            if (Feld[x][y] != enginetype::FIELD_WALL) {
                distances[x][y] = std::abs(goal_x - x) + std::abs(goal_y - y);
            }
        }
    }
}

}