/**
 * @file: default.cpp
 *
 * @brief: Default controller which does nothing
 * 
 * @author: Jake Tuero
 * Date: October 2019
 * Contact: tuero@ualberta.ca
 */

#include "default.h"


Default::Default() {}


/*
 * Does nothing.
 */
Action Default::getAction() {
    return Action::noop;
}