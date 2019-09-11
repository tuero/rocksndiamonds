
#include "tree_node.h"

#include <algorithm>
#include <iostream>
#include <cassert>

// Engine
#include "../../engine/engine_helper.h"

TreeNode::TreeNode(TreeNode* parent, const std::vector<Action> &actionsFromStart) :  
    parent_(parent), actionsFromStart_(actionsFromStart)
{
    actionTaken_ = Action::noop;

    // Reset statistics
    visitCount_ = 0;
    depth_ = (parent_ == nullptr ? 0 : parent_->depth_ + 1);
    value_ = 0;

    is_terminal = getTerminalStatusFromEngine();
    is_deadly = getDeadlyStatusFromEngine();
    is_solved = getSolvedStatusFromEngine();
}


void TreeNode::setActions() {
    // Simulator must be in this new state to determine the possible child actions
    // We always keep noop at front so far left of tree is sequence of noops
    actions_.clear();
    actions_.push_back(Action::noop);

    for (Action action : ALL_ACTIONS) {
        // Player is blocked by wall
        if (enginehelper::isWall(action) || action == Action::noop) {continue;}
        actions_.push_back(action);
    }

    std::random_shuffle(std::next(actions_.begin()), actions_.end());
}


bool TreeNode::allExpanded() const {
    return children_.size() >= actions_.size();
}


Action TreeNode::getActionTaken() {
    return actionTaken_;
}


bool TreeNode::getTerminalStatusFromEngine() const {
    return (enginehelper::engineGameFailed() || enginehelper::engineGameSolved());
}


bool TreeNode::getDeadlyStatusFromEngine() const {
    return enginehelper::engineGameFailed();
}

bool TreeNode::getSolvedStatusFromEngine() const {
    return enginehelper::engineGameSolved();
}


bool TreeNode::isTerminal() const {
    return is_terminal;
}


bool TreeNode::isDeadly() const {
    return is_deadly;
}


bool TreeNode::isSolved() const {
    return is_solved;
}


TreeNode* TreeNode::expand() {
    // Should be non-terminal and not fully expanded. MCTS checks this outside
    assert(!allExpanded() && !getTerminalStatusFromEngine());

    // Create new node
    Pointer child = std::make_unique<TreeNode>(this, actionsFromStart_);
    TreeNode* childNode = child.get();
    Action child_action = actions_[children_.size()];
    childNode->actionTaken_ = child_action;

    // Simulate to child
    enginehelper::setEnginePlayerAction(child_action);

    for (int i = 0; i < enginetype::ENGINE_RESOLUTION; i++) {
        childNode->actionsFromStart_.push_back(child_action);
        enginehelper::engineSimulateSingle();
    }

    // Simulator is set to the child state, so determine what the available actions
    // the child has
    childNode->setActions();

    childNode->is_terminal = getTerminalStatusFromEngine();
    childNode->is_deadly = getDeadlyStatusFromEngine();
    childNode->is_solved = getSolvedStatusFromEngine();

    // Store child
    children_.push_back(std::move(child));

    return children_.back().get();
}


TreeNode* TreeNode::getParent() {
    return parent_;
}


void TreeNode::updateStats(const float value) {
    visitCount_ += 1;
    value_ += value;
}

TreeNode* TreeNode::getChild(unsigned int index) {
    assert(index < children_.size());
    return children_[index].get();
}

TreeNode::Pointer TreeNode::getChildByAction(Action action) {
    for (unsigned int i = 0; i < children_.size(); i++) {
        if (children_[i].get()->getActionTaken() == action) {
            return std::move(children_[i]);
        }
    }
    return nullptr;
}


void TreeNode::setParent(TreeNode* parent) {
    parent_ = parent;
}


// ------------------- Statistic Getters ---------------------

std::size_t TreeNode::getChildCount() const {
    return children_.size();
}

float TreeNode::getValue() const {
    return value_;
}

int TreeNode::getVisitCount() const {
    return visitCount_;
}

int TreeNode::getDepth() const {
    return depth_;
}

// int TreeNode::getDistance() const {
//     return distance_to_goal;
// }

// float TreeNode::getGoalCount() const {
//     return count_goals_found;
// }

// float TreeNode::getDiedCount() const {
//     return count_times_died;
// }

// float TreeNode::getIsSafe() const {
//     return is_safe;
// }