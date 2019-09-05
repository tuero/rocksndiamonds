

#ifndef ABSTRACTNODE_H
#define ABSTRACTNODE_H

#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <algorithm>
#include <memory>

#include "../../engine/engine_helper.h"
#include "../../engine/engine_types.h"

class AbstractNode {
    typedef std::unique_ptr<AbstractNode> Pointer;
private:
    AbstractNode* parent_;                          // Pointer to parent state at higher abstraction level.
    std::map<int, AbstractNode*> neighbours_;       // Neighbour nodes at same level of abstraction.
    std::map<int, Pointer> children_;               // Children nodes which current nodes abstracts.
    std::vector<enginetype::GridCell> represented_cells_;   // Grid cells the current nodes represents through abstraction.
    AbstractNode* path_parent_;                     // Pointer to parent as found during pathfinding.

    int id_;                        // Unique ID representing node.
    int level_;                     // Level of abstraction the node represents.
    float h_, g_;                   // G and H values used during pathfinding.
    bool represents_goal_;          // Flag to check if node represents grid cell containing goal.

public:

    /*
     * Contructor for base level node (represents a single grid cell).
     */
    AbstractNode(int id, int level, float value, enginetype::GridCell represented_cell);

    /*
     * Constructor for node abstracting given children.
     */
    AbstractNode(int id, int level, std::vector<Pointer> &children);

    /*
     * Add a neighbour node at the same level of abstraction.
     */
    void addNeighbour(AbstractNode* neighbour);

    /*
     * Update the nodes H value. This is computed by averaging the H value of 
     * all the children nodes.
     */
    void updateValue();


    // ------------------- Setters -------------------

    /*
     * Set the parent node which abstracts the current node.
     */
    void setParent(AbstractNode* parent);

    /*
     * Set the parent as found during pathfinding.
     */
    void setPathParent(AbstractNode* path_parent);

    /*
     * Set the unique node ID.
     */
    void setId(int id);

    /*
     * Set the H value, which is the L1 distance from the goal.
     */
    void setValueH(float value);

    /*
     * Set the G value, which is the centroid distance between abstract nodes.
     */
    void setValueG(float value);

    /*
     * Set the level of abstraction the node represents.
     */
    void setLevel(int level);


    // ------------------- Getters -------------------

    /*
     * Get the parent node as found during pathfinding.
     */
    AbstractNode* getPathParent();

    /*
     * Get the ID of the node.
     */
    int getId();

    /*
     * Get the H value, which is the L1 distance from goal.
     */
    float getValueH();

    /*
     * Get the G value, which is the centroid distance between abstract nodes.
     */
    float getValueG();

    /*
     * Get the sum of G and H values.
     */
    float getValueF();

    /*
     * Get the level of abstraction the node is in.
     */
    int getLevel();

    /*
     * Checks if the node represents the goal grid cell.
     */
    bool representsGoal();

    /*
     * Check if the node represents the given grid cell.
     */
    bool representsCell(enginetype::GridCell &grid_cell);

    /*
     * Get the children nodes the current node represents.
     */
    std::vector<AbstractNode*> getChildren();

    /*
     * Get the neighbour abstract nodes at the same level of abstraction.
     */
    const std::map<int, AbstractNode*> & getNeighbours();

    /*
     * Get the neighbour abstract nodes at the same level of abstraction.
     */
    std::vector<AbstractNode*> getNeighboursVec();

    /*
     * Get the grid cells the current node represents through abstraction.
     */
    const std::vector<enginetype::GridCell> & getRepresentedCells() const;


};


class CompareAbstractNode {
public:
    bool operator() (AbstractNode* left, AbstractNode* right) {
        return left->getValueF() > right->getValueF();
    }
};

#endif  //ABSTRACTNODE_H


