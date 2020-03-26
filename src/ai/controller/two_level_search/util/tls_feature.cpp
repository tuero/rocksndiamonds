/**
 * @file: tls_feature.cpp
 *
 * @brief: Construct the features for learning
 * 
 * @author: Jake Tuero
 * Date: February 2020
 * Contact: tuero@ualberta.ca
 */

#include "tls_feature.h"

// Standard Libary/STL
#include <unordered_map>
#include <algorithm>            // iota

// Pytorch
#include <torch/torch.h>

// Includes
#include "engine_types.h"
#include "engine_helper.h"
#include "logger.h"

// namespace
using namespace enginehelper;


namespace tlsfeature {


/**
 * Given a node, find a path of gridcells that that visits each 
 * high level action without constraints.
 */
std::deque<enginetype::GridCell> getNodePath(const std::vector<BaseOption*> &path) {
    enginetype::GridCell prev_cell;
    enginetype::GridCell next_cell = gridinfo::getPlayerPosition();
    std::deque<enginetype::GridCell> totalPath{next_cell};

    // Get path from prev highlevel path to curr highlevel path, over all HLA in the path
    for (std::size_t i = 0; i < path.size(); ++i) {
        prev_cell = next_cell;
        next_cell = path[i]->getGoalCell();
        path[i]->runAStar(prev_cell, next_cell);
        std::deque<enginetype::GridCell> solution_path = path[i]->getSolutionPath();
        totalPath.insert(totalPath.end(), solution_path.begin(), solution_path.end());
    }
    return totalPath;
}


// Create object map of random floats [0,1]
// 7 193 183 88 23 126 176 115 240 143 214 191 71 101 79 155
std::unordered_map<int, float> objectMap{
    {0x0000, 0},        // empty
    {0x0001, 193},      // dirt
    {0x006A, 183},      // rock
    {0x0268, 88},       // key_red
    {0x0269, 23},       // key_yellow
    {0x026A, 126},      // key_green
    {0x026B, 176},      // key_blue
    {0x00CB, 115},      // gate_red
    {0x00CC, 240},      // gate_yellow
    {0x00CD, 143},      // gate_green
    {0x00CE, 214},      // gate_blue
    {0x0007, 191},      // door_exit
    {0x000D, 71},       // wall_steel
    {0x0003, 101},      // wall_round
    {0x0038, 7},        // gem_diamond
    {0x0050, 79},       // agent
    {0x0002, 155}       // bitwall
};


/**
 * Get the input feature representation for the levin node
 * 2-channel tensor (2 x width x height) representing the gameboard and highlevel path taken
 */
at::Tensor getNodeFeature(const NodeLevin &node, short Feld[128][128]) {
    int levelWidth = levelinfo::getLevelWidth();
    int levelHeight = levelinfo::getLevelHeight();

    // Data vector for map represented by rands (0,1)
    std::vector<float> levelData;
    for (int y = 0; y < levelHeight; y++) {
        for (int x = 0; x < levelWidth; x++) {
            levelData.push_back((float)objectMap.at(Feld[x][y]) / (float)255);
        }
    }

    // path vector representing high level path on actual map board
    std::vector<float> pathData(levelData.size(), 0);

    // Gradient representing path ordering
    std::vector<float> pathGradient(node.fullGridPath.size());
    std::iota(pathGradient.begin(), pathGradient.end(), 1);
    for (auto & p : pathGradient) {p = p / pathGradient.size();}

    // Set pathData
    std::unordered_map<int, int> cellCountMap;
    for (std::size_t i = 0; i < pathGradient.size(); i++) {
        enginetype::GridCell cell = node.fullGridPath[i];
        pathData[cell.y*levelWidth + cell.x] += pathGradient[i];
        cellCountMap[gridinfo::cellToIndex(cell)] += 1;
    }

    // Average out the multiple visits
    for (auto const & itr : cellCountMap) {
        enginetype::GridCell cell = gridinfo::indexToCell(itr.first);
        pathData[cell.y*levelWidth + cell.x] /= itr.second;
    }

    // Combine the 2channel maps into one 
    levelData.insert(levelData.end(), pathData.begin(), pathData.end());

    // Create tensor (2 x height x width)
    return torch::from_blob(&levelData[0], {2, levelHeight, levelWidth}).clone();
}


/**
 * Get the observed runtime from the levin node.
 */
at::Tensor getNodeObservation(const NodeLevin &node, bool isSolution, bool wasExhausted) {
    // PLOGE_(logger::ConsoleLogger) << node.timesVisited << " " << isSolution << " " << wasExhausted << " " << node.wasSkipped << 
    //     " "  << node.numConstraints;
    // PLOGE_(logger::ConsoleLogger) << node.timesVisited;
    // PLOGE_(logger::ConsoleLogger) << node.numConstraints;
    std::array<float, 4> observation{(float)node.timesVisited, (float)isSolution, (float)wasExhausted, (float)node.wasSkipped};
    return torch::from_blob(&observation, {4}).clone();
}

} //namespace tlsfeature
