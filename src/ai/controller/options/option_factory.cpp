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
#include "option_collectible_sprite.h"
#include "option_to_exit.h"
#include "option_door.h"

// Includes
#include "engine_types.h"
#include "engine_helper.h"
#include "logger.h"

using namespace enginehelper;

/**
 * Create options for each of the single step actions.
 */
std::vector<BaseOption*> OptionFactory::createSingleActionOptions() {
    std::vector<BaseOption*> optionPointers;
    options_.clear();

    // Single action for each available 
    for (Action action : enginetype::ALL_ACTIONS) {
        std::unique_ptr<OptionSingleStep> option = std::make_unique<OptionSingleStep>(action, 1);
        options_.push_back(std::move(option));
        optionPointers.push_back(options_.back().get());
    }

    return optionPointers;
}


/**
 * Create options for each of the single step actions (excluding NOOP).
 */
std::vector<BaseOption*> OptionFactory::createSingleActionNoNoopOptions() {
    std::vector<BaseOption*> optionPointers;
    options_.clear();

    // Single action for each available 
    for (Action action : enginetype::ALL_ACTIONS_NO_NOOP) {
        std::unique_ptr<OptionSingleStep> option = std::make_unique<OptionSingleStep>(action, 1);
        options_.push_back(std::move(option));
        optionPointers.push_back(options_.back().get());
    }

    return optionPointers;
}


/**
 * Create options for path finding for each sprite on screen
 */
std::vector<BaseOption*> OptionFactory::createPathToSpriteOptions() {
    std::vector<BaseOption*> optionPointers;
    options_.clear();

    for (enginetype::GridCell cell : gridinfo::getMapSprites()) {
        int spriteID = gridinfo::getSpriteID(cell);
        // Collectible sprite (diamond in this case)
        if (elementproperty::isCollectable(cell)) {
            std::unique_ptr<OptionCollectibleSprite> option = std::make_unique<OptionCollectibleSprite>(spriteID);
            options_.push_back(std::move(option));
            optionPointers.push_back(options_.back().get());
        }
        // Exit
        else if (elementproperty::isExit(cell)) {
            std::unique_ptr<OptionToExit> option = std::make_unique<OptionToExit>(spriteID);
            options_.push_back(std::move(option));
            optionPointers.push_back(options_.back().get());
        }
    }
    return optionPointers;
}


/**
 * Create options for the two level search.
 * This uses pathing to collectible sprites (diamonds, keys, etc.) and to exit.
 * Used for Masters thesis.
 */
std::vector<BaseOption*> OptionFactory::createTwoLevelSearchOptions() {
    std::vector<BaseOption*> optionPointers;
    options_.clear();

    for (enginetype::GridCell cell : gridinfo::getMapSprites()) {
        int spriteID = gridinfo::getSpriteID(cell);
        // Collectible sprite (diamond in this case)
        if (elementproperty::isCollectable(cell)) {
            std::unique_ptr<OptionCollectibleSprite> option = std::make_unique<OptionCollectibleSprite>(spriteID);
            options_.push_back(std::move(option));
            optionPointers.push_back(options_.back().get());
        }
        // Exit
        else if (elementproperty::isExit(cell)) {
            std::unique_ptr<OptionToExit> option = std::make_unique<OptionToExit>(spriteID);
            options_.push_back(std::move(option));
            optionPointers.push_back(options_.back().get());
        }
        // Door
        else if (elementproperty::isGate(cell)) {
            std::unique_ptr<OptionDoor> option = std::make_unique<OptionDoor>(spriteID);
            options_.push_back(std::move(option));
            optionPointers.push_back(options_.back().get());
        }
    }

    return optionPointers;
}


/**
 * @note Your custom option grouping here.
 */
std::vector<BaseOption*> OptionFactory::createCustomOptions() {
    std::vector<BaseOption*> optionPointers;
    options_.clear();
    return optionPointers;
}


/**
 * Create options for use in search. 
 * 
 * @note Here is where you can define similar functions to the above for new controllers.
 */
std::vector<BaseOption*> OptionFactory::createOptions(OptionFactoryType optionFactoryType) {
    optionFactoryType_ = optionFactoryType;
    switch (optionFactoryType) {
        case OptionFactoryType::SINGLE_ACTION:
            return createSingleActionOptions();
        case OptionFactoryType::SINGLE_ACTION_NO_NOOP:
            return createSingleActionNoNoopOptions();
        case OptionFactoryType::PATH_TO_SPRITE:
            return createPathToSpriteOptions();
        case OptionFactoryType::TWO_LEVEL_SEARCH :
            return createTwoLevelSearchOptions();
        case OptionFactoryType::CUSTOM :
            return createCustomOptions();
        default :
            optionFactoryType_ = OptionFactoryType::SINGLE_ACTION;
            PLOGE_(logger::FileLogger) << "Unknown option factory type.";
            PLOGE_(logger::ConsoleLogger) << "Unknown option factory type.";
    }
    return {};
}