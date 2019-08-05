

#ifndef ABSTRACTNODE_H
#define ABSTRACTNODE_H

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <memory>

#include "../../engine/engine_helper.h"
#include "../../engine/engine_types.h"



class AbstractNode {
    typedef std::unique_ptr<AbstractNode> Pointer;
private:
    AbstractNode* parent_;                       // Pointer to parent state
    // Action action_taken;                    // Action which led to this state from parent
    // std::vector<AbstractNode*> neighbours_;        // current children from this node
    std::map<int, AbstractNode*> neighbours_;
    // std::vector<Pointer> children_;          // current children from this node
    std::map<int, Pointer> children_;
    std::vector<enginetype::GridCell> represented_cells_;

    int id_;
    int level_;
    float value_;

public:

    AbstractNode(int id, int level, float value, enginetype::GridCell represented_cell);

    AbstractNode(int id, int level, std::vector<Pointer> &children);

    void setParent(AbstractNode* parent);

    void addNeighbours(std::vector<AbstractNode*> &neighbours);

    void addNeighbour(AbstractNode* neighbour);


    void setId(int id);

    void setValue(float value);

    void setLevel(int level);

    int getId();

    float getValue();

    int getLevel();

    std::vector<AbstractNode*> getChildren();

    const std::map<int, AbstractNode*> & getNeighbours();

    const std::vector<enginetype::GridCell> & getRepresentedCells() const;


};

#endif  //ABSTRACTNODE_H


