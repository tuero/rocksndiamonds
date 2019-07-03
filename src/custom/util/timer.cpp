
#include "timer.h"


Timer::Timer() {
    init();
}


void Timer::init() {
    start_time = Clock::now();
    end_time = Clock::now();
    laps = 0;
}


void Timer::start() {
    start_time = Clock::now();
    end_time = Clock::now();
    laps += 1;
}


void Timer::stop() {
    end_time = Clock::now();
}


int Timer::checkTime() {
    auto end_time = std::chrono::high_resolution_clock::now();
    int clock_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    return clock_duration;
}

int Timer::getDuration() {
    return std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
}

