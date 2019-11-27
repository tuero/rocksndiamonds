/**
 * @file: option_push_rock.cpp
 *
 * @brief: An option which performs a push of a rock
 * 
 * @author: Jake Tuero
 * Date: September 2019
 * Contact: tuero@ualberta.ca
 */

#ifndef OPTION_PUSH_ROCK_H
#define OPTION_PUSH_ROCK_H


#include "base_option.h"
#include "../../engine/action.h"

/**
 * Skeleton definition which each derived Option is based off of.
 *
 * All options implemented should be derived from this class. An option is a policy
 * which executes (possible non-deterministic) actions. The controller should determine
 * the type of option to run depending on the state. The method run() are required to 
 * be implemented.
 */
class OptionPushRock : public BaseOption {

private:
    Action action_;
    int counter_;
    std::string optionStringName = "Push boulder: ";

    void findRock();

public:
    
    OptionPushRock(int spriteID, Action action);


    /**
     * Set action which agent should immediately take.
     * 
     * Called when the currentSolution is empty. Agent always executes the next action
     * from currentSolution. If conducting search on the future state while the agent 
     * is conducting the current action, the controller should set currentSolution to
     * the forwardSolution.
     */
    bool run() override;

    bool getNextAction(Action &action) override;

    bool isValid_() override;

    std::string toString() const override;
};



#endif  //OPTION_PUSH_ROCK_H


