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
#include <map>
#include <string>
#include <unordered_map>
#include <array>

namespace statistics {

extern int numGameTicks;
extern int runTimeMili;
extern int numLevelTries;

extern std::map<int, uint64_t> nodesExpanded;
extern std::map<int, std::unordered_map<uint64_t, int>> pathCounts;
extern std::map<int, std::array<uint64_t, 2>> solutionPathCounts;
extern std::map<int, int> solutionConstraintCount;
extern std::string output_msg;

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

void outputPathCounts();

void outputRunLengthToFile(int run, const std::string & levelset, int level);

}

#endif  //STATISTICS_H


