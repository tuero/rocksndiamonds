/**
 * @file: timer.cpp
 *
 * @brief: Timer object to be used as a clock during simulation.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */

#include "timer.h"


Timer::Timer(int limit) : limit_(limit) {
    reset();
}


/*
 * Reset the timer.
 */
void Timer::reset() {
    startTime_ = Clock::now();
    endTime_ = Clock::now();
    laps_ = 0;
}


/*
 * Set the timer limit.
 *
 * @param limit The limit for the timer in microseconds.
 */
void Timer::setLimit(int limit) {
    limit_ = limit;
}


/*
 * Start the timer.
 */
void Timer::start() {
    startTime_ = Clock::now();
    endTime_ = Clock::now();
    laps_ += 1;
}


/*
 * Stop the timer.
 */
void Timer::stop() {
    endTime_ = Clock::now();
}


/*
 * Check if there is still time left on the timer.
 *
 * @return True if the timer still has time left.
 */
bool Timer::hasTimeLeft() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    int clock_duration = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - startTime_).count();
    return clock_duration < limit_;
}


/*
 * Check if there is still time left on the timer.
 *
 * @return True if the timer still has time left.
 */
int Timer::getTimeLeft() {
    auto endTime_ = std::chrono::high_resolution_clock::now();
    int clock_duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime_ - startTime_).count();
    return limit_ - clock_duration;
}


/*
 * Check the duration of the timer.
 *
 * @return number of microseconds since timer start.
 */
int Timer::getDuration() {
    return std::chrono::duration_cast<std::chrono::microseconds>(endTime_ - startTime_).count();
}

