/**
 * @file: base_controller.h
 *
 * @brief: Base controller which all implemented controllers should be derived from.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */

#ifndef BASE_CONTROLLER_H
#define BASE_CONTROLLER_H


// Standard Libary/STL
#include <vector>
#include <map>
#include <string>

// Options
#include "base_option.h"
#include "option_factory.h"

// Includes
#include "engine_types.h"
#include "timer.h"


/**
 * Skeleton definition which each derived controller is based off of.
 *
 */
class BaseController {
protected:
    OptionFactory optionFactory_;                       // Factory object to create and manage option memory
    OptionFactoryType optionFactoryType_;               // Type of options to create
    std::vector<BaseOption*> availableOptions_;         // Vector of options available for the controller (set from optionFactory)
    bool requestReset_ = false;                         // Flag to signal a reset of the level
    Timer timer;                                        // Timer for internal testing/real-time tracking

public:

    virtual ~BaseController() {};

    /**
     * Default constructor
     * 
     * The available options to plan over is set by default to be the single step 
     * actions (left, right, up, down, and noop).
     */
    BaseController() : 
        optionFactoryType_(OptionFactoryType::SINGLE_ACTION)
    {}

    /**
     * Constructor with given option factory type.
     * 
     * The available options to plan over is set by the input option factory type.
     * 
     * @param optionFactoryType The specified option factory type to use as the available options.
     */
    BaseController(OptionFactoryType optionFactoryType) : 
        optionFactoryType_(optionFactoryType)
    {}

    /**
     * Initialize the options by asking the factory.
     */
    virtual void initializeOptions() {
        availableOptions_ = optionFactory_.createOptions(optionFactoryType_);
    }

    /**
     * Reset the options which are available.
     * This is called during level start, as we need to know the sprites available to
     * accurately set what options are available.
     */
    virtual void resetOptions() {
        for (auto const & option : availableOptions_) {
            option->reset();
        }
    }

    /**
     * Called on every loop to check if controller wants to reset the level.
     * The underlying flag should be set either in handleEmpty() or run().
     * 
     * @return True if the controller wants to reset the level.
     */
    virtual bool requestRest() {return requestReset_;}

    /**
     * Handle setup required at level start.
     * 
     * Called only during level start. Any preprocessing or intiailizations needed for the 
     * controller that wouldn't otherwise be done during each game tick, should be setup here.
     */
    virtual void handleLevelStart() {requestReset_ = false;}

    /**
     * Flag for controller to try again if level fails.
     * If this is set to true, on level fail (player died, time runs out, etc.),
     * the level will be restarted. Otherwise, the program will terminate.
     */
    virtual bool retryOnLevelFail() const {return false;}

    /**
     * Handle necessary items before the level gets restarted.
     * This is only applicable if the controller handles reattempts. 
     */
    virtual void handleLevelRestartBefore() {};

    /**
     * Handle necessary items after the level gets restarted.
     * This is only applicable if the controller handles reattempts. 
     */
    virtual void handleLevelRestartAfter() {};

    /**
     * Set option which agent should immediately take.
     * 
     * Called every enginetype::ENGINE_RESOLUTION game steps (resolution the
     * player moves by).
     * 
     * @return The action to perform
     */
    virtual Action getAction() = 0;


    /**
     * Called every game step to allow the controller time to plan while
     * executing the current action as taken from getAction().
     * 
     * To ensure the game remaines real-time, plain() should not take longer than ~18ms.
     */
    virtual void plan() {};

    /**
     * Set the available options for the controller to plan with.
     *
     * @param optionFactoryType Enum type to represent set of options to build
     */
    void setAvailableOptions(const OptionFactoryType optionFactoryType) {
        availableOptions_ = optionFactory_.createOptions(optionFactoryType);
    }

    /**
     * Convey any important details about the controller in string format.
     * Useful for logging relevant information about the current controller configuration.
     * 
     * @return The controller details in string format.
     */
    virtual std::string controllerDetailsToString() {return "Default controller";}

};



#endif  //BASE_CONTROLLER_H


