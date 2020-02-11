/**
 * @file: file_dir_naming.cpp
 *
 * @brief: Functions dealing with file/dir naming for logs/replays/stats
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */

#include "file_dir_naming.h"


// Standard Libary/STL
#include <sys/types.h>
#include <unistd.h>


/**
 * Get the current datetime in string format to name the log/replay files/dirs
 */
std::string datetimeToString() {
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    time (&rawtime);
    timeinfo = localtime(&rawtime);

    // TODO: Prone to overflow?
    strftime(buffer,sizeof(buffer),"%Y-%m-%d_%H:%M:%S",timeinfo);

    return std::string(buffer);
}


/**
 * Get the dir/file name used for both logging and replay.
 * File/dir name is the datetime and PID.
 */
std::string getFileDirName() {
    pid_t pid = getpid();
    return datetimeToString() + "_" + std::to_string(pid);
}