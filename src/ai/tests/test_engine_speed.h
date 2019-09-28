

#ifndef TEST_ENGINE_SPEED_H
#define TEST_ENGINE_SPEED_H


#include <iostream>
#include <plog/Log.h>

#include "../engine/engine_helper.h"
#include "../engine/game_state.h"
#include "../util/timer.h"
#include "../util/logging_wrapper.h"
#include "../controller/controller.h"

extern "C" {
    #include "../../main.h"
}


namespace testenginespeed {
    static const int MAX_SIMULATIONS = 1000;

    void testEngineSpeedNoOptimizations();

    void testEngineSpeedWithOptimizations();

    void testBfsSpeed();

    void testMctsSpeed();
}



#endif  //TEST_ENGINE_SPEED_H


