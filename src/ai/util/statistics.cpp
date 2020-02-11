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

// Standard library and STL
#include <fstream>
#include <iomanip>              // precision
#include <numeric>              // accumulate
#include <cmath>                // std

//Include
#include "logger.h"
#include "file_dir_naming.h"


namespace statistics {

int numGameTicks = 0;
int runTimeMili = 0;
int numLevelTries = 0;

// File output
const std::string STATS_DIR = "./src/ai/stats/";
const std::string STATS_EXTENSION = ".txt";
static std::ofstream statsFile;


void resetAllStatistics(){
    numGameTicks = 0;
    runTimeMili = 0;
    numLevelTries = 0;
}


/**
 * Open the stats file.
 */
void openStatsFile() {
    std::string statsFileFullPath = STATS_DIR + getFileDirName() + STATS_EXTENSION;
    PLOGI_(logger::FileLogger) << "Creating stats file \"" << statsFileFullPath << "\"";
    PLOGI_(logger::ConsoleLogger) << "Creating stats file \"" << statsFileFullPath << "\"";
    statsFile.open(statsFileFullPath, std::ios::app);
}


/**
 * Close the stats file.
 */
void closeStatsFile() {
    if (!statsFile.is_open()) {
        return;
    }
    statsFile.close();
}


/**
 * Output the necessary statistics to file.
 */
void outputStatsToFile() {
    // Your stats output here...
    statsFile << "Number of attempts = " << numLevelTries << std::endl;
}

}
