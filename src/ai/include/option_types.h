/**
 * @file: base_option.h
 *
 * @brief: Base option which all implemented options should be derived from.
 * 
 * @author: Jake Tuero
 * Date: September 2019
 * Contact: tuero@ualberta.ca
 */

#ifndef OPTION_TYPES_H
#define OPTION_TYPES_H


/**
 * Option type definitions.
 * 
 * @note Add new option types here.
 */
enum OptionType {SingleAction, SingleActionNoNoop, ToCollectibleSprite, ToExit, ToDoor, Misc};


/**
 * Option grouping definitions for factory.
 * 
 * @note Add new option groupings here.
 */
enum class OptionFactoryType {
    SINGLE_ACTION = 0,
    SINGLE_ACTION_NO_NOOP,
    PATH_TO_SPRITE,
    TWO_LEVEL_SEARCH,
    CUSTOM
};


#endif  //OPTION_TYPES_H


