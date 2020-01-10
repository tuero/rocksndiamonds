/**
 * @file: option_push_rock.cpp
 *
 * @brief: An option which performs a push of a rock
 * 
 * @author: Jake Tuero
 * Date: September 2019
 * Contact: tuero@ualberta.ca
 */

#include "option_wait_rock.h"

// Includes
#include "engine_types.h"
#include "engine_helper.h"
#include "../../engine/game_state.h"


OptionWaitRock::OptionWaitRock(int spriteID) {
    optionType_ = OptionType::PushRock;
    spriteID_ = spriteID;
    counter_ = 0;
    optionStringName_ += std::to_string(spriteID_);
}

// enginetype::GridCell findRock() {
//     enginetype::GridCell rockCell = {-1, -1};

//     for (enginetype::GridCell cell : enginehelper::getMapSprites()) {
//         if (enginehelper::getGridElement(cell) == enginetype::FIELD_BOULDER) {
//             rockCell.x = cell.x;
//             rockCell.y = cell.y;
//             break;
//         }
//     }
//     return rockCell;
// }

bool OptionWaitRock::run() {
    int loopCounter = 0;
    while(true) {
        goalCell_ = enginehelper::getSpriteGridCell(spriteID_);

        if (enginehelper::getGridMovPos(goalCell_) == 0) {break;}
        enginehelper::setEnginePlayerAction(Action::noop);
        enginehelper::engineSimulate();

        if (loopCounter == 100) {
            // std::cout << "This shouldn't happen" << std::endl;
            break;
        }
        loopCounter += 1;
    }


    timesCalled_ += 1;
    return true;
}


bool OptionWaitRock::getNextAction(Action &action) {
    action = Action::noop;
    return (enginehelper::getGridMovPos(enginehelper::getSpriteGridCell(spriteID_)) == 0);

    // if (enginehelper::getGridMovPos(findRock()) == 0 && counter_ <= 0) {
    //     counter_ = 2;
    // }

    // counter_ -= 1;
    // return (counter_ == 0);
}


bool OptionWaitRock::isValid() {
    if (!enginehelper::isSpriteActive(spriteID_)) {return false;}
    return (enginehelper::getGridMovPos(enginehelper::getSpriteGridCell(spriteID_)) != 0);
}


std::string OptionWaitRock::toString() const {
    return optionStringName_;
}
