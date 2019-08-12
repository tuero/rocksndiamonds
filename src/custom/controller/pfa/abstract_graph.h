

#ifndef ABSTRACTGRAPH_H
#define ABSTRACTGRAPH_H

#include <iostream>
#include <vector>
#include <array>
#include <deque>
#include <map>
#include <set>
#include <algorithm>
#include <memory>
#include <limits>
#include <fstream>

#include "abstract_node.h"

#include "../../engine/engine_helper.h"
#include "../util/timer.h"

//Logging
#include "../util/logging_wrapper.h"
#include <plog/Log.h>   


class AbstractGraph {
    typedef std::unique_ptr<AbstractNode> Pointer;
private:
    int id_;                // Current ID used for lastly created node (should always increment).
    int top_level_;         // Level number of current abstracted level (should always increment).
    std::map<int, Pointer> current_level_;              // Current level of nodes abstracted.
    std::map<int, Pointer> next_level_;                 // Temporary container while next level is built.
    std::map<enginetype::GridCell, int> rep_map_;       // Fast access node ID who represents given grid cell


    /*
     * Make an abstract node representing the given children, and
     * add it to the next_level_ map.
     */
    void makeNode(std::vector<Pointer> &children);

    /*
     * Store node ID for cells that it represents. This map is used later
     * when determining neighbours.
     */
    void recordRepresentedCells(AbstractNode* node);

    /*
     * Helper to recursively get nodes at a g vien level
     */
    void getNodesRecursive(AbstractNode* node, int level, std::vector<AbstractNode*> &nodes_at_level);

    /*
     * Check for cliques of size 3 and create an abstract node.
     */
    void findCliquesThree();

    /*
     * Check for cliques of size 2 and create an abstract node.
     */
    void findCliquesTwo();

    /*
     * Abstract remaining nodes as a clique of size 1.
     */
    void findCliquesOne();

    /*
     * Check if the given grid cell is a neighbour to the node represented
     * by the given ID.
     */
    bool checkNeighbour(int current_id, enginetype::GridCell grid_cell);

    /*
     * Find neighbours in the new abstract level and join.
     */
    void joinNeighbours();

    /*
     * Store the newly created abstracted level
     */
    void setCurrentFromNext();

    /*
     * Helper method to recursively traverse downward, setting distances for all
     * child nodes, then update the current node using the average of child values.
     */
    void calcDistancesRecursive(AbstractNode* current_node);

    /*
     * Pretty print the board with the repsective abstract nodes
     */
    void boardPrint(std::vector<std::vector<int>> &print_array);

public:

    AbstractGraph(); 

    /*
     * Initialize base layer of abstract graph.
     * A node is created for each individual walkable grid cell (non-wall).
     */
    void init();

    /*
     * Abstract the base level upwards until there is a single abstracted node.
     */
    void abstract();

    /*
     * Get the node which represents the top level of the abstraction.
     */
    AbstractNode* getTopLevelNode();

    /*
     * Get the node at the specified abstraction level which contains the player's 
     * starting position. Used for pathfinding.
     */
    AbstractNode* getStartNode(int level);

    /*
     * Get the abstract nodes for a given level.
     */
    std::vector<AbstractNode*> getNodesAtLevel(int level);

    /*
     * Get the abstract node ID for each gridcell at the specified level
     */
    std::vector<std::vector<int>> getAbstractRepresentation(int level, bool min_colouring=false);

    /*
     * Get the number of abstracted levels
     */
    int getLevel();

    /*
     * Set the goal location.
     * Will force all nodes to recalculate their values (average 
     * distance to the goal).
     */
    void setGoal(int goal_x, int goal_y);

    /*
     * Log details of the abstract graph.
     */
    void logGraph();



};


#endif  //ABSTRACTGRAPH_H


