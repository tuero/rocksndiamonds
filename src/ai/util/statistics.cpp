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
#include <vector>
#include <array>
#include <map>
#include <unordered_map>
#include <algorithm>            // max_element
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

std::map<int, uint64_t> nodesExpanded;
std::map<int, std::unordered_map<uint64_t, int>> pathCounts;
std::map<int, std::array<uint64_t, 2>> solutionPathCounts;

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
    outputPathCounts();
}

void outputPathCounts() {
    int solution_path_visits_count = 0;
    int total_path_visit_counts = 0;
    std::vector<int> solutionPaths;
    std::vector<int> totalPaths;

    for (auto const & level_run : pathCounts) {
        statsFile << "Level: " << level_run.first << std::endl;

        std::vector<int> counts;
        int maxCount = 0;
        uint64_t maxHash = 0;
        for (auto const & run : level_run.second) {
            counts.push_back(run.second);
            if (run.second > maxCount) {
                maxCount = run.second;
                maxHash = run.first;
            }
        }

        // Total unique paths
        statsFile << "Total number of unique paths: " << level_run.second.size() << std::endl;
        // Total tries
        int total_count = std::accumulate(counts.begin(), counts.end(), 0);
        statsFile << "Total number of attempts: " << total_count << std::endl;
        statsFile << "Maximum number of attempts: (" << maxHash << "), " << maxCount << std::endl;
        // Average path visit
        float avg = (float)(std::accumulate(counts.begin(), counts.end(), 0)) / (counts.empty() ? 1 : counts.size());
        statsFile << "Average number of attempts: " << avg << std::endl;

        // Solution high level path visit count
        solution_path_visits_count += (solutionPathCounts[level_run.first])[1];
        solutionPaths.push_back((solutionPathCounts[level_run.first])[1]);
        totalPaths.push_back(total_count);
        total_path_visit_counts += total_count;
        statsFile << "Solution path (" << (solutionPathCounts[level_run.first])[0] << ") attempts: " << (solutionPathCounts[level_run.first])[1] << std::endl;
        statsFile << std::endl;
    }

    // total attempts
    int sizeTotal = (int)totalPaths.size();
    std::sort(totalPaths.begin(), totalPaths.end());
    double sumTotal = (double)std::accumulate(totalPaths.begin(), totalPaths.end(), 0);
    double meanTotal = sumTotal / sizeTotal;
    std::vector<double> diffTotal(sizeTotal);
    std::transform(totalPaths.begin(), totalPaths.end(), diffTotal.begin(), [meanTotal](double x) { return x - meanTotal; });
    double sq_sumTotal = std::inner_product(diffTotal.begin(), diffTotal.end(), diffTotal.begin(), 0.0);
    double stdevTotal = std::sqrt(sq_sumTotal / sizeTotal);
    double medianTotal = (sizeTotal % 2) ? ((totalPaths[sizeTotal / 2 - 1] + totalPaths[sizeTotal / 2]) / 2) : (totalPaths[sizeTotal / 2]);

    statsFile << "----------------------------------------------------------------" << std::endl;
    statsFile << "Levelset mean total attempts: " << meanTotal << std::endl;
    statsFile << "Levelset median total attempts: " << medianTotal << std::endl;
    statsFile << "Levelset std total attempts: " << stdevTotal << std::endl << std::endl;

    // solution attempts
    int sizeSolution = (int)solutionPaths.size();
    std::sort(solutionPaths.begin(), solutionPaths.end());
    double sumSolution = (double)std::accumulate(solutionPaths.begin(), solutionPaths.end(), 0);
    double meanSolution = sumSolution / sizeSolution;
    std::vector<double> diffSolution(sizeSolution);
    std::transform(solutionPaths.begin(), solutionPaths.end(), diffSolution.begin(), [meanSolution](double x) { return x - meanSolution; });
    double sq_sumSolution = std::inner_product(diffSolution.begin(), diffSolution.end(), diffSolution.begin(), 0.0);
    double stdevSolution = std::sqrt(sq_sumSolution / sizeSolution);
    double medianSolution = (sizeSolution % 2) ? ((solutionPaths[sizeSolution / 2 - 1] + solutionPaths[sizeSolution / 2]) / 2) : (solutionPaths[sizeSolution / 2]);
    auto maxSolutionLength = std::max_element(solutionPaths.begin(), solutionPaths.end());


    statsFile << "Levelset mean solution path attempts: " << meanSolution << std::endl;
    statsFile << "Levelset median solution path attempts: " << medianSolution << std::endl;
    statsFile << "Levelset std solution path attempts: " << stdevSolution << std::endl;
    statsFile << "Levelset solution max attempts: " << *maxSolutionLength << std::endl;
}


}
