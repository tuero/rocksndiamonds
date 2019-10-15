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


#include <vector>
#include <map>
#include <string>

// Options
#include "options/base_option.h"
#include "options/option_factory.h"

// Engine
#include "../engine/action.h"

// Util
#include "../util/timer.h"

//Logging
#include "../util/logging_wrapper.h"
#include <plog/Log.h>   


/**
 * Skeleton definition which each derived controller is based off of.
 *
 * All controllers implemented should be derived from this class. The methods
 * handleEmpty() and run() are required to be implemented, with handleLevelStart() 
 * being optional. 
 */
class BaseController {
protected:
    OptionFactory optionFactory;
    std::vector<BaseOption*> availableOptions_;
    Timer timer;

    /*
     * Long the current state, along with message to consol.
     * 
     * @param msg The message to display before logging the current state.
     * @param sendToConsol Flag to display above message to consol as well.
     */
    void logCurrentState(std::string msg, bool sendToConsol) {
        PLOGD_(logwrap::FileLogger) << msg;
        PLOGD_IF_(logwrap::ConsolLogger, sendToConsol) << msg;

        // logwrap::logPlayerDetails();
        // logwrap::logBoardState();
        // logwrap::logMovPosState();
        // logwrap::logBoardSpriteIDs();
    }

public:

    /*
     * Default constructor
     * 
     * The available options to plan over is set by default to be the single step 
     * actions (left, right, up, down, and noop).
     */
    BaseController() : availableOptions_(optionFactory.createSingleStepOptions()) {}

    /*
     * Handle setup required at level start.
     * 
     * Called only during level start. Any preprocessing or intiailizations needed for the 
     * controller that wouldn't otherwise be done during each game tick, should be setup here.
     */
    virtual void handleLevelStart() {}

    /*
     * Set option which agent should immediately take.
     * 
     * Called when the currentOption is complete. Agent always executes the next option
     * from currentSolution. If conducting search on the future state while the agent 
     * is conducting the current option, the controller should set currentOption to
     * the nextOption.
     * 
     * @param currentOption Option which the agent gets to execute.
     * @param nextOption Planned option for the agent to take at the future state.
     */
    virtual void handleEmpty(BaseOption **currentOption, BaseOption **nextOption) = 0;


    /*
     * Continue to find the next option the agent should take.
     *
     * Called during every game tick. In most cases, controller is planning on the next 
     * future state while agent is conducting the current option. currentOption holds 
     * the option the agent is currently conducting. The option to be taken once 
     * current option is complete should be put into nextOption.
     *
     * @param currentOption Option which the agent gets to execute.
     * @param nextOption Planned option for the agent to take at the future state.
     * @param statistics Statistic information of the search performed by the controller.
     */
    virtual void run(BaseOption **currentOption, BaseOption **nextOption, 
        std::map<enginetype::Statistics, int> &statistics) = 0;

    /*
     * Set the available options for the controller to plan with.
     *
     * @param availableOptions A vector of options the controller will plan over.
     */
    void setAvailableOptions(const std::vector<BaseOption*> &availableOptions) {
        availableOptions_ = availableOptions;
    }


    virtual std::string controllerDetailsToString() {return "";}

};



#endif  //BASE_CONTROLLER_H


