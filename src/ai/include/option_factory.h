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

// Standard Libary/STL
#include <memory>
#include <vector>

// Includes
#include "base_option.h"
#include "option_types.h"


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
    std::vector<std::unique_ptr<BaseOption>> options_;                              // Option is responsible for memory managing the option objects
    
    /**
     * Create options for each of the single step actions.
     *
     * @return Vector containing pointers to each single step option.
     */
    std::vector<BaseOption*> createSingleActionOptions();

    /**
     * Create options for each of the single step actions (excluding NOOP).
     */
    std::vector<BaseOption*> createSingleActionNoNoopOptions();

    /**
     * Create options for path finding for each sprite on screen
     *
     * @return Vector containing pointers to each sprite option.
     */
    std::vector<BaseOption*> createPathToSpriteOptions();

    /**
     * Create options for the two level search.
     * This uses pathing to collectible sprites (diamonds, keys, etc.) and to exit.
     * Used for Masters thesis.
     *
     * @return Vector containing pointers to each sprite option.
     */
    std::vector<BaseOption*> createTwoLevelSearchOptions();

    /**
     * Used for a custom list of options.
     * 
     * @note Here is where you can define similar functions to the above for new controllers.
     *
     * @return Vector containing pointers to each sprite option.
     */
    std::vector<BaseOption*> createCustomOptions();

public:

    OptionFactory() {}

    /**
     * Create options for use in search. 
     * Calls specific option factory method depending on input.
     * 
     * @param optionFactoryType The factory type specifying the group of options to create.
     * @return Vector containing pointers to each single step option.
     */
    std::vector<BaseOption*> createOptions(OptionFactoryType optionFactoryType);

};



#endif  //OPTION_FACTORY_H


