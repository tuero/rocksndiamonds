#ifndef RANDOMGENERATOR_H
#define RANDOMGENERATOR_H

#include <random>


class RNG {
private:
    std::random_device rd;
    std::mt19937 gen();
    std::uniform_int_distribution<uint64_t> dis(0, MAX_HASH);
public:

    RNG();

    void init();

    void start();

    void stop();

    bool checkTime();

};


void startClock(); 
int stopClock();

void debug_print();

#endif  //RANDOMGENERATOR_H