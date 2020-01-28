/**
 * @file: file_naming.cpp
 *
 * @brief: Functions dealing with file naming for logs/replays/stats
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */

#include "file_naming.h"


// Standard Libary/STL
#include <sys/types.h>
#include <unistd.h>


/**
 * Get the current datetime in string format to name the log/replay files
 */
std::string datetimeToString() {
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    time (&rawtime);
    timeinfo = localtime(&rawtime);

    // TODO: Prone to overflow?
    strftime(buffer,sizeof(buffer),"%Y-%m-%d %H:%M:%S",timeinfo);

    return std::string(buffer);
}


/**
 * Get the file name used for both logging and replay.
 * File name is the datetime and PID.
 */
std::string getFileName() {
    pid_t pid = getpid();
    return datetimeToString() + " " + std::to_string(pid);
}