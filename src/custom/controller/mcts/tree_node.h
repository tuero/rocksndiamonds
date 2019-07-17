

#ifndef TREENODEMCTS_H
#define TREENODEMCTS_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>
#include <cassert>

#include "../../engine/engine_helper.h"
#include "../../engine/action.h"
#include "../../engine/game_state.h"



class TreeNode {
    typedef std::unique_ptr<TreeNode> Pointer;
private:
    TreeNode* parent;                       // Pointer to parent state
    std::vector<Action> actions_from_start;
    Action action_taken;                    // Action which led to this state from parent
    std::vector<Pointer> children;          // current children from this node
    std::vector<Action> actions;

    // statistics
    int visits_count;
    int depth;
    float value;
    int distance_to_goal;
    float count_goals_found;
    float count_times_died;

    bool is_terminal;
    bool is_deadly;
    bool is_solved;

    float is_safe;

    void addChild(const Action childAction);

public:

    TreeNode(TreeNode* parent, const std::vector<Action> &actions_from_start = {});

    void setActions();

    bool allExpanded() const;

    Action getActionTaken();

    bool getTerminalStatusFromEngine() const;

    bool getDeadlyStatusFromEngine() const;

    bool getSolvedStatusFromEngine() const;

    bool isTerminal() const;

    bool isDeadly() const;

    bool isSolved() const;

    TreeNode* expand();

    TreeNode* getParent();

    void updateStats(const float goal_found, const float died, const float countSafe);

    TreeNode* getChild(unsigned index);

    Pointer getChildByAction(Action action);


    // ------------------- Statistic Getters ---------------------

    unsigned int getChildCount() const;

    float getValue() const;

    int getVisitCount() const;

    int getDistance() const;

    int getDepth() const;

    float getGoalCount() const;

    float getDiedCount() const;

    float getIsSafe() const;




    void setParent(TreeNode* parent);

    TreeNode* getChild(const Action action);

    void setActionTaken(Action actionTaken);

    void setSimulatorToCurrent();


};

#endif  //TREENODEMCTS_H


