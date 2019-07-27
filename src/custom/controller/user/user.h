

#ifndef USER_H
#define USER_H


#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "../base_controller.h"

#include "../../engine/engine_types.h"
#include "../../engine/engine_helper.h"
#include "../../engine/action.h"

//Logging
#include "../util/logging_wrapper.h"
#include <plog/Log.h>   


class User : public BaseController {
public:

    User();

    void handleEmpty(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution) override;

    void run(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution, 
        std::map<enginetype::Statistics, int> &statistics) override;    

};




#endif  //USER_H


