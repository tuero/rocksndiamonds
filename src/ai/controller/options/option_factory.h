/**
 * @file: option_factory.h
 *
 * @brief: Factory class to get sets of options depending on the needs of the controller.
 * 
 * @author: Jake Tuero
 * Date: September 2019
 * Contact: tuero@ualberta.ca
 */

#ifndef OPTION_FACTORY_H
#define OPTION_FACTORY_H

#include <memory>
#include <vector>

#include "base_option.h"


enum class OptionFactoryType {
    SINGLE_ACTION = 0,
    PATH_TO_SPRITE,
    TWO_LEVEL_SEARCH,
    CUSTOM
};


/**
 * Factory class which creates options for controllers.
 *
 * The factory is responsible for creating the necessary options and will be responsible
 * for ownership of the objects. The factory will return pointers to the options for the
 * controllers use.
 */
class OptionFactory {
private:
    OptionFactoryType optionFactoryType_ = OptionFactoryType::SINGLE_ACTION;
    std::vector<std::unique_ptr<BaseOption>> options_;
    
    /**
     * Create options for each of the single step actions.
     *
     * @return Vector containing pointers to each single step option.
     */
    std::vector<BaseOption*> createSingleActionOptions();

    /**
     * Create options for path finding for each sprite on screen
     *
     * @return Vector containing pointers to each sprite option.
     */
    std::vector<BaseOption*> createPathToSpriteOptions();


    std::vector<BaseOption*> createTwoLevelSearchOptions();

    std::vector<BaseOption*> createCustomOptions();

public:

    OptionFactory() {}

    /**
     * Create options for use in search. 
     * 
     * Calls specific option factory method depending on input.
     * 
     * @param optionFactoryType The factory type specifying the group of options to create.
     * @return Vector containing pointers to each single step option.
     */
    std::vector<BaseOption*> createOptions(OptionFactoryType optionFactoryType);

};



#endif  //OPTION_FACTORY_H


