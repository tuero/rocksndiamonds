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
#include "controller_listing.h"
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
    std::vector<Action> currentAction_;
    Action nextAction_;
    std::unique_ptr<BaseController> baseController_;



public:
    /*
     * Default constructor which gets the controller type from the engine
     */
    Controller();

    /*
     * Constructor which sets the controller based on a given controller type
     * Used for testing.
     * 
     * @param controller Controller type.
     */
    Controller(ControllerType controller);

    /**
     * Check if the controller wants to request a reset.
     * 
     * @return True if the controller wants to request a rest.
     */
    bool requestReset();

    /**
     * Called when level is solved.
     * This will close logging and cleanup.
     */
    void handleLevelSolved();

    /**
     * Called when level is failed. 
     * Depending on the controller, this will either terminate or attempt the level again.
     */
    void handleLevelFailed();

    /*
     * Reset the controller, which is called at level start.
     */
    void reset();

    /*
     * Get the action from the controller.
     */
    Action getAction();

    /*
     * Inits the controller.
     * Controller type is determined by command line argument.
     */
    void initController();

    /*
     * Inits the controller.
     *
     * @param controller Controller enum type 
     */
    void initController(ControllerType controller);

};



#endif  //CONTROLLER_H


