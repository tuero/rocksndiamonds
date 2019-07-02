
#include "tree_node.h"


TreeNode::TreeNode(TreeNode* parent, const std::vector<Action> &actions_from_start)
    :  parent(parent), actions_from_start(actions_from_start) 
{
    action_taken = Action::noop;
    num_visits = 0;
    depth = (parent == nullptr ? 0 : parent->depth + 1);
    value = 0;
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

    // Store child
    children.push_back(std::move(child));

    return children.back().get();
}










void TreeNode::addChild(const Action childAction) {
    Pointer child = std::make_unique<TreeNode>(this, actions_from_start);
    child.get()->action_taken = childAction;
    children.push_back(std::move(child));
}


TreeNode* TreeNode::getParent() {
    return parent;
}


TreeNode* TreeNode::getChild(const Action action) {
    TreeNode* child_node = new TreeNode(this, actions_from_start);
    child_node->setActionTaken(action);
    stored_player[0].action = action;

    for (int i = 0; i < 8; i++) {
        child_node->actions_from_start.push_back(action);
        HandleGameActions();
    }

    return child_node;
}


void TreeNode::setActionTaken(Action actionTaken) {
    action_taken = actionTaken;
    stored_player[0].action = action_taken;

    for (int i = 0; i < 8; i++) {
        actions_from_start.push_back(action_taken);
        HandleGameActions();
    }
}


void TreeNode::setSimulatorToCurrent() {
    for (auto & element : actions_from_start) {
        stored_player[0].action = element;
        HandleGameActions();
    }
}