
#include "tree_node.h"


TreeNode::TreeNode(TreeNode* parent, const std::vector<Action> &actions_from_start)
    :  parent(parent), actions_from_start(actions_from_start) 
{
    action_taken = Action::noop;
    visits_count = 0;
    depth = (parent == nullptr ? 0 : parent->depth + 1);
    value = 0;
    distance = 0;
    depth_found = 0;
}


void TreeNode::setActions() {
    // Simulator must be in this new state to determine the possible child actions
    actions.clear();

    for (Action action : ALL_ACTIONS) {
        // Player is blocked by wall
        if (isWall(action)) {continue;}
        actions.push_back(action);
    }

    std::random_shuffle(actions.begin(), actions.end());
}


bool TreeNode::allExpanded() const {
    return children.size() >= actions.size();
}


Action TreeNode::getActionTaken() {
    return action_taken;
}


bool TreeNode::isTerminal() const {
    return (engineGameFailed() || engineGameSolved());
}


TreeNode* TreeNode::expand() {
    // Should be non-terminal and not fully expanded. MCTS checks this outside
    assert(!allExpanded() && !isTerminal());

    // Create new node
    Pointer child = std::make_unique<TreeNode>(this, actions_from_start);
    Action child_action = actions[children.size()];
    child.get()->action_taken = child_action;

    // Simulate to child
    setEnginePlayerAction(child_action);

    for (int i = 0; i < _ENGINE_RESOLUTION; i++) {
        child.get()->actions_from_start.push_back(child_action);
        engineSimulateSingle();
    }

    // Simulator is set to the child state, so determine what the available actions
    // the child has
    child.get()->setActions();
    child.get()->distance = getDistanceToGoal();

    // Store child
    children.push_back(std::move(child));

    return children.back().get();
}


TreeNode* TreeNode::getParent() {
    return parent;
}


void TreeNode::updateStats(const float value, const int depth_found) {
    visits_count += 1;
    this->value = value;
    this->depth_found = depth_found;
}


int TreeNode::getChildCount() const {
    return children.size();
}

TreeNode* TreeNode::getChild(int index) {
    assert(index < children.size());
    return children[index].get();
}


float TreeNode::getValue() const {
    return value;
}

int TreeNode::getVisitCount() const {
    return visits_count;
}

float TreeNode::getDistance() const {
    return distance;
}

float TreeNode::getDepth() const {
    return (float)depth;
}

float TreeNode::getDepthFound() const {
    return (float)depth_found;
}
