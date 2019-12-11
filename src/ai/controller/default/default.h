/**
 * @file: default.h
 *
 * @brief: Default controller which does nothing
 * 
 * @author: Jake Tuero
 * Date: October 2019
 * Contact: tuero@ualberta.ca
 */

#ifndef DEFAULT_H
#define DEFAULT_H


#include "../base_controller.h"

// Engine
#include "../../engine/engine_types.h"


/**
 * Default controller
 *
 * This does nothing, and acts as a backup to prevent unknown controller input from breaking
 */
class Default : public BaseController {
private:

public:

    Default();

    /**
     * Always returns the noop action.
     * 
     * @return Action noop.
     */
    Action getAction() override;
};

#endif  //DEFAULT_H


