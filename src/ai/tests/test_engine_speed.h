

#ifndef TEST_ENGINE_SPEED_H
#define TEST_ENGINE_SPEED_H


#include <iostream>
#include <vector>
#include <time.h>
#include <plog/Log.h>

#include "../engine/action.h"
#include "../engine/engine_helper.h"
#include "../engine/game_state.h"
#include "../util/timer.h"
#include "../util/rng.h"
#include "../util/logger.h"
#include "../controller/controller.h"

extern "C" {
    #include "../../main.h"
}


namespace testenginespeed {
    static const int MAX_SIMULATIONS = 1000;
    static const int MAX_DEPTH = 20;

    void testEngineSpeedNoOptimizations();

    void testEngineSpeedWithOptimizations();

    void testMctsSpeed();
}



#endif  //TEST_ENGINE_SPEED_H


