

#ifndef PFA_TREENODEMCTS_H
#define PFA_TREENODEMCTS_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>
#include <cassert>
#include <cmath>

#include "../../engine/engine_helper.h"
#include "../../engine/action.h"
#include "../../engine/game_state.h"



class PFATreeNode {
    typedef std::unique_ptr<PFATreeNode> Pointer;
private:
    PFATreeNode* parent;                       // Pointer to parent state
    std::vector<Action> actions_from_start;
    Action action_taken;                    // Action which led to this state from parent
    std::vector<Pointer> children;          // current children from this node
    std::vector<Action> actions;

    int depth;
    float value;

    // statistics
    int visits_count;
    int distance_min_goal;                   // distance to goal when expanded
    int distance_l1_goal;
    float survival_frequency_;                // frequency of deaths when expanded
    int depth_goal_found;

    int distance_sum_;
    float count_goals_found;
    float count_times_died;

    bool is_terminal;
    bool is_deadly;
    bool is_solved;

    float is_safe;

    void addChild(const Action childAction);

public:

    PFATreeNode(PFATreeNode* parent, const std::vector<Action> &actions_from_start = {});

    void setActions(std::vector<enginetype::GridCell> &allowed_cells);

    bool allExpanded() const;

    Action getActionTaken();

    bool getTerminalStatusFromEngine() const;

    bool getDeadlyStatusFromEngine() const;

    bool getSolvedStatusFromEngine() const;

    bool isTerminal() const;

    bool isDeadly() const;

    bool isSolved() const;

    PFATreeNode* expand(std::vector<enginetype::GridCell> &allowed_cells);

    PFATreeNode* getParent();

    void updateStats(const float goal_found, const float died, const int depth, 
        const float countSafe, const int min_distance_found);

    PFATreeNode* getChild(unsigned index);

    Pointer getChildByAction(Action action);


    // ------------------- Statistic Getters ---------------------

    unsigned int getChildCount() const;

    float getValue() const;

    int getVisitCount() const;

    int getMinDistance() const;

    int getL1Distance() const;

    int getDepth() const;

    float getGoalCount() const;

    float getDiedCount() const;

    float getIsSafe() const;

    int getDistanceSum() const;

    int getMinDepthToGoal() const;

    float getSurvivalFrequency() const;


    void updateMinDistance(int min_distance_found);

    void setParent(PFATreeNode* parent);

    PFATreeNode* getChild(const Action action);

    void setActionTaken(Action actionTaken);

    void setSimulatorToCurrent();


};

#endif  //PFA_TREENODEMCTS_H


