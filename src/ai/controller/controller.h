/**
 * @file: controller.h
 *
 * @brief: Controller interface which every controller is based on.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */

#ifndef CONTROLLER_H
#define CONTROLLER_H


#include <vector>
#include <map>
#include <memory>

// Controllers
#include "base_controller.h"
#include "options/base_option.h"
#include "options/option_single_step.h"
#include "options/option_factory.h"

// Engine and typedefs
#include "../engine/action.h"           // Action enum
#include "../engine/engine_types.h"


/**
 * Controller interface which 
 *
 * All controllers implemented should be derived from this class. The methods
 * handleEmpty() and run() are required to be implemented, with handleLevelStart() 
 * being optional. 
 */
class Controller {
private:
    int step_counter_ = 0;

    // !TODO --> Make this a proper struct?
    std::map<enginetype::Statistics, int> statistics_ = {{enginetype::RUN_TIME, 0},
                                                        {enginetype::COUNT_EXPANDED_NODES, 0}, 
                                                        {enginetype::COUNT_SIMULATED_NODES, 0},
                                                        {enginetype::MAX_DEPTH, 0}
                                                       };
    std::vector<Action> currentSolution_;
    std::vector<Action> forwardSolution_;
    BaseOption* currentOption_;
    BaseOption* nextOption_;
    std::unique_ptr<BaseController> baseController_;
    OptionFactory optionFactory_;
    bool optionStatusFlag_;

    void logNextOptionDetails();


public:
    /*
     * Default constructor which gets the controller type from the engine
     */
    Controller();

    /*
     * Constructor which sets the controller based on a given controller type
     * Used for testing
     */
    Controller(enginetype::ControllerType controller);

    /*
     * Reset the controller, which is called at level start.
     */
    void reset();

    /*
     * Get the action from the controller.
     */
    Action getAction();

    /*
     * Get the runtime of the controller.
     */
    int getRunTime();

    /*
     * Get the number of nodes expanded by the type of tree search used.
     */
    int getCountExpandedNodes();

    /*
     * Get the number of nodes simulated by the type of tree search used.
     */
    int getCountSimulatedNodes();

    /*
     * Get the max depth of nodes expanded by the type of tree search used.
     */
    int getMaxDepth();

    /*
     * Inits the controller.
     * Controller type is determined by command line argument.
     *
     * @param controller Controller enum type 
     */
    void initController(enginetype::ControllerType controller = enginetype::ControllerType::DEFAULT);

};



#endif  //CONTROLLER_H


