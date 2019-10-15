/**
 * @file: option_single_step.cpp
 *
 * @brief: An option which performs a single step (action).
 * 
 * @author: Jake Tuero
 * Date: September 2019
 * Contact: tuero@ualberta.ca
 */

#include "option_single_step.h"
#include "option_types.h"

#include "../../engine/engine_helper.h"


OptionSingleStep::OptionSingleStep(Action action, int numTimes) {
    optionType_ = OptionType::SingleStep;
    action_ = action;
    numTimes_ = numTimes;
    counter_ = 0;
}


bool OptionSingleStep::run() {
    for (int i = 0; i < enginetype::ENGINE_RESOLUTION * numTimes_; i++) {
        enginehelper::setEnginePlayerAction(action_);
        enginehelper::engineSimulateSingle();
    }
    return true;
}


bool OptionSingleStep::singleStep(Action &action) {
    action = action_;

    if (counter_ == 0) {
        counter_ = numTimes_;
    }

    counter_ -= 1;
    return (counter_ == 0);
}


bool OptionSingleStep::isValid_() {
    return !enginehelper::isWall(action_);
}


std::string OptionSingleStep::optionToString() {
    return optionStringName + actionToString(action_);
}


// https://stackoverflow.com/questions/1549930/c-equivalent-of-javas-tostring
std::ostream& OptionSingleStep::toString(std::ostream& o) const {
    return o << "Single step action: " << actionToString(action_);
}