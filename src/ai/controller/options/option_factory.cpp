/**
 * @file: option_factory.cpp
 *
 * @brief: Factory class to get sets of options depending on the needs of the controller.
 * 
 * @author: Jake Tuero
 * Date: September 2019
 * Contact: tuero@ualberta.ca
 */

#include "option_factory.h"

// Option types
#include "option_single_step.h"
#include "option_to_sprite.h"
#include "option_to_rock.h"
#include "option_push_rock.h"
#include "option_wait_rock.h"

// Engine
#include "../../engine/action.h"
#include "../../engine/engine_helper.h"


/*
 * Create options for each of the single step actions.
 */
std::vector<BaseOption*> OptionFactory::createSingleStepOptions() {
    std::vector<BaseOption*> optionPointers;
    options_.clear();

    // Single step option for each available 
    for (Action action : ALL_ACTIONS) {
        std::unique_ptr<OptionSingleStep> option = std::make_unique<OptionSingleStep>(action, 1);
        options_.push_back(std::move(option));
        optionPointers.push_back(options_.back().get());
    }

    return optionPointers;
}


/*
 * Create options for path finding for each sprite on screen
 */
std::vector<BaseOption*> OptionFactory::createPathToSpriteOptions() {
    std::vector<BaseOption*> optionPointers;
    options_.clear();

    // Single step option for each available 
    for (enginetype::GridCell cell : enginehelper::getMapSprites()) {
        int spriteID = enginehelper::getSpriteID(cell);
        std::unique_ptr<OptionToSprite> option = std::make_unique<OptionToSprite>(spriteID);
        options_.push_back(std::move(option));
        optionPointers.push_back(options_.back().get());
    }

    return optionPointers;
}


std::vector<BaseOption*> OptionFactory::createCustomOptions() {
    std::vector<BaseOption*> optionPointers;
    options_.clear();


    // Single step option for each available 
    for (Action action : ALL_ACTIONS) {
        if (action != Action::noop) {
            // std::unique_ptr<OptionSingleStep> option = std::make_unique<OptionSingleStep>(action, 1);
            // options_.push_back(std::move(option));
            // optionPointers.push_back(options_.back().get());
        }
        else {
            std::unique_ptr<OptionSingleStep> option = std::make_unique<OptionSingleStep>(action, 4);
            options_.push_back(std::move(option));
            optionPointers.push_back(options_.back().get());
        }
    }

    for (enginetype::GridCell cell : enginehelper::getMapSprites()) {
        if (enginehelper::getGridItem(cell) == enginetype::FIELD_BOULDER) {
            int spriteID = enginehelper::getSpriteID(cell);

            // To rock
            for (Action direction : ALL_ACTIONS) {
                if (direction == Action::noop || direction == Action::up) {continue;}
                std::unique_ptr<OptionToRock> option = std::make_unique<OptionToRock>(spriteID, direction);
                options_.push_back(std::move(option));
                optionPointers.push_back(options_.back().get());
            }

            // Rock wait
            // !~~ Maybe not needed
            std::unique_ptr<OptionWaitRock> optionWaitRock = std::make_unique<OptionWaitRock>(spriteID);
            options_.push_back(std::move(optionWaitRock));
            optionPointers.push_back(options_.back().get());

            // Rock push
            std::unique_ptr<OptionPushRock> optionLeft = std::make_unique<OptionPushRock>(spriteID, Action::left);
            options_.push_back(std::move(optionLeft));
            optionPointers.push_back(options_.back().get());

            std::unique_ptr<OptionPushRock> optionRight = std::make_unique<OptionPushRock>(spriteID, Action::right);
            options_.push_back(std::move(optionRight));
            optionPointers.push_back(options_.back().get());
        }
        else {
            int spriteID = enginehelper::getSpriteID(cell);
            std::unique_ptr<OptionToSprite> option = std::make_unique<OptionToSprite>(spriteID);
            options_.push_back(std::move(option));
            optionPointers.push_back(options_.back().get());
        }
    }

    return optionPointers;
}