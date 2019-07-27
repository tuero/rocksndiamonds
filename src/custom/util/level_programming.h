
#ifndef LEVEL_PROGRAMMING_H
#define LEVEL_PROGRAMMING_H

#include <vector>
#include <array>

// Engine
#include "../engine/engine_helper.h"
#include "../engine/engine_types.h"
#include "../engine/action.h"

// Logging
#include "../util/logging_wrapper.h"
#include <plog/Log.h> 


namespace levelprogramming {

    /*
     * Spawn new elements based on level rules
     */
    void spawnElements();

}


#endif  //LEVEL_PROGRAMMING_H


