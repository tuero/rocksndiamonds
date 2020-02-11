/**
 * @file: file_dir_naming.h
 *
 * @brief: Functions dealing with file/dir naming for logs/replays/stats
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */


#include <string>


/**
 * Get the file/dir name used for both logging, replay, and stats.
 * dir name is the datetime and PID.
 * 
 * @return String name of the file/dir to use for logging, replays, or states
 */
std::string getFileDirName();