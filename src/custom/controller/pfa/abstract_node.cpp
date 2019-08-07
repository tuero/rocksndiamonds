
#include "abstract_node.h"

AbstractNode::AbstractNode(int id, int level, float h, enginetype::GridCell represented_cell) 
{
    parent_ = nullptr;
    path_parent_ = nullptr;
    id_ = id;
    level_ = level;
    h_ = h;
    represents_goal_ = (h == 0.0);
    represented_cells_.push_back(represented_cell);
}


AbstractNode::AbstractNode(int id, int level, std::vector<Pointer> &children) 
{
    parent_ = nullptr;
    path_parent_ = nullptr;
    id_ = id;
    level_ = level;
    h_ = 0.0;
    represents_goal_ = false;

    // Add children
    for (unsigned int i = 0; i < children.size(); i++) {
        AbstractNode* child_node = children[i].get();
        int id = child_node->getId();
        h_ += child_node->getValueH();
        represents_goal_ = (represents_goal_ || child_node->representsGoal());
        // Set child parent
        child_node->setParent(this);
        // Add to represented cells
        std::vector<enginetype::GridCell> represented_cell = child_node->getRepresentedCells();
        represented_cells_.insert(represented_cells_.end(), represented_cell.begin(), represented_cell.end());
        children_[id] = std::move(children[i]);
    }

    // Calculate value
    h_ = h_ / children.size();
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


void AbstractNode::setPathParent(AbstractNode* path_parent) {
    path_parent_ = path_parent;
}



void AbstractNode::setId(int id) {
    id_ = id;
}


void AbstractNode::setValueH(float h) {
    h_ = h;
    represents_goal_ = (h == 0.0);
}

void AbstractNode::setValueG(float g) {
    g_ = g;
}

void AbstractNode::setLevel(int level) {
    level_ = level;
}

void AbstractNode::updateValue() {
    h_ = 0.0;

    for (auto const &child : children_) {
        AbstractNode* child_node = child.second.get();
        h_ += child_node->getValueH();
    }

    h_ = h_ / children_.size();
}

// ------------------- Getters -------------------

AbstractNode* AbstractNode::getPathParent() {
    return path_parent_;
}


int AbstractNode::getId() {
    return id_;
}


float AbstractNode::getValueH() {
    return h_;
}

float AbstractNode::getValueG() {
    return g_;
}

float AbstractNode::getValueF() {
    return h_ + g_;
}

int AbstractNode::getLevel() {
    return level_;
}

bool AbstractNode::representsGoal() {
    return represents_goal_;
}

bool AbstractNode::representsCell(enginetype::GridCell &grid_cell) {
    return std::find(represented_cells_.begin(), represented_cells_.end(), grid_cell) != represented_cells_.end();
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