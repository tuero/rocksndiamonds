

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

    int visits_count;
    int depth;
    float value;
    float distance;
    int depth_found;

    void addChild(const Action childAction);

public:

    TreeNode(TreeNode* parent, const std::vector<Action> &actions_from_start = {});

    void setActions();

    bool allExpanded() const;

    Action getActionTaken();

    bool isTerminal() const;

    TreeNode* expand();

    TreeNode* getParent();

    void updateStats(const float value, const int depth_found);

    int getChildCount() const;

    TreeNode* getChild(int index);

    float getValue() const;

    int getVisitCount() const;

    float getDistance() const;

    float getDepth() const;

    float getDepthFound() const;






    TreeNode* getChild(const Action action);

    void setActionTaken(Action actionTaken);

    void setSimulatorToCurrent();


};

#endif  //TREENODEMCTS_H


