
#include "pfa_tree_node.h"

// typedef std::unique_ptr<PFATreeNode> Pointer;

PFATreeNode::PFATreeNode(PFATreeNode* parent, const std::vector<Action> &actions_from_start)
    :  parent(parent), actions_from_start(actions_from_start) 
{
    action_taken = Action::noop;

    // Reset statistics
    visits_count = 0;
    depth = (parent == nullptr ? 0 : parent->depth + 1);
    value = 0;
    distance_min_goal = -1;
    distance_l1_goal = enginehelper::getPlayerDistanceToNextNode();

    distance_sum_ = 0;
    survival_frequency_ = 0.1;
    depth_goal_found = std::numeric_limits<int>::max();
    count_goals_found = 0;
    count_times_died = 0;
    is_safe = 0;

    is_terminal = getTerminalStatusFromEngine();
    is_deadly = getDeadlyStatusFromEngine();
    is_solved = getSolvedStatusFromEngine();
}


void PFATreeNode::setActions(std::vector<enginetype::GridCell> &allowed_cells) {
    // Simulator must be in this new state to determine the possible child actions
    // We always keep noop at front so far left of tree is sequence of noops
    actions.clear();
    actions.push_back(Action::noop);

    for (Action action : ALL_ACTIONS) {
        // Player is blocked by wall
        if (action == Action::noop || !enginehelper::canExpand(action, allowed_cells)) {
            continue;
        }
        actions.push_back(action);
    }

    std::random_shuffle(std::next(actions.begin()), actions.end());
}


bool PFATreeNode::allExpanded() const {
    return children.size() >= actions.size();
}


Action PFATreeNode::getActionTaken() {
    return action_taken;
}


bool PFATreeNode::getTerminalStatusFromEngine() const {
    return (enginehelper::engineGameFailed() || enginehelper::engineGameSolved());
}


bool PFATreeNode::getDeadlyStatusFromEngine() const {
    return enginehelper::engineGameFailed();
}

bool PFATreeNode::getSolvedStatusFromEngine() const {
    return enginehelper::engineGameSolved();
}


bool PFATreeNode::isTerminal() const {
    return is_terminal;
}


bool PFATreeNode::isDeadly() const {
    return is_deadly;
}


bool PFATreeNode::isSolved() const {
    return is_solved;
}


PFATreeNode* PFATreeNode::expand(std::vector<enginetype::GridCell> &allowed_cells) 
{
    // Should be non-terminal and not fully expanded. MCTS checks this outside
    assert(!allExpanded() && !getTerminalStatusFromEngine());

    // Create new node
    Pointer child = std::make_unique<PFATreeNode>(this, actions_from_start);
    PFATreeNode* child_node = child.get();
    Action child_action = actions[children.size()];
    child_node->action_taken = child_action;
    child_node->actions_from_start.insert(child_node->actions_from_start.end(), 
        enginetype::ENGINE_RESOLUTION, child_action);

    // Reference to parent state (current), as we will be simulating the child node
    // due to the stochastic nature
    GameState parent_state;
    GameState alive_state;
    parent_state.setFromEngineState();

    // Simulate to child
    bool did_survive = false;
    for (int i = 0; i < 5; i++) {
        enginehelper::setEnginePlayerAction(child_action);
        enginehelper::engineSimulate();
        if (!getDeadlyStatusFromEngine()) {
            if (!did_survive) {alive_state.setFromEngineState();}
            child_node->survival_frequency_ += 0.2;
            did_survive = true;
        }
        if (i < 4) {parent_state.restoreEngineState();}
    }

    // Ensure we are in a alive state if possible
    if (did_survive && getDeadlyStatusFromEngine()) {
        alive_state.restoreEngineState();
    }

    child_node->survival_frequency_ = child_node->survival_frequency_ / 1.1;

    // Simulator is set to the child state, so determine what the available actions
    // the child has
    int distance = enginehelper::getPlayerDistanceToNextNode();
    child_node->setActions(allowed_cells);
    child_node->distance_min_goal = distance;
    child_node->distance_l1_goal = distance;
    child_node->depth_goal_found = (distance == 0 ? child_node->depth : std::numeric_limits<int>::max());

    // child_node->is_deadly = getDeadlyStatusFromEngine();
    child_node->is_deadly = (!did_survive);
    child_node->is_solved = (distance == 0);
    child_node->is_terminal = (child_node->is_deadly || child_node->is_solved);

    // Store child
    children.push_back(std::move(child));

    return children.back().get();
}


PFATreeNode* PFATreeNode::getParent() {
    return parent;
}


void PFATreeNode::updateStats(const float goal_found, const float survival_frequency, 
    const int depth, const float countSafe, const int min_distance_found) 
{
    visits_count += 1;
    // count_goals_found += (goal_found ? 1 : 0);
    // count_times_died += (died ? 1 : 0);
    count_goals_found += goal_found;
    // count_times_died += died;
    is_safe += countSafe;

    survival_frequency_ *= std::pow(survival_frequency, 1.0/depth);
    depth_goal_found = std::min(depth_goal_found, min_distance_found);

    distance_sum_ += depth;
}

PFATreeNode* PFATreeNode::getChild(unsigned int index) {
    assert(index < children.size());
    return children[index].get();
}

PFATreeNode::Pointer PFATreeNode::getChildByAction(Action action) {
    for (unsigned int i = 0; i < children.size(); i++) {
        if (children[i].get()->getActionTaken() == action) {
            return std::move(children[i]);
        }
    }
    return nullptr;
}


void PFATreeNode::setParent(PFATreeNode* parent) {
    this->parent = parent;
}

void PFATreeNode::updateMinDistance(int min_distance_found) {
    depth_goal_found = std::min(depth_goal_found, min_distance_found);
}


// ------------------- Statistic Getters ---------------------

unsigned int PFATreeNode::getChildCount() const {
    return children.size();
}

float PFATreeNode::getValue() const {
    return value;
}

int PFATreeNode::getVisitCount() const {
    return visits_count;
}

int PFATreeNode::getDepth() const {
    return depth;
}

int PFATreeNode::getMinDistance() const {
    return distance_min_goal;
}

int PFATreeNode::getL1Distance() const {
    return distance_l1_goal;
}

float PFATreeNode::getGoalCount() const {
    return count_goals_found;
}

float PFATreeNode::getDiedCount() const {
    return count_times_died;
}

float PFATreeNode::getIsSafe() const {
    return is_safe;
}

int PFATreeNode::getDistanceSum() const {
    return distance_sum_;
}

int PFATreeNode::getMinDepthToGoal() const {
    return depth_goal_found;
}

float PFATreeNode::getSurvivalFrequency() const {
    return survival_frequency_;
}