/**
 * @file: two_level_search.h
 *
 * @brief: Masters thesis controller.
 * 
 * @author: Jake Tuero
 * Date: November 2019
 * Contact: tuero@ualberta.ca
 */

#ifndef TWO_LEVEL_SEARCH_H
#define TWO_LEVEL_SEARCH_H

// Standard Libary/STL
#include <string>
#include <unordered_map>
#include <vector>
#include <unordered_set>
#include <set>
#include <array>
#include <algorithm>
#include <cmath>                // pow
#include <cstdint>              // fixed-width datatypes

// Includes
#include "tls_node_policy.h"
#include "base_controller.h"
#include "base_option.h"
#include "game_state.h"

// #define SINGLE_PATH
#define SET_RESTRICTIONS
// #define MANUAL_CONSTRAINTS

/**
 * Default controller
 *
 * This does nothing, and acts as a backup to prevent unknown controller input from breaking
 */
class TwoLevelSearch : public BaseController {
private:
    bool optionStatusFlag_ = true;                                                  // Flag signifying current option is complete
    int solutionIndex_;                                                             // Current index in the high level path
    BaseOption* previousOption_;                                                    // Pointer to previous option in high level path
    BaseOption* currentOption_;                                                     // Pointer to current option in high level path

    // HLS 
    std::vector<BaseOption*> highlevelPlannedPath_ = {};                            // Current path of high level options
    uint64_t currentHighLevelPathHash_;                                             // Hash representing the current high level option path
    std::set<NodeLevin, CompareLevinNode> openLevinNodes_;                  // Open list for LevinTS
    std::unordered_map<uint64_t, int> hashPathTimesVisited;                 // Map tracking number of visits for each (partial) path

    // Constraint identification
    std::unordered_map<uint64_t, std::unordered_set<int>> restrictedCellsByOption_;        // Restricted cells for each option pair
    std::unordered_map<uint64_t, std::set<int>> restrictedCellsByPath_;        // Restricted cells for each option pair
    std::unordered_map<uint64_t, int> restrictedCellsByOptionCount_;                // Count of restricted cells for each option, faster access
    typedef std::array<uint64_t, 2> OptionIndexPair;                        // Typedef for pairs of options (for return types)
    std::array<enginetype::GridCell, 2> playerCells_;                       // Player cell on the current/prev game step

    GameState initialState;


    /**
     * Initializations which need to occur BOTH on first level start
     * and on every level restart after a failure.
     */
    void initializationForEveryLevelStart();

    void incrementPathTimesVisited();


    // --------------- HLS --------------- 

    /**
     * Set the constraints for each node on the high-level path, before we 
     * initiaze the low-level search
     * 
     * @param node The Levin node which represents the high-level path.
     */
    void setLowLevelConstraints(const NodeLevin &node);

    /**
     * Modified leveinTS.
     */
    void modifiedLevinTS();

    /**
     * Check and add new constraints found in previous searches, then
     * find the next high level path to try. Once the path is found, the
     * low level path algorithm is called.
     */
    void highLevelSearch();

    void singlePath();

    // --------------- Constraints --------------- 

    void setPathRestrictionSet(uint64_t hash, const std::vector<BaseOption*> &path);

    int restrictionCountForPath(const std::vector<BaseOption*> &path);

    /**
     * Update any nodes in LevinTS open that had a new constraint added on its path.
     * 
     * @param hash The hash of the option pair to check if paths contain
     */
    void updateAffectedLevinNodes(uint64_t hash);

    /**
     * Check for newely moved objects as a result of player actions.
     * Objects are stored as sprite and gridcell pairs for a given pair of options (option from -> option to)
     * In addNewConstraints(), we store only the intersection of gridcells in restrictedCellsByOption_
     */
    void checkForMovedObjects();

    // --------------- Logging --------------- 
    /**
     * Log the given path of gridcells.
     * 
     * @param path The given path of gridcells
     */
    void logPath(const std::vector<enginetype::GridCell> &path);

    /**
     * Log all the available options the controller has access to.
     */
    void logAvailableOptions();

    /**
     * Log the set high level path of options.
     */
    void logHighLevelPath();

    /**
     * Log all the restricted cells found thus far for each pair of options.
     */
    void logRestrictedSprites();

    void logLevinNodes();

public:

    TwoLevelSearch() {}

    TwoLevelSearch(OptionFactoryType optionType) : BaseController(optionType) {}

    void initializeOptions() override;

    /**
     * Reset the options which are available.
     * This is called during level start, as we need to know the sprites available to
     * accurately set what options are available.
     */
    void resetOptions() override;

    /**
     * Flag for controller to try again if level fails.
     */
    bool retryOnLevelFail() const override {return true;}

    /**
     * Handle necessary items before the level gets restarted.
     *  
     */
    void handleLevelRestartBefore() override;

    /**
     * Handle necessary items after the level gets restarted.
     *  
     */
    void handleLevelRestartAfter() override;

    /*
     * Handle setup required at level start.
     * 
     * Called only during level start. Any preprocessing or intiailizations needed for the 
     * controller that wouldn't otherwise be done during each game tick, should be setup here.
     */
    void handleLevelStart() override;

    /**
     * Get the action from the controller.
     * 
     * @param currentOption Option which the agent gets to execute.
     * @param nextOption Planned option for the agent to take at the future state.
     */
    Action getAction() override;

    /**
     * Use this time to check for moved objects.
     */
    void plan() override;

    /**
     * Convey any important details about the controller in string format.
     * @return The controller details in string format.
     */
    std::string controllerDetailsToString() override;

};

#endif  //TWO_LEVEL_SEARCH_H


