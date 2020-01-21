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


// Standard Libary/STL
#include <vector>
#include <string>

// Controllers
#include "controller_listing.h"
#include "base_controller.h"

// Includes
#include "engine_types.h"


/**
 * Controller interface which is responsible for which methods of implemented controllers
 * should be called upon various events.
 */
class Controller {
private:
    int step_counter_ = 0;                                  // Current engine game step
    std::vector<Action> currentAction_;                     // Current action which is being sent to engine
    std::unique_ptr<BaseController> baseController_;        // Pointer to controller being used
    std::vector<std::string> actionsTaken_;                 // Vector of string actions for playthrough which solves level

public:
    
    /**
     * Default constructor which gets the controller type from the engine
     */
    Controller();

    /**
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

    /**
     * Called when the level is started for the first time.
     * Special initializations in which we only want to occur once.
     */
    void handleFirstLevelStart();

    /**
     * Reset the controller, which is called at every level start.
     */
    void reset();

    /**
     * Get the integer action code from the controller.
     * 
     * @return The integer action code from the controller.
     */
    int getAction();

    /**
     * Inits the controller.
     * Controller type is determined by command line argument.
     */
    void initController();

    /**
     * Inits the controller.
     *
     * @param controller Controller enum type.
     */
    void initController(ControllerType controller);

};



#endif  //CONTROLLER_H


