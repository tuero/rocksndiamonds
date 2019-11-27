/**
 * @file: statistics.h
 *
 * @brief: Level statistics for the controller
 * 
 * @author: Jake Tuero
 * Date: November 2019
 * Contact: tuero@ualberta.ca
 */


#ifndef STATISTICS_H
#define STATISTICS_H

#include <string>


namespace statistics {

extern int numGameTicks;
extern int runTimeMili;
extern int numLevelTries;

void resetAllStatistics();

std::string getAllStatistics();

}

#endif  //STATISTICS_H


