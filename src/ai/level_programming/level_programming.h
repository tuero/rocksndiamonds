/**
 * @file: level_programming.h
 *
 * @brief: Handle custom level programming such as spawning new elements.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */

#ifndef LEVEL_PROGRAMMING_H
#define LEVEL_PROGRAMMING_H


namespace levelprogramming {

    /**
     * Handle custom level programming for each game tick.
     */
    void customLevelProgrammingUpdate();

    /**
     * Handle custom level programming at level start.
     */
    void customLevelProgrammingStart();

}


#endif  //LEVEL_PROGRAMMING_H


