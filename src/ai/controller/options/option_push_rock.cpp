/**
 * @file: option_push_rock.cpp
 *
 * @brief: An option which performs a push of a rock
 * 
 * @author: Jake Tuero
 * Date: September 2019
 * Contact: tuero@ualberta.ca
 */

#include "option_push_rock.h"
#include "option_types.h"
#include "../../engine/engine_helper.h"


OptionPushRock::OptionPushRock(int spriteID, Action action) {
    optionType_ = OptionType::PushRock;
    spriteID_ = spriteID;
    action_ = action;
    counter_ = 0;
}


bool OptionPushRock::run() {
    for (int i = 0; i < enginetype::ENGINE_RESOLUTION * 2; i++) {
        enginehelper::setEnginePlayerAction(action_);
        enginehelper::engineSimulateSingle();
    }
    for (int i = 0; i < enginetype::ENGINE_RESOLUTION; i++) {
        enginehelper::setEnginePlayerAction(Action::noop);
        enginehelper::engineSimulateSingle();
    }


    // enginetype::GridCell goalCell_ = enginehelper::getSpriteGridCell(spriteID_);
    // while (enginehelper::getGridMovPos(goalCell_) == 0) {
    //     for (int i = 0; i < enginetype::ENGINE_RESOLUTION * 2; i++) {
    //         enginehelper::setEnginePlayerAction(action_);
    //         enginehelper::engineSimulateSingle();
    //     }
    // }
    // while (enginehelper::getGridMovPos(goalCell_) != 0) {
    //     for (int i = 0; i < enginetype::ENGINE_RESOLUTION * 2; i++) {
    //         enginehelper::setEnginePlayerAction(action_);
    //         enginehelper::engineSimulateSingle();
    //     }
    // }


    timesCalled_ += 1;
    return true;
}


bool OptionPushRock::getNextAction(Action &action) {
    action = action_;
    // enginetype::GridCell goalCell_ = enginehelper::getSpriteGridCell(spriteID_);

    // // Attempting to push
    // if (enginehelper::getGridMovPos(goalCell_) == 0 && counter_ == 0) {
    //     counter_ = 1;
    // }

    // // Currently pushing
    // if (enginehelper::getGridMovPos(goalCell_) != 0 && counter_ == 1) {
    //     counter_ = 2;
    // }

    // // Stopped pushing
    // if (enginehelper::getGridMovPos(goalCell_) == 0 && counter_ == 2) {
    //     counter_ = 0;
    // }

    // return (counter_ == 0);


    if (counter_ == 0) {
        counter_ = 3;
    }
    if (counter_ == 1) {
        action = Action::noop;
    }

    counter_ -= 1;
    return (counter_ == 0);
}


bool OptionPushRock::isValid_() {
    if (!enginehelper::isSpriteActive(spriteID_)) {return false;}
    
    bool valid = false;

    enginetype::GridCell cell = enginehelper::getPlayerPosition();
    enginetype::GridCell goalCell_ = enginehelper::getSpriteGridCell(spriteID_);

    // std::cout << enginehelper::getGridElement((enginetype::GridCell){cell.x, cell.y}) << " ";
    // std::cout << enginehelper::getGridElement((enginetype::GridCell){cell.x+1, cell.y}) << " ";
    // std::cout << enginehelper::getGridElement((enginetype::GridCell){cell.x+2, cell.y}) << " ";
    // std::cout << std::endl;

    if (action_ == Action::left) {
        valid = ((enginetype::GridCell){cell.x-1, cell.y} == goalCell_ && 
                 enginehelper::getGridElement((enginetype::GridCell){cell.x-2, cell.y}) == enginetype::FIELD_EMPTY &&
                 enginehelper::getGridMovPos((enginetype::GridCell){cell.x-1, cell.y}) == 0);
        // valid = (enginehelper::getGridElement((enginetype::GridCell){cell.x-1, cell.y}) == enginetype::FIELD_BOULDER && 
        //          enginehelper::getGridElement((enginetype::GridCell){cell.x-2, cell.y}) == enginetype::FIELD_EMPTY &&
        //          enginehelper::getGridMovPos((enginetype::GridCell){cell.x-1, cell.y}) == 0);
    }
    else if (action_ == Action::right) {
        valid = ((enginetype::GridCell){cell.x+1, cell.y} == goalCell_ && 
                 enginehelper::getGridElement((enginetype::GridCell){cell.x+2, cell.y}) == enginetype::FIELD_EMPTY &&
                 enginehelper::getGridMovPos((enginetype::GridCell){cell.x+1, cell.y}) == 0);
        // valid = (enginehelper::getGridElement((enginetype::GridCell){cell.x+1, cell.y}) == enginetype::FIELD_BOULDER && 
        //          enginehelper::getGridElement((enginetype::GridCell){cell.x+2, cell.y}) == enginetype::FIELD_EMPTY &&
        //          enginehelper::getGridMovPos((enginetype::GridCell){cell.x+1, cell.y}) == 0);
    }

    return valid;
}


std::string OptionPushRock::toString() const {
    return optionStringName + actionToString(action_);
}
