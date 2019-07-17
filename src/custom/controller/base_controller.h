

#ifndef BASE_CONTROLLER_H
#define BASE_CONTROLLER_H


#include <vector>
#include <map>
#include "../engine/action.h"


class BaseController {

// protected:

public:

    virtual void handleEmpty(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution) = 0;

    virtual void run(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution, 
        std::map<enginetype::Statistics, int> &statistics) = 0;

};



#endif  //BASE_CONTROLLER_H


