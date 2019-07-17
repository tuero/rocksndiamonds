

#ifndef TREENODEBFS_H
#define TREENODEBFS_H

// ------------- Includes  -------------
#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>

#include "../engine/engine_types.h"
#include "../engine/engine_helper.h"
#include "../engine/action.h"




class TreeNodeBFS {
private:
public:
    TreeNodeBFS* parent;            // Pointer to parent state
    std::vector<Action> actions_from_start;
    Action action;                  // Action which led to this state from parent
    std::vector<Action> actions;    // All possible actions to take from starting node


    /*
     * Create new node
     *
     * @param parent -> Pointer to parent node (for reverse traversal in solution)
     * @param actions_from_start -> Actions performed from inital node, used to set simulator
     */
    TreeNodeBFS(TreeNodeBFS* parent, const std::vector<Action> &actions_from_start = {});

    /*
     * Get pointer to parent which generated the current node
     */
    TreeNodeBFS* getParent();

    /*
     * Get action which resulted in parent leading to this node
     */
    Action getActionTaken();

    /*
     * Set the action taken which will lead to this node from the parent
     *
     * @param actionTaken -> Action taken to get to this node
     */
    void setActionTaken(Action actionTaken);

    /*
     * Set the simulator engine state to this current state
     * This will simulate the engine forward using the list of actions taken
     * since the inital reference point.
     */
    void setSimulatorToCurrent();

};

#endif  //TREENODE


