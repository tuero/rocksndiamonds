

#ifndef TEST_RNG_H
#define TEST_RNG_H


#include <iostream>
#include <vector>
#include <time.h>
#include <plog/Log.h>

#include "../engine/engine_helper.h"
#include "../engine/action.h"
#include "../engine/game_state.h"
#include "../util/rng.h"
#include "../util/logger.h"
#include "../controller/controller.h"

extern "C" {
    #include "../../main.h"
}


namespace testrng{
    static const int MAX_SIMULATIONS = 100;
    static const int MAX_DEPTH = 20;

    void testStateAfterSimulations();
}



#endif  //TEST_RNG_H


