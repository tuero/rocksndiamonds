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

#include "unordered_map"

namespace statistics {

int numGameTicks = 0;
int runTimeMili = 0;
int numLevelTries = 0;

std::unordered_map<uint64_t, int> pathCounts;


void resetAllStatistics(){
    numGameTicks = 0;
    runTimeMili = 0;
    numLevelTries = 0;
    pathCounts.clear();
}


void openStatsFile() {
    
}


void closeStatsFile() {

}


void outputToFile() {

}



}
