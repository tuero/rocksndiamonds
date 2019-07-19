
#include "tree_node.h"

typedef std::unique_ptr<TreeNode> Pointer;

TreeNode::TreeNode(TreeNode* parent, const std::vector<Action> &actions_from_start)
    :  parent(parent), actions_from_start(actions_from_start) 
{
    action_taken = Action::noop;

    // Reset statistics
    visits_count = 0;
    depth = (parent == nullptr ? 0 : parent->depth + 1);
    value = 0;
    distance_to_goal = -1;
    count_goals_found = 0;
    count_times_died = 0;
    is_safe = 0;

    is_terminal = getTerminalStatusFromEngine();
    is_deadly = getDeadlyStatusFromEngine();
    is_solved = getSolvedStatusFromEngine();
}


void TreeNode::setActions() {
    // Simulator must be in this new state to determine the possible child actions
    // We always keep noop at front so far left of tree is sequence of noops
    actions.clear();
    actions.push_back(Action::noop);

    for (Action action : ALL_ACTIONS) {
        // Player is blocked by wall
        if (enginehelper::isWall(action) || action == Action::noop) {continue;}
        actions.push_back(action);
    }

    std::random_shuffle(std::next(actions.begin()), actions.end());
}


bool TreeNode::allExpanded() const {
    return children.size() >= actions.size();
}


Action TreeNode::getActionTaken() {
    return action_taken;
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
    Pointer child = std::make_unique<TreeNode>(this, actions_from_start);
    Action child_action = actions[children.size()];
    child.get()->action_taken = child_action;

    // Simulate to child
    enginehelper::setEnginePlayerAction(child_action);

    for (unsigned int i = 0; i < enginetype::ENGINE_RESOLUTION; i++) {
        child.get()->actions_from_start.push_back(child_action);
        enginehelper::engineSimulateSingle();
    }

    // Simulator is set to the child state, so determine what the available actions
    // the child has
    child.get()->setActions();
    child.get()->distance_to_goal = enginehelper::getDistanceToGoal();

    child.get()->is_terminal = getTerminalStatusFromEngine();
    child.get()->is_deadly = getDeadlyStatusFromEngine();
    child.get()->is_solved = getSolvedStatusFromEngine();

    GameState reference;
    reference.setFromSimulator();
    for (unsigned int i = 0; i < enginetype::ENGINE_RESOLUTION - 1; i++) {
        enginehelper::setEnginePlayerAction(Action::noop);
        enginehelper::engineSimulateSingle();
        child.get()->is_terminal = child.get()->is_terminal | getTerminalStatusFromEngine();
        child.get()->is_deadly = child.get()->is_deadly | getDeadlyStatusFromEngine();
    }
    enginehelper::setEnginePlayerAction(Action::noop);
    enginehelper::engineSimulateSingle();

    // Extra steps to ensure we do not terminate the next tick
    // child.get()->is_terminal = getTerminalStatusFromEngine();
    // child.get()->is_deadly = getDeadlyStatusFromEngine();
    // child.get()->is_solved = getSolvedStatusFromEngine();

    // reference.restoreSimulator();
    // RNG::setSeedEngineHash();

    // Store child
    children.push_back(std::move(child));

    return children.back().get();
}


TreeNode* TreeNode::getParent() {
    return parent;
}


void TreeNode::updateStats(const float goal_found, const float died, const float countSafe) {
    visits_count += 1;
    // count_goals_found += (goal_found ? 1 : 0);
    // count_times_died += (died ? 1 : 0);
    count_goals_found += goal_found;
    count_times_died += died;
    is_safe += countSafe;
}

TreeNode* TreeNode::getChild(unsigned int index) {
    assert(index < children.size());
    return children[index].get();
}

Pointer TreeNode::getChildByAction(Action action) {
    for (unsigned int i = 0; i < children.size(); i++) {
        if (children[i].get()->getActionTaken() == action) {
            return std::move(children[i]);
        }
    }
    return nullptr;
}


void TreeNode::setParent(TreeNode* parent) {
    this->parent = parent;
}


// ------------------- Statistic Getters ---------------------

unsigned int TreeNode::getChildCount() const {
    return children.size();
}

float TreeNode::getValue() const {
    return value;
}

int TreeNode::getVisitCount() const {
    return visits_count;
}

int TreeNode::getDepth() const {
    return depth;
}

int TreeNode::getDistance() const {
    return distance_to_goal;
}

float TreeNode::getGoalCount() const {
    return count_goals_found;
}

float TreeNode::getDiedCount() const {
    return count_times_died;
}

float TreeNode::getIsSafe() const {
    return is_safe;
}