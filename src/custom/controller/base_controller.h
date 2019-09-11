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
#include "../engine/action.h"


/**
 * Skeleton definition which each derived controller is based off of.
 *
 * All controllers implemented should be derived from this class. The methods
 * handleEmpty() and run() are required to be implemented, with handleLevelStart() 
 * being optional. 
 */
class BaseController {

public:

    /**
     * Handle setup required at level start.
     * 
     * Called only during level start. Any preprocessing or intiailizations needed for the 
     * controller that wouldn't otherwise be done during each game tick, should be setup here.
     */
    virtual void handleLevelStart() {}

    /**
     * Set action which agent should immediately take.
     * 
     * Called when the currentSolution is empty. Agent always executes the next action
     * from currentSolution. If conducting search on the future state while the agent 
     * is conducting the current action, the controller should set currentSolution to
     * the forwardSolution.
     * 
     * @param currentSolution Actions which the agent gets to execute.
     * @param forwardSolution Planned actions for the agent to take at the future state.
     */
    virtual void handleEmpty(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution) = 0;


    /**
     * Continue to find the next action the agent should take.
     *
     * Called during every game tick. In most cases, controller is planning on the next 
     * future state while agent is conducting the current action. currentSolution holds 
     * the action the agent is currently conducting. Action to be taken once current action 
     * is complete should be put into forwardSolution.
     *
     * @param currentSolution Actions which the agent gets to execute.
     * @param forwardSolution Planned actions for the agent to take at the future state.
     * @param statistics Statistic information of the search performed by the controller.
     */
    virtual void run(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution, 
        std::map<enginetype::Statistics, int> &statistics) = 0;

};



#endif  //BASE_CONTROLLER_H


