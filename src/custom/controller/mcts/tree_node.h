

#ifndef TREENODEMCTS_H
#define TREENODEMCTS_H

#include <vector>
#include <memory>

// Engine
#include "../../engine/action.h"
#include "../../engine/game_state.h"



class TreeNode {
    typedef std::unique_ptr<TreeNode> Pointer;
private:
    TreeNode* parent_;                       // Pointer to parent state
    std::vector<Action> actionsFromStart_;
    Action actionTaken_;                    // Action which led to this state from parent
    std::vector<Pointer> children_;          // current children from this node
    std::vector<Action> actions_;

    // statistics
    int visitCount_;
    int depth_;
    float value_;

    bool is_terminal;
    bool is_deadly;
    bool is_solved;

    void addChild(const Action childAction);

public:

    TreeNode(TreeNode* parent, const std::vector<Action> &actionsFromStart = {});

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

    void updateStats(const float value);

    TreeNode* getChild(unsigned index);

    Pointer getChildByAction(Action action);


    // ------------------- Statistic Getters ---------------------

    std::size_t getChildCount() const;

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


