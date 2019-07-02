#ifndef CUSTOMTIMER_H
#define CUSTOMTIMER_H

#include <chrono>


class Timer {
    typedef std::chrono::high_resolution_clock Clock;
private:
    Clock::time_point start_time, end_time;
    int laps;
public:

    Timer();

    /*
     * Reset clock to current time along with additional tracking statistics
     */
    void init();

    /*
     * Start the clock and increment statistics
     */
    void start();

    /*
     * Stop the clock
     */
    void stop();

    /*
     * Check how long as passed since the clock has started (in micro-seconds)
     */
    bool checkTime();

    /*
     * Get time in micro-seconds 
     */
    int getDuration();

};


#endif  //CUSTOMTIMER_H