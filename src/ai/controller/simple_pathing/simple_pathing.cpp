/**
 * @file: simple_pathing.cpp
 *
 * @brief: Sample controller which paths to every possible collectible sprite it can.
 * 
 * @author: Jake Tuero
 * Date: February 2019
 * Contact: tuero@ualberta.ca
 */


#include "simple_pathing.h"

// Standard Libary/STL
#include <vector>
#include <algorithm>            // min_element

// Include
#include "logger.h"


std::string SimplePathing::controllerDetailsToString() {
    return "Simple pathing sample controller";
}


void SimplePathing::handleLevelStart() {
    optionStatusFlag_ = true;
}


bool SimplePathing::nextOptionFound() {
    // Find options which are valid
    std::vector<BaseOption*> validOptions;
    for (auto const & option : availableOptions_) {
        if (option->isValid()) {
            validOptions.push_back(option);
        }
    }

    // No valid options
    if (validOptions.empty()) {return false;}

    // Find option with shortest A* path distance
    currentOption_ = *std::min_element(validOptions.begin(), validOptions.end(), 
        [] (BaseOption* lhs, BaseOption* rhs) {return lhs->getSolutionPath().size() < rhs->getSolutionPath().size();}
    );
    return true;
}


Action SimplePathing::getAction() {
    // Current pathing option is complete, find next one
    if (optionStatusFlag_) {
        // Try and find next option
        if (!nextOptionFound()) {
            PLOGE_(logger::ConsoleLogger) << "No valid options, exiting";
            PLOGE_(logger::FileLogger) << "No valid options, exiting";
            throw std::exception();
        }

        PLOGI_(logger::ConsoleLogger) << "Next option to execute: " << currentOption_->toString();
        PLOGI_(logger::FileLogger) << "Next option to execute: " << currentOption_->toString();
    }

    // Get next action
    Action action = Action::noop;
    optionStatusFlag_ = true;

    // If option still valid (i.e. not collected) then continue getting action
    if (currentOption_->isValid()) {
        optionStatusFlag_ = currentOption_->getNextAction(action);
    }

    return action;
}