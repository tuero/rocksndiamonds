/**
 * @file: base_option.h
 *
 * @brief: Base option which all implemented options should be derived from.
 * 
 * @author: Jake Tuero
 * Date: September 2019
 * Contact: tuero@ualberta.ca
 */

#ifndef BASE_OPTION_H
#define BASE_OPTION_H


// Standard Libary/STL
#include <string>
#include <deque>
#include <unordered_set>
#include <vector>

// Option type enum
#include "option_types.h"

// Includes
#include "engine_types.h"
#include "engine_helper.h"


/**
 * Skeleton definition which each derived Option is based off of.
 *
 * All options implemented should be derived from this class. An option is a policy
 * which executes (possible non-deterministic) actions. The controller should determine
 * the type of option to run depending on the state. The method run() are required to 
 * be implemented.
 */
class BaseOption {
protected:
    OptionType optionType_ = OptionType::Misc;                  // Option enum type, used to group related options
    enginetype::GridCell goalCell_;                             // The gridcell associated with A* as a solution.
    std::deque<enginetype::GridCell> solutionPath_  = {};       // Path found by A*
    std::vector<enginetype::GridCell> restrictedCells_ = {};    // Container of restricted cells A* must avoid
    // A* priority settings
    bool avoidNonGoalCollectibleCells = false;                  // Flag to avoid collectible cells which are not the current goal.
    bool prioritizeSafeCells = false;                           // Flag to avoid cells which may cause the player to immediately die.
    int spriteID_ = -1;                                         // Associated sprite for option
    int timesCalled_ = 0;                                       // Number of times called
    int counter_ = 0;                                           // Internal counter for tracking actions taking multiple game ticks

    /**
     * Set the Options solutionPath as the path found during A* 
     * at the grid level (time-independent).
     * 
     * No goalCell is provided, so the currently saved cell is used 
     * (this might be a static grid on the map no related to sprites,
     * and we may not want this changed).
     */
    void runAStar();

    /**
     * Set the Options solutionPath as the path found during A* 
     * at the grid level (time-independent).
     * 
     * @param startCell The gridcell designated as the start for A*.
     * @param goalCell The gridcell designated as the goal for A*.
     */
    void runAStar(enginetype::GridCell startCell, enginetype::GridCell goalCell);

public:

    virtual ~BaseOption() {};

    /**
     * Run the action(s) defined by the option, usually done during simulation (to
     * forward the search tree).
     * 
     * The definition of the options can be their own programs in the sense that they can
     * either step forward a single step, or perform many actions such as moving to a 
     * sprite of particular interest. It is up to the Option implementation to handle
     * events such as forwarding the engine.
     * 
     * @return True if the option was able to perform is actions without error, false otherwise.
     */
    virtual bool run() = 0;

    /**
     * Queries the next action to perform from the option, and signals if the option is complete.
     * 
     * This doesn't forward the engine game state. It should however forward the internal
     * representation of the option. For example, if the option is to apply the action
     * LEFT 4 times, calling once should decrease an internal counter of the option
     * from 4 to 3. Once the option has peformed its last move, it should signal its complete
     * by returning true.
     * 
     * @param action Reference to action which will be set by the option to perform
     * @return True if the option has determined it is complete, false otherwise.
     */
    virtual bool getNextAction(Action &action) = 0;

    /**
     * Determine if the option is valid to perform as per the current game engine state.
     * 
     * @return True if the option is valid, false otherwise.
     */
    virtual bool isValid() = 0;

    /**
     * String representation of the option and its characteristics
     * 
     * Used for logging. The string should decribe the option behaviour as well as any sprite it
     * is associated with. For example, "Walk to sprite: Diamond". 
     * 
     * @return String representation of option.
     */
    virtual std::string toString() const = 0;

    /**
     * Reset the option.
     * Resetting includes finding the cell that corresponds to the sprite, and 
     * clearing the solution path and restricted cells.
     */
    void reset();

    /**
     * Set the flag which indicates whether A* will avoid collectible elements which are
     * not the current goal location.
     */
    void setAvoidNonGoalCollectibleCells(bool flag);

    /**
     * Set the flag which indicates whether A* will avoid cells which can cause the
     * player to explode.
     */
    void setPrioritizeSafeCells(bool flag);

    /**
     * Get the solution path as found by A*.
     * 
     * @return A deque of gridcells representing the found path.
     */
    std::deque<enginetype::GridCell> getSolutionPath();

    /**
     * Set the restricted cells.
     * 
     * @param restrictedCells A vector of restricted cells.
     */
    void setRestrictedCells(std::vector<enginetype::GridCell> &restrictedCells);

    /**
     * Set the restricted cells.
     * 
     * @param restrictedCells A set of restricted cells indices.
     */
    void setRestrictedCells(std::unordered_set<int> &restrictedCells);

    /**
     * Get the list of restricted cells.
     * 
     * @return The vector of restricted cells.
     */
    const std::vector<enginetype::GridCell> & getRestrictedCells();

    /**
     * Clear the saved restricted cells.
     */
    void clearRestrictedCells();

    /**
     * Add a restricted cell.
     * 
     * @param The gridcell to add as a restriction.
     */
    void addRestrictedCell(enginetype::GridCell &cell);

    /**
     * Add a restricted cell.
     * 
     * @param The index representing the cell to add as a restriction.
     */
    void addRestrictedCell(int index);

    /**
     * Get the option type.
     * 
     * @return The option category type.
     */
    OptionType getOptionType() const;

    /**
     * Increment the number of times the option has been expanded during search.
     */
    void incrementTimesCalled();

    /**
     * Reset the number of times the option has been expanded during search.
     */
    void resetTimesCalled();

    /**
     * Get the number of times the option has been expanded during search.
     *  
     * @return The number of times option has been expanded.
     */
    int getTimesCalled() const;

    /**
     * Set the sprite ID that the option represents/interacts with.
     * 
     * @param spriteID The unique sprite ID to set.
     */
    void setSpriteID(int spriteID);

    /**
     * Get the sprite ID that the option represents/interacts with.
     * 
     * @return The unique sprite ID represented by the option.
     */
    int getSpriteID() const;
};



#endif  //BASE_OPTION_Hd


