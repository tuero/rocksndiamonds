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

#include <chrono>


/**
 * Simple timer class.
 *
 * Allows for start/stop, and checks if time has exceeded the given limit. The limit can be 
 * given as an optional consturctor argument, or  
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

    /*
     * Reset the timer.
     */
    void reset();

    /*
     * Set the timer limit.
     */
    void setLimit(int limit);

    /*
     * Start the timer.
     */
    void start();

    /*
     * Stop the timer.
     */
    void stop();

    /*
     * Check if there is still time left on the timer.
     */
    bool hasTimeLeft();

    /*
     * Check if there is still time left on the timer.
     */
    int getTimeLeft();

    /*
     * Check the duration of the timer.
     */
    int getDuration();

};


#endif  //CUSTOMTIMER_H