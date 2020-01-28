/**
 * @file: file_naming.h
 *
 * @brief: Functions dealing with file naming for logs/replays/stats
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */


#include <string>


/**
 * Get the file name used for both logging, replay, and stats.
 * File name is the datetime and PID.
 * 
 * @return String name of the file to use for logging, replays, or states
 */
std::string getFileName();