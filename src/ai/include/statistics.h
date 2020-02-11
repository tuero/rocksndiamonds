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


// Standard library and STL
#include <string>


namespace statistics {

extern int numGameTicks;
extern int runTimeMili;
extern int numLevelTries;

/**
 * Reset the statistic values
 */
void resetAllStatistics();


/**
 * Open the stats file.
 */
void openStatsFile();

/**
 * Close the stats file.
 */
void closeStatsFile();

/**
 * Output the necessary statistics to file.
 */
void outputStatsToFile();

}

#endif  //STATISTICS_H


