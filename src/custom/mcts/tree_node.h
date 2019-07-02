

#ifndef TREENODEMCTS_H
#define TREENODEMCTS_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>
#include <cassert>

#include "../engine/engine_helper.h"
#include "../engine/action.h"



class TreeNode {
    typedef std::unique_ptr<TreeNode> Pointer;
private:
    TreeNode* parent;                       // Pointer to parent state
    std::vector<Action> actions_from_start;
    Action action_taken;                    // Action which led to this state from parent
    std::vector<Pointer> children;          // current children from this node
    std::vector<Action> actions;

    int num_visits;
    int depth;
    float value;

    void addChild(const Action childAction);

public:

    TreeNode(TreeNode* parent, const std::vector<Action> &actions_from_start = {});

    void setActions();

    bool allExpanded() const;

    Action getActionTaken();

    bool isTerminal() const;

    TreeNode* expand();








    TreeNode* getParent();

    TreeNode* getChild(const Action action);

    void setActionTaken(Action actionTaken);

    void setSimulatorToCurrent();


};

#endif  //TREENODEMCTS_H


