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
#include <deque>
#include <unordered_map>
#include <vector>
#include <array>
#include <queue>
#include <algorithm>

// Includes
#include "base_controller.h"
#include "base_option.h"

#include "../../engine/game_state_abstract.h"

/**
 * Default controller
 *
 * This does nothing, and acts as a backup to prevent unknown controller input from breaking
 */
class TwoLevelSearch : public BaseController {
private:
    bool optionStatusFlag_ = true;                                                  // Flag signifying current option is complete
    int solutionIndex_;                                                             // Current index in the high level path
    BaseOption* currentOption_;                                                     // Pointer to current option in high level path
    std::deque<BaseOption*> highlevelPlannedPath_ = {};                             // Path of high level options
    uint64_t currentHighLevelPathHash;
    std::unordered_map<BaseOption*, std::vector<enginetype::GridCell>> restrictedCellsByOption_;        // Restricted cells for each option
    std::unordered_map<BaseOption*, std::vector<enginetype::GridCell>> knownConstraints_;
    
    uint64_t hash = 0;                                                              // Hash of the current option being attempted
    std::unordered_map<uint64_t, AbstractGameState> storedPathStates_;              // Cached previous paths attempted to avoid duplicate attempts

    // Struct for restricted cell
    struct SpriteRestriction {
        int spriteID;                   // Sprite restriction corresponds to
        enginetype::GridCell cell;      // The gridcell to avoid

        bool operator==(const SpriteRestriction &other) const {
            return spriteID == other.spriteID && cell == other.cell;
        }
    };

    // High level costs


    // Constraint identification
    typedef std::array<int, 2> OptionIndexPair;
    enginetype::GridCell prevPlayerCell_;                               // Player cell on the previous game step (Used to find restricted cells on current step)
    enginetype::GridCell currPlayerCell_;                               // Player cell on the current game step
    std::unordered_map<int, bool> prevIsMoving_;                        // Map of sprites which are moving for the previous game step
    std::unordered_map<int, bool> currIsMoving_;                        // Map of sprites which are moving for the current game step
    std::unordered_map<int, enginetype::GridCell> prevSprites_;
    std::unordered_map<int, enginetype::GridCell> currSprites_;
    std::deque<enginetype::GridCell> lowlevelPlannedPath_;              // Path of individual grid cells for the current option
    std::unordered_map<BaseOption*, std::vector<SpriteRestriction>> spritesMoved; // Current list of sprites which moved during player actions
    // std::unordered_map<uint64_t, std::vector<SpriteRestriction>> spritesMoved; // Current list of sprites which moved during player actions

    struct HighLevelNode {
        BaseOption *id;                             // Fast access node ID = gridcell index 
        BaseOption *parentId;                       // node ID for the parent node
        BaseOption *option;                 // Represented data member to search over
        double g;                            // g-value used in A*
        double h;                            // h-value used in A* (Euclidean distance)
    };

    // Custom comparator for priority queue 
    class CompareHighLevelNode {
        public:
            bool operator() (HighLevelNode left, HighLevelNode right) {
                return (left.g + left.h) > (right.g + right.h);
            }
    };

    void initializationForEveryLevelStart();

    // HLS
    void CBS();

    void LevinTS();

    void highLevelSearchGemsInOrder();

    void smartAStar();

    void highLevelSearch();

    // Constraints
    void addNewConstraints();

    void checkForMovedObjects();

    // Logging
    void logPath(const std::deque<enginetype::GridCell> &path);

    void logAvailableOptions();

    void logHighLevelPath();

    void logRestrictedSprites();

    // Path hashing
    int optionIndexPairToHash(int indexCurr, int indexPrev);

    OptionIndexPair hashToOptionIndexPair(int hash);

    uint64_t optionPathToHash(std::deque<BaseOption*> path);

    void findUnseenPath();

    void saveCurrentPathHash();

public:

    TwoLevelSearch() {}

    TwoLevelSearch(OptionFactoryType optionType) : BaseController(optionType) {}

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


