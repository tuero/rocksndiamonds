
#include "abstract_node.h"

AbstractNode::AbstractNode(int id, int level, float value, enginetype::GridCell represented_cell) 
{
    parent_ = nullptr;
    id_ = id;
    level_ = level;
    value_ = value;
    represented_cells_.push_back(represented_cell);
}


AbstractNode::AbstractNode(int id, int level, std::vector<Pointer> &children) 
{
    parent_ = nullptr;
    id_ = id;
    level_ = level;
    value_ = 0.0;

    // Add children
    for (unsigned int i = 0; i < children.size(); i++) {
        AbstractNode* current_node = children[i].get();
        int id = current_node->getId();
        value_ += current_node->getValue();
        // Set child parent
        current_node->setParent(this);
        // Add to represented cells
        std::vector<enginetype::GridCell> represented_cell = current_node->getRepresentedCells();
        represented_cells_.insert(represented_cells_.end(), represented_cell.begin(), represented_cell.end());
        children_[id] = std::move(children[i]);
    }

    // Calculate value
    value_ = value_ / children.size();
}


void AbstractNode::addNeighbours(std::vector<AbstractNode*> &neighbours) {
    for (auto const &neighbour : neighbours) {
        neighbours_[neighbour->getId()] = neighbour;
    }
}

void AbstractNode::addNeighbour(AbstractNode* neighbour) {
    neighbours_[neighbour->getId()] = neighbour;
}


// ------------------- Setters -------------------

void AbstractNode::setParent(AbstractNode* parent) {
    parent_ = parent;
}


void AbstractNode::setId(int id) {
    id_ = id;
}


void AbstractNode::setValue(float value) {
    value_ = value;
}

void AbstractNode::setLevel(int level) {
    level_ = level;
}

// ------------------- Getters -------------------

int AbstractNode::getId() {
    return id_;
}


float AbstractNode::getValue() {
    return value_;
}

int AbstractNode::getLevel() {
    return level_;
}

std::vector<AbstractNode*> AbstractNode::getChildren() {
    std::vector<AbstractNode*> children;
    for (auto const &child : children_) {
        AbstractNode* child_node = child.second.get();
        children.push_back(child_node);
    }
    return children;
}

const std::map<int, AbstractNode*> & AbstractNode::getNeighbours() {
    return neighbours_;
}

const std::vector<enginetype::GridCell> & AbstractNode::getRepresentedCells() const {
    return represented_cells_;
}