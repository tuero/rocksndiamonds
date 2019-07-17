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
     * Check to see if there is time remaining for the given limit
     */
    bool hasTimeLeft(int limit);

    /*
     * Get time remaining (difference between limit and start time)
     */
    int getTimeLeft(int limit);

    /*
     * Get time in micro-seconds 
     */
    int getDuration();

};


#endif  //CUSTOMTIMER_H