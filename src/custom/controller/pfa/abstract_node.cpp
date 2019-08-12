
#include "abstract_node.h"


/*
 * Contructor for base level node (represents a single grid cell).
 */
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


/*
 * Constructor for node abstracting given children.
 */
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
        h_ += child_node->getValueH();
        represents_goal_ = (represents_goal_ || child_node->representsGoal());

        // Set child parent
        child_node->setParent(this);
        
        // Add to represented cells
        std::vector<enginetype::GridCell> represented_cell = child_node->getRepresentedCells();
        represented_cells_.insert(represented_cells_.end(), represented_cell.begin(), represented_cell.end());
        children_[child_node->getId()] = std::move(children[i]);
    }

    // Calculate value
    h_ = h_ / children.size();
}


/*
 * Add a neighbour node at the same level of abstraction.
 */
void AbstractNode::addNeighbour(AbstractNode* neighbour) {
    neighbours_[neighbour->getId()] = neighbour;
}


/*
 * Update the nodes H value. This is computed by averaging the H value of 
 * all the children nodes.
 */
void AbstractNode::updateValue() {
    h_ = 0.0;

    for (auto const &child : children_) {
        AbstractNode* child_node = child.second.get();
        h_ += child_node->getValueH();
    }

    h_ = h_ / children_.size();
}


// ------------------- Setters -------------------

/*
 * Set the parent node which abstracts the current node.
 */
void AbstractNode::setParent(AbstractNode* parent) {
    parent_ = parent;
}


/*
 * Set the parent as found during pathfinding.
 */
void AbstractNode::setPathParent(AbstractNode* path_parent) {
    path_parent_ = path_parent;
}


/*
 * Set the unique node ID.
 */
void AbstractNode::setId(int id) {
    id_ = id;
}


/*
 * Set the H value, which is the L1 distance from the goal.
 */
void AbstractNode::setValueH(float h) {
    h_ = h;
    represents_goal_ = (h == 0.0);
}


/*
 * Set the G value, which is the centroid distance between abstract nodes.
 */
void AbstractNode::setValueG(float g) {
    g_ = g;
}


/*
 * Set the level of abstraction the node represents.
 */
void AbstractNode::setLevel(int level) {
    level_ = level;
}


// ------------------- Getters -------------------


/*
 * Get the parent node as found during pathfinding.
 */
AbstractNode* AbstractNode::getPathParent() {
    return path_parent_;
}


/*
 * Get the ID of the node.
 */
int AbstractNode::getId() {
    return id_;
}


/*
 * Get the H value, which is the L1 distance from goal.
 */
float AbstractNode::getValueH() {
    return h_;
}


/*
 * Get the G value, which is the centroid distance between abstract nodes.
 */
float AbstractNode::getValueG() {
    return g_;
}


/*
 * Get the sum of G and H values.
 */
float AbstractNode::getValueF() {
    return h_ + g_;
}


/*
 * Get the level of abstraction the node is in.
 */
int AbstractNode::getLevel() {
    return level_;
}


/*
 * Checks if the node represents the goal grid cell.
 */
bool AbstractNode::representsGoal() {
    return represents_goal_;
}


/*
 * Check if the node represents the given grid cell.
 */
bool AbstractNode::representsCell(enginetype::GridCell &grid_cell) {
    return std::find(represented_cells_.begin(), represented_cells_.end(), grid_cell) != represented_cells_.end();
}


/*
 * Get the children nodes the current node represents.
 */
std::vector<AbstractNode*> AbstractNode::getChildren() {
    std::vector<AbstractNode*> children;
    for (auto const &child : children_) {
        AbstractNode* child_node = child.second.get();
        children.push_back(child_node);
    }
    return children;
}


/*
 * Get the neighbour abstract nodes at the same level of abstraction.
 */
const std::map<int, AbstractNode*> & AbstractNode::getNeighbours() {
    return neighbours_;
}


/*
 * Get the grid cells the current node represents through abstraction.
 */
const std::vector<enginetype::GridCell> & AbstractNode::getRepresentedCells() const {
    return represented_cells_;
}