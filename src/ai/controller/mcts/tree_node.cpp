/**
 * @file: tree_node.cpp
 *
 * @brief: Tree node used in MCTS.
 * 
 * @author: Jake Tuero
 * Date: September 2019
 * Contact: tuero@ualberta.ca
 */


#include "tree_node.h"

// Standard Libary/STL
#include <algorithm>
#include <typeinfo>

// Includes
#include "option_types.h"
#include "engine_helper.h"
#include "logger.h"


typedef std::unique_ptr<TreeNode> Pointer;

TreeNode::TreeNode(TreeNode* parent, const std::vector<Action> &actionsFromStart) :  
    parent_(parent), actionsFromStart_(actionsFromStart)
{
    children_.clear();

    // Reset statistics
    visitCount_ = 0;
    depth_ = (parent_ == nullptr ? 0 : parent_->depth_ + 1);
    value_ = 0;

    // Set internal engine status flags
    isFailed_ = enginehelper::engineGameFailed();
    isSolved_ = enginehelper::engineGameSolved();
    isTerminal_ = isFailed_ && isSolved_;
}


/*
 * Expand the next possible child.
 * 
 * The parent is responsible for the child memory. The child represents the state 
 * which follows from the next available option.
 */
TreeNode* TreeNode::expand() {
    // Should be non-terminal and not fully expanded. MCTS checks this outside
    if (allExpanded()) {
        PLOGE_(logger::FileLogger) << "Trying to expand a fully expanded node.";
        PLOGE_(logger::ConsoleLogger) << "Trying to expand a fully expanded node.";
        throw std::exception();
    }

    // Create new node
    Pointer child = std::make_unique<TreeNode>(this, actionsFromStart_);
    TreeNode* childNode = child.get();

    // Set child node option taken
    BaseOption* option = availableOptions_[children_.size()];
    childNode->optionTaken_ = option;

    // Simulate to child
    childNode->isTerminal_ = false;
    if (!option->run()) {
        childNode->isTerminal_ = true;
    }

    // Simulator is set to the child state, so determine what the available actions the child has
    childNode->setOptions(allOptions_);

    // Set termination flags
    childNode->isTerminal_ |= (enginehelper::engineGameFailed() || enginehelper::engineGameSolved());
    childNode->isFailed_ = enginehelper::engineGameFailed();
    childNode->isSolved_ = enginehelper::engineGameSolved();

    // Store child
    children_.push_back(std::move(child));

    // Get pointer to child
    return children_.back().get();
}


/*
 * Check if all child nodes have been expanded.
 */
bool TreeNode::allExpanded() const {
    return children_.size() >= availableOptions_.size();
}


/*
 * Check if the state represented by the tree node is terminal.
 *
 * A tree node is terminal if its failed or solved.
 */
bool TreeNode::isTerminal() const {
    return isTerminal_;
}


/*
 * Check if the state represented by the tree node is failed.
 */
bool TreeNode::isFailed() const {
    return isFailed_;
}

/*
 * Check if the state represented by the tree node is solved.
 */
bool TreeNode::isSolved() const {
    return isSolved_;
}


/*
 * Update the tree node stats.
 */
void TreeNode::updateStats(const float value) {
    visitCount_ += 1;
    value_ += value;
}


/*
 * Get the option taken by the tree node.
 */
BaseOption* TreeNode::getOptionTaken() {
    return optionTaken_;
}


/*
 * Set the options the tree node can represent.
 */
void TreeNode::setOptions(std::vector<BaseOption*> &allOptions) {
    availableOptions_.clear();
    allOptions_ = allOptions;

    // Only store valid options
    for (auto const & option : allOptions) {
        if (!option->isValid()) {continue;}
        availableOptions_.push_back(option);
    }
}


/*
 * Get the child for a specified index.
 */
TreeNode* TreeNode::getChild(unsigned int index) {
    if (index >= children_.size()) {
        PLOGE_(logger::FileLogger) << "Child index out of bounds.";
        PLOGE_(logger::ConsoleLogger) << "Child index out of bounds.";
        throw std::exception();
    }
    return children_[index].get();
}


/*
 * Get the child for a specified option taken.
 */
Pointer TreeNode::getChild(BaseOption* optionTaken) {
    for (std::size_t i = 0; i < children_.size(); i++) {
        if (children_[i].get()->getOptionTaken() == optionTaken) {
            return std::move(children_[i]);
        }
    }

    PLOGE_(logger::FileLogger) << "Can't find child representing given option.";
    PLOGE_(logger::ConsoleLogger) << "Can't find child representing given option.";
    throw std::exception();
}


/*
 * Get the parent for the node.
 */
TreeNode* TreeNode::getParent() {
    return parent_;
}


/*
 * Set the parent node.
 */
void TreeNode::setParent(TreeNode* parent) {
    parent_ = parent;
}


/*
 * Get the number of children already expanded.
 */
std::size_t TreeNode::getChildCount() const {
    return children_.size();
}


/*
 * Get the node value, as calculated by MCTS.
 */
float TreeNode::getValue() const {
    return value_;
}


/*
 * Get the number of times the node has been visited.
 */
int TreeNode::getVisitCount() const {
    return visitCount_;
}


/*
 * Get the depth of the node.
 */
int TreeNode::getDepth() const {
    return depth_;
}
