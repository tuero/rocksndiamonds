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

#include <algorithm>
#include <iostream>
#include <cassert>
#include <typeinfo>

#include "../options/option_types.h"

// Engine
#include "../../engine/engine_helper.h"

//Logging
#include "../util/logging_wrapper.h"
#include <plog/Log.h>   


typedef std::unique_ptr<TreeNode> Pointer;

TreeNode::TreeNode(TreeNode* parent, const std::vector<Action> &actionsFromStart) :  
    parent_(parent), actionsFromStart_(actionsFromStart)
{
    children_.clear();

    // Reset statistics
    visitCount_ = 0;
    depth_ = (parent_ == nullptr ? 0 : parent_->depth_ + 1);
    value_ = 0;

    isTerminal_ = (enginehelper::engineGameFailed() || enginehelper::engineGameSolved());
    isFailed_ = enginehelper::engineGameFailed();
    isSolved_ = enginehelper::engineGameSolved();
}


/*
 * Expand the next possible child.
 * 
 * The parent is responsible for the child memory. The child represents the state 
 * which follows from the next available option.
 */
TreeNode* TreeNode::expand() {
    // Should be non-terminal and not fully expanded. MCTS checks this outside
    assert(!allExpanded());
    // assert(!allExpanded() && !(enginehelper::engineGameFailed() || enginehelper::engineGameSolved()));

    // Create new node
    Pointer child = std::make_unique<TreeNode>(this, actionsFromStart_);
    TreeNode* childNode = child.get();

    BaseOption* option = availableOptions_[children_.size()];
    childNode->optionTaken_ = option;

    // Simulate to child
    PLOGE_(logwrap::FileLogger) << "Expanding Option: " << option->optionToString();
    // option->run();

    childNode->isTerminal_ = false;
    if (!option->run()) {
        childNode->isTerminal_ = true;
        PLOGE_(logwrap::FileLogger) << "Is Terminal"; 
    }

    // Simulator is set to the child state, so determine what the available actions
    // the child has
    childNode->setOptions(allOptions_);


    // Set termination flags
    childNode->isTerminal_ |= (enginehelper::engineGameFailed() || enginehelper::engineGameSolved());
    childNode->isFailed_ = enginehelper::engineGameFailed();
    childNode->isSolved_ = enginehelper::engineGameSolved();

    // Store child
    children_.push_back(std::move(child));

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

bool compareBaseOptionPointer(BaseOption* a, BaseOption* b) { return (a->getTimesCalled() < b->getTimesCalled());}


/*
 * Set the options the tree node can represent.
 */
void TreeNode::setOptions(std::vector<BaseOption*> &allOptions) {
    // Prioritize recent valid options.
    // Option is recently valid if currently valid while not valid in parent.
    // Create separate vectors (newely available, prevously visited?), shuffle each then concatenate.
    std::vector<BaseOption*> singleStepOptions;
    std::vector<BaseOption*> recentlAvailableOptions;
    std::vector<BaseOption*> otherOptions;
    availableOptions_.clear();
    allOptions_ = allOptions;

    for (BaseOption* option : allOptions) {
        if (!option->isValid()) {continue;}

        // Check if was not valid in parent
        if (parent_ != nullptr && std::find(parent_->availableOptions_.begin(), 
            parent_->availableOptions_.end(), option) != parent_->availableOptions_.end()) 
        {
            recentlAvailableOptions.push_back(option);
        }
        // Non-single step option
        // else if (option->getOptionType() != OptionType::SingleStep) {
        //     otherOptions.push_back(option);
        // }
        else if (option->optionToString().compare("Single step action: noop") == 0) {
            singleStepOptions.push_back(option);
        }
        else {
           otherOptions.push_back(option);
        }
        // availableOptions_.push_back(option); 
    }
    // std::random_shuffle(std::next(availableOptions_.begin()), availableOptions_.end());
    std::random_shuffle(singleStepOptions.begin(), singleStepOptions.end(), [&](int i) {return std::rand() % i;});
    std::random_shuffle(recentlAvailableOptions.begin(), recentlAvailableOptions.end(), [&](int i) {return std::rand() % i;});
    std::random_shuffle(otherOptions.begin(), otherOptions.end(), [&](int i) {return std::rand() % i;});
    sort(otherOptions.begin(), otherOptions.end(), compareBaseOptionPointer);

    availableOptions_.insert(availableOptions_.end(), recentlAvailableOptions.begin(), recentlAvailableOptions.end());
    availableOptions_.insert(availableOptions_.end(), singleStepOptions.begin(), singleStepOptions.end());
    availableOptions_.insert(availableOptions_.end(), otherOptions.begin(), otherOptions.end());
}


/*
 * Get the child for a specified index.
 */
TreeNode* TreeNode::getChild(unsigned int index) {
    assert(index < children_.size());
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

    PLOGE_(logwrap::FileLogger) << "Couldn't find child representing option: " << optionTaken->optionToString();

    return nullptr;
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
