

#include "user.h"

User::User(){}



void User::handleEmpty(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution) {
    // Silent compiler warning
    (void)currentSolution;
    (void)forwardSolution;
}


void User::run(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution, 
        std::map<enginetype::Statistics, int> &statistics) 
{
    // Silent compiler warning
    (void)forwardSolution;
    (void)statistics;

    currentSolution.clear();
    Action action =  static_cast<Action>(enginehelper::getEnginePlayerAction());
    currentSolution.push_back(action);

}
