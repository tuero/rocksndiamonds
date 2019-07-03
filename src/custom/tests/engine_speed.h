

#ifndef TEST_ENGINE_SPEED_H
#define TEST_ENGINE_SPEED_H


#include <iostream>
#include "../engine/engine_helper.h"
#include "../util/timer.h"
#include "../bfs/bfs.h"
#include "../mcts/mcts.h"

extern "C" {
    #include "../../main.h"
}

static const int MAX_SIMULATIONS = 1000;

void engineSpeedNoOptimizations();

void engineSpeedWithOptimizations();

void bfsSpeed();

void mctsSpeed();



#endif  //TEST_ENGINE_SPEED_H


