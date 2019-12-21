/**
 * @file: timer.h
 *
 * @brief: Timer object to be used as a clock during simulation.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */

#ifndef CUSTOMTIMER_H
#define CUSTOMTIMER_H


// Standard Libary/STL
#include <chrono>


/**
 * Simple timer class.
 *
 * Allows for start/stop, and checks if time has exceeded the given limit. The base unit of
 * time for the clock is in microseconds.
 */
class Timer {
    typedef std::chrono::high_resolution_clock Clock;

private:
    Clock::time_point startTime_;           // Wall clock start time
    Clock::time_point endTime_;             // Wall clock end time
    int laps_ = 0;                          // Number of times clock has started since last reset
    int limit_ = 0;                         // Timer limit in microseconds

public:
    Timer(int limit = 0);

    /**
     * Reset the timer.
     */
    void reset();

    /**
     * Set the timer limit.
     * 
     * @param limit The limit in microseconds.
     */
    void setLimit(int limit);

    /**
     * Start the timer.
     */
    void start();

    /**
     * Stop the timer.
     */
    void stop();

    /**
     * Check if there is still time left on the timer.
     * If no limit is given, then this will always return true
     * 
     * @return True if the time duration doesn't exceed the given limit, or if no limit was given.
     */
    bool hasTimeLeft();

    /**
     * Get the time remaining in microseconds.
     * 
     * @return The time remaining on the clock.
     */
    int getTimeRemaining();

    /**
     * Check the duration of the timer.
     * If the clock is still running, it will be the time the clock has been running for.
     * If the clock was stopped, it will be the duration from start to finish.
     * 
     * @return The duration of the timer in microseconds.
     */
    int getDuration();

};


#endif  //CUSTOMTIMER_H