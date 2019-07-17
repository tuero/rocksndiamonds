
#include "tree_node_bfs.h"


/*
 * Create new node
 *
 * @param parent -> Pointer to parent node (for reverse traversal in solution)
 * @param actions_from_start -> Actions performed from inital node, used to set simulator
 */
TreeNodeBFS::TreeNodeBFS(TreeNodeBFS* parent, const std::vector<Action> &actions_from_start)
    :  parent(parent), actions_from_start(actions_from_start) 
{
    action = Action::noop;
}


/*
 * Get pointer to parent which generated the current node
 */
TreeNodeBFS* TreeNodeBFS::getParent() {
    return parent;
}


/*
 * Get action which resulted in parent leading to this node
 */
Action TreeNodeBFS::getActionTaken() {
    return action;
}


/*
 * Set the action taken which will lead to this node from the parent
 *
 * @param actionTaken -> Action taken to get to this node
 */
void TreeNodeBFS::setActionTaken(Action actionTaken) {
    action = actionTaken;
    enginehelper::setEnginePlayerAction(actionTaken);

    for (int i = 0; i < enginetype::ENGINE_RESOLUTION; i++) {
        actions_from_start.push_back(action);
        enginehelper::engineSimulateSingle();
    }
}


/*
 * Set the simulator engine state to this current state
 * This will simulate the engine forward using the list of actions taken
 * since the inital reference point.
 */
void TreeNodeBFS::setSimulatorToCurrent() {
    for (auto & element : actions_from_start) {
        enginehelper::setEnginePlayerAction(element);
        enginehelper::engineSimulateSingle();
    }
}