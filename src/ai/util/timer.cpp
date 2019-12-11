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


/**
 * Reset the timer.
 */
void Timer::reset() {
    startTime_ = Clock::now();
    endTime_ = startTime_;
    laps_ = 0;
}


/**
 * Set the timer limit.
 *
 * @param limit The limit for the timer in microseconds.
 */
void Timer::setLimit(int limit) {
    limit_ = limit;
}


/**
 * Start the timer.
 */
void Timer::start() {
    startTime_ = Clock::now();
    endTime_ = startTime_;
    laps_ += 1;
}


/**
 * Stop the timer.
 */
void Timer::stop() {
    endTime_ = Clock::now();
}


/**
 * Check if there is still time left on the timer.
 */
bool Timer::hasTimeLeft() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    int clock_duration = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - startTime_).count();
    return (limit_ == 0) ? true : (clock_duration < limit_);
}


/**
 * Check if there is still time left on the timer.
 *
 * @return True if the timer still has time left.
 */
int Timer::getTimeRemaining() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    int clock_duration = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - startTime_).count();
    auto delta = limit_ - clock_duration;
    return (delta > 0) ? delta : 0;
}


/**
 * Check the duration of the timer.
 *
 * @return number of microseconds since timer start.
 */
int Timer::getDuration() {
    // if clock still running (starttime==endtime), return time since clock started
    auto referencePoint = (startTime_ == endTime_) ? std::chrono::high_resolution_clock::now() : endTime_;
    return std::chrono::duration_cast<std::chrono::microseconds>(referencePoint - startTime_).count();
}

