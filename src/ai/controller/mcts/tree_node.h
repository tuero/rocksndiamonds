/**
 * @file: tree_node.h
 *
 * @brief: Tree node used in MCTS.
 * 
 * @author: Jake Tuero
 * Date: September 2019
 * Contact: tuero@ualberta.ca
 */

#ifndef TREENODEMCTS_H
#define TREENODEMCTS_H

#include <vector>
#include <memory>

// Engine
#include "../../engine/action.h"
#include "../../engine/game_state.h"
#include "../options/base_option.h"


/**
 * Tree node which MCTS plans over.
 *
 * Tree nodes are responsible for its childrens memory management. 
 */
class TreeNode {
    typedef std::unique_ptr<TreeNode> Pointer;
private:
    TreeNode* parent_;                       // Pointer to parent state
    std::vector<Action> actionsFromStart_;
    std::vector<Pointer> children_;          // current children from this node

    BaseOption* optionTaken_;
    std::vector<BaseOption*> availableOptions_;
    std::vector<BaseOption*> allOptions_;

    // statistics
    int visitCount_;
    int depth_;
    float value_;

    bool isTerminal_;
    bool isFailed_;
    bool isSolved_;

public:

    /*
     * Tree node constructor.
     *
     * @param parent The parent to the node we are creating.
     * @param optionsFromStart The options taken from the start to get to this node.
     */
    TreeNode(TreeNode* parent, const std::vector<Action> &actionsFromStart = {});

    /*
     * Expand the next possible child.
     * 
     * The parent is responsible for the child memory. The child represents the state 
     * which follows from the next available option.
     *
     * @return The new child node which is expanded.
     */
    TreeNode* expand();

    /*
     * Check if all child nodes have been expanded.
     *
     * @return True if all child nodes have been expanded.
     */
    bool allExpanded() const;

    /*
     * Check if the state represented by the tree node is terminal.
     *
     * A tree node is terminal if its failed or solved.
     *
     * @return True if the state represented by the tree node is terminal.
     */
    bool isTerminal() const;

    /*
     * Check if the state represented by the tree node is failed.
     *
     * @return True if the state represented by the tree node is failed.
     */
    bool isFailed() const;

    /*
     * Check if the state represented by the tree node is solved.
     *
     * @return True if the state represented by the tree node is solved.
     */
    bool isSolved() const;

    /*
     * Update the tree node stats.
     *
     * The value is added, and count is incremented.
     *
     * @value The child value being back propagated.
     */
    void updateStats(const float value);

    /*
     * Get the option taken by the tree node.
     *
     * @return The option taken by the tree node.
     */
    BaseOption* getOptionTaken();

    /*
     * Set the options the tree node can represent.
     *
     * @param allOptions The list of options that the tree node can represent.
     */
    void setOptions(std::vector<BaseOption*> &allOptions);

    /*
     * Get the child for a specified index.
     *
     * @param index The child index to query.
     * @return Pointer to the child at the given index.
     */
    TreeNode* getChild(unsigned index);

    /*
     * Get the child for a specified option taken.
     *
     * @param optionTaken Pointer to the option taken.
     * @return Unique pointer to the child for the given option taken.
     */
    Pointer getChild(BaseOption* optionTaken);

    /*
     * Get the parent for the node.
     * 
     * @return Pointer to the parent node.
     */
    TreeNode* getParent();

    /*
     * Set the parent node.
     *
     * @param parent Pointer to the parent.
     */
    void setParent(TreeNode* parent);

    /*
     * Get the number of children already expanded.
     *
     * @return The number of children already expanded.
     */
    std::size_t getChildCount() const;

    /*
     * Get the node value, as calculated by MCTS.
     *
     * @return The node value.
     */
    float getValue() const;

    /*
     * Get the number of times the node has been visited.
     *
     * @return The number of visits to the node.
     */
    int getVisitCount() const;

    /*
     * Get the depth of the node.
     *
     * @return the depth of the node.
     */
    int getDepth() const;


};

#endif  //TREENODEMCTS_H


