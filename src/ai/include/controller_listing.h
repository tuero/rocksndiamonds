/**
 * @file: controller_listing.h
 *
 * @brief: Controller type definition and related information
 * 
 * @author: Jake Tuero
 * Date: November 2019
 * Contact: tuero@ualberta.ca
 */

#ifndef CONTROLLER_LISTING_H
#define CONTROLLER_LISTING_H


#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))

/**
 * Controller types
 * For additional controllers, add a new option to the end.
 */
typedef enum ControllerType {CONTROLLER_DEFAULT, CONTROLLER_REPLAY, CONTROLLER_MCTS, 
                             CONTROLLER_MCTS_OPTIONS, CONTROLLER_TWOLEVEL} ControllerType;

/**
 * Command line strings to set the controller.
 * The ith string corresponds to the ith ControllerType. Add new options to the end.
 */
#define CONTROLLER_STRINGS {"default", "replay", "mcts", "mcts_options", "two_level"};


#endif  //CONTROLLER_LISTING_H