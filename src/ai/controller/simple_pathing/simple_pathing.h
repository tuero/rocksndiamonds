/**
 * @file: simple_pathing.h
 *
 * @brief: Sample controller which paths to every possible collectible sprite it can.
 * 
 * @author: Jake Tuero
 * Date: February 2019
 * Contact: tuero@ualberta.ca
 */

#ifndef SIMPLE_PATHING_H
#define SIMPLE_PATHING_H

// Standard Library/STL
#include <string>

// Includes
#include "base_controller.h"
#include "engine_types.h"
#include "base_option.h"


/**
 * Sample controller which paths to the next available collectiple element.
 * 
 * @note: This can only solve levels which are solvable by just pathing, this will fail on 
 *        levels that require intricate movements such as pushing rocks out of the way.
 */
class SimplePathing : public BaseController {
private:
    bool optionStatusFlag_ = true;
    BaseOption* currentOption_; 

    bool nextOptionFound();

public:

    SimplePathing() {}

    SimplePathing(OptionFactoryType optionType) : BaseController(optionType) {}

    /*
     * Handle setup required at level start.
     */
    void handleLevelStart() override;

    /**
     * Get the action from the controller.
     * 
     * @return The next action to execute
     */
    Action getAction() override;

    /**
     * Convey any important details about the controller in string format.
     * @return The controller details in string format.
     */
    std::string controllerDetailsToString() override;
};

#endif  //SIMPLE_PATHING_H