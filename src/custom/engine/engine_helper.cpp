

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
 * Set the action for the engine to perform on behalf of the player on
 * the next iteration as a random action
 */
void setEngineRandomPlayerAction() {
    // stored_player[0].action = ALL_ACTIONS[std::rand() % _NUM_ACTIONS];
    std::vector<Action> available_actions;

    int playerX = stored_player[0].jx;
    int playerY = stored_player[0].jy;

    available_actions.push_back(Action::noop);
    if (Feld[playerX][playerY+1] != _FIELD_WALL) {
        available_actions.push_back(Action::down);
    }
    else if (Feld[playerX+1][playerY] != _FIELD_WALL) {
        available_actions.push_back(Action::right);
    }
    else if (Feld[playerX][playerY-1] != _FIELD_WALL) {
        available_actions.push_back(Action::up);
    }
    else if (Feld[playerX-1][playerY] != _FIELD_WALL) {
        available_actions.push_back(Action::left);
    }
    
    stored_player[0].action = available_actions[std::rand() % available_actions.size()];
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
    for (int i = 0; i < _ENGINE_RESOLUTION; i++) {
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

void setSimulatorFlag(bool simulator_flag) {
    is_simulating = (simulator_flag ? TRUE : FALSE);
}


float getDistanceToGoal() {
    int playerX = stored_player[0].jx;
    int playerY = stored_player[0].jy;
    return (float)distances[playerY][playerX];
}



typedef std::array<int, 2> Point;
int INF = std::numeric_limits<int>::max();
std::vector<std::vector<int>> distances;

void getMinDistanceIndex(std::vector<Point> &Q, int &index) {
    int min_index = -1;
    int min_value = INF;
    for (int i = 0; i < (int)Q.size(); i++) {
        int x = Q[i][0]; 
        int y = Q[i][1];
        if (distances[y][x] <= min_value) {
            min_index = i;
            min_value = distances[y][x];
        }
    }
    index = min_index;
}


void getNeighbours(Point u, std::vector<Point> &neighbours, std::vector<Point> &Q) {
    int x = u[0];
    int y = u[1];

    if (x-1 >= 0 && Feld[x-1][y] != _FIELD_WALL 
        && std::find(Q.begin(), Q.end(), (Point){x-1,y}) != Q.end()) 
    {
        neighbours.push_back((Point){x-1,y});
    }
    if (x+1 < level.fieldx && Feld[x+1][y] != _FIELD_WALL && 
        std::find(Q.begin(), Q.end(), (Point){x+1,y}) != Q.end()) 
    {
        neighbours.push_back((Point){x+1,y});
    }

    if (y-1 >= 0 && Feld[x][y-1] != _FIELD_WALL 
        && std::find(Q.begin(), Q.end(), (Point){x,y-1}) != Q.end()) 
    {
        neighbours.push_back((Point){x,y-1});
    }
    if (y+1 < level.fieldy && Feld[x][y+1] != _FIELD_WALL && 
        std::find(Q.begin(), Q.end(), (Point){x,y+1}) != Q.end()) 
    {
        neighbours.push_back((Point){x,y+1});
    }
}


void dijkstra() {
    int goalX=-1, goalY=-1;
    int x, y;
    distances.clear();
    std::vector<Point> Q;

    // Find goal
    for (y = 0; y < level.fieldy; y++) {
        std::vector<int> temp;
        for (x = 0; x < level.fieldx; x++) {
            int dist = INF;
            if (Feld[x][y] == _FIELD_GOAL) {
                goalX = x;
                goalY = y;
                dist = 0;
            }
            if (Feld[x][y] != _FIELD_WALL) {
                Q.push_back({x, y});
            }
            temp.push_back(dist);
        }
        distances.push_back(temp);
    }

    // Calc distances
    Point u;
    int index;
    while (!Q.empty()) {
        // Get min distance from vertex set and remove
        getMinDistanceIndex(Q, index);
        u = Q[index];
        Q.erase(Q.begin() + index);

        // Get neighbours
        std::vector<Point> neighbours;
        getNeighbours(u, neighbours, Q);

        // For each neighbour, update distance
        for (Point v : neighbours) {
            int alt = distances[u[1]][u[0]] + 1;
            if (alt < distances[v[1]][v[0]]) {
                distances[v[1]][v[0]] = alt;
            }
        }
    }

    // Set max distances to neg
    for (y = 0; y < level.fieldy; y++) {
        for (x = 0; x < level.fieldx; x++) {
            distances[y][x] = (distances[y][x] == INF ? -1 : distances[y][x]);
        }
    }
}


/*
 * Print current engine status
 */
void debug_print_distances() {
    int x, y;
    for (y = 0; y < level.fieldy; y++) {
        for (x = 0; x < level.fieldx; x++) {
            std::cout.width(5);
            std::cout << distances[y][x] << " ";
        }
        std::cout << std::endl;
    }

    std::cout << "-----------------" << std::endl;
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