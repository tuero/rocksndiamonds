

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
    AbstractNode* parent_;                       // Pointer to parent state
    std::map<int, AbstractNode*> neighbours_;
    std::map<int, Pointer> children_;
    std::vector<enginetype::GridCell> represented_cells_;

    AbstractNode* path_parent_; 

    int id_;
    int level_;
    float h_, g_;

    bool represents_goal_;

public:

    AbstractNode(int id, int level, float value, enginetype::GridCell represented_cell);

    AbstractNode(int id, int level, std::vector<Pointer> &children);

    void setParent(AbstractNode* parent);

    void setPathParent(AbstractNode* path_parent);

    void addNeighbours(std::vector<AbstractNode*> &neighbours);

    void addNeighbour(AbstractNode* neighbour);


    void setId(int id);

    void setValueH(float value);

    void setValueG(float value);

    void setLevel(int level);

    void updateValue();

    AbstractNode* getPathParent();

    int getId();

    float getValueH();

    float getValueG();

    float getValueF();

    int getLevel();

    bool representsGoal();

    bool representsCell(enginetype::GridCell &grid_cell);

    std::vector<AbstractNode*> getChildren();

    // Make a 
    const std::map<int, AbstractNode*> & getNeighbours();

    const std::vector<enginetype::GridCell> & getRepresentedCells() const;


};


class CompareAbstractNode {
public:
    bool operator() (AbstractNode* left, AbstractNode* right) {
        return left->getValueF() < right->getValueF();
    }
};

#endif  //ABSTRACTNODE_H


