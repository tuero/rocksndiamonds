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
#include <array>
#include <queue>
#include <algorithm>

// Includes
#include "base_controller.h"
#include "base_option.h"


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
    std::vector<BaseOption*> highlevelPlannedPath_ = {};                            // Current path of high level options
    uint64_t currentHighLevelPathHash;                                              // Hash representing the current high level option path
    std::unordered_map<int, std::vector<enginetype::GridCell>> restrictedCellsByOption_;        // Restricted cells for each option pair
    std::unordered_map<int, std::vector<enginetype::GridCell>> knownConstraints_;               // Restricted cells already accounted for during planning (subset of restrictedCellsByOption_)
    
    // Struct for restricted cell
    struct SpriteRestriction {
        int spriteID;                   // Sprite restriction corresponds to
        enginetype::GridCell cell;      // The gridcell to avoid

        bool operator==(const SpriteRestriction &other) const {
            return spriteID == other.spriteID && cell == other.cell;
        }
    };

    // Constraint identification
    typedef std::array<int, 2> OptionIndexPair;                             // Typedef for pairs of options (for return types)
    enginetype::GridCell prevPlayerCell_;                                   // Player cell on the previous game step (Used to find restricted cells on current step)
    enginetype::GridCell currPlayerCell_;                                   // Player cell on the current game step
    std::unordered_map<int, bool> prevIsMoving_;                            // Map of sprites which are moving for the previous game step
    std::unordered_map<int, bool> currIsMoving_;                            // Map of sprites which are moving for the current game step
    std::unordered_map<int, enginetype::GridCell> prevSprites_;             // Map of sprites for each option pair which were active previously
    std::unordered_map<int, enginetype::GridCell> currSprites_;             // Map of sprites for each option pair which are active currently
    std::unordered_map<int, std::vector<SpriteRestriction>> spritesMoved;   // Current list of sprites which moved during player actions for the option pair

    /**
     * Initializations which need to occur BOTH on first level start
     * and on every level restart after a failure.
     */
    void initializationForEveryLevelStart();

    // --------------- HLS --------------- 
    /**
     * Runs one iteration of CBS on the currentHighLevelPathHash.
     * An iteration is counted as a single replay, which will use the restricted cells
     * set in the best node in OPEN, and will insert the children nodes into OPEN for
     * later iterations.
     */
    void CBS();

    void LevinTS();

    /**
     * Check and add new constraints found in previous searches, then
     * find the next high level path to try. Once the path is found, the
     * low level path algorithm is called.
     */
    void highLevelSearch();

    /**
     * Find the path of high level options which corresponds to the collectible sprites
     * in order of (row, col), with the exit at the end. This is a deterministic path
     * that never changes, good for testing sanity.
     */
    void highLevelSearchGemsInOrder();

    // --------------- Constraints --------------- 
    /**
     * Add new constraints for a given pair of options
     * This is called during each step, and adds restrictions based on those found 
     * from checkForMovedObjects()
     */
    void addNewConstraints();

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

    // --------------- Path hashing ---------------
    /**
     * Create hash for all pairs of options.
     * 
     * @return A vector of hash ints representing each pair of options
     */
    std::vector<int> allOptionPairHashes();

    /**
     * Create a hash for the given pair of options of moving from option prevOption to option currOption. 
     * @param currOption The current option (option To)
     * @param prevOption The previous option (option From)
     * @return The hash for the pair of given options (in order)
     */
    int optionPairHash(BaseOption *currOption, BaseOption *prevOption);

    /**
     * Create a hash for each pair of a given path of options.  
     * 
     * @param path The path of options (in order)
     * @return A vector of hashes for each pair (in order) of the given path
     */
    std::vector<int> givenPathOptionPairHashes(std::vector<BaseOption*> path);

    /**
     * Create a hash for a given pair of indices for the master list of options availableOptions_.
     * Hash value is the string concatenation of the 2 indices.
     * 
     * @param indexCurr The index of the option in availableOptions_ for the option To.
     * @param indexPrev The index of the option in availableOptions_ for the option From.
     */
    int optionIndexPairToHash(int indexCurr, int indexPrev);

    /**
     * Get a pair of options represented by the given hash.
     * 
     * @param hash The input hash
     * @return A typedef of array[2], with OptionIndexPair[0] = currOption, OptionIndexPair[1] = prevOption
     */
    OptionIndexPair hashToOptionIndexPair(int hash);

    /**
     * Create a hash for a given path.
     * This hash represents all options in order in the path (can be more than 2).
     * 
     * @param path The given path of options.
     * @return A hash representing the given path of options.
     */
    uint64_t optionPathToHash(std::vector<BaseOption*> path);

public:

    TwoLevelSearch() {}

    TwoLevelSearch(OptionFactoryType optionType) : BaseController(optionType) {}

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

    /*
     * Continue to find the next option the agent should take.
     * 
     *
     * @param currentOption Option which the agent gets to execute.
     * @param nextOption Planned option for the agent to take at the future state.
     */
    // void run(BaseOption **currentOption, BaseOption **nextOption) override;
    void plan() override;

    /**
     * Convey any important details about the controller in string format.
     * @return The controller details in string format.
     */
    std::string controllerDetailsToString() override;

};

#endif  //TWO_LEVEL_SEARCH_H


