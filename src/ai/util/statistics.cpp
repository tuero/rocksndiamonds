/**
 * @file: statistics.cpp
 *
 * @brief: Level statistics for the controller
 * 
 * @author: Jake Tuero
 * Date: November 2019
 * Contact: tuero@ualberta.ca
 */


#include "statistics.h"

namespace statistics {

int numGameTicks = 0;
int runTimeMili = 0;
int numLevelTries = 0;


void resetAllStatistics(){
    numGameTicks = 0;
    runTimeMili = 0;
    numLevelTries = 0;
}



}
