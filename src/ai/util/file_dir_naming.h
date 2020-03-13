/**
 * @file: file_dir_naming.h
 *
 * @brief: Functions dealing with file/dir naming for logs/replays/stats
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */


// Standard Libary/STL
#include <string>
#include <fstream>


/**
 * Get the file/dir name used for both logging, replay, and stats.
 * dir name is the datetime and PID.
 * 
 * @return String name of the file/dir to use for logging, replays, or states
 */
std::string getFileDirName();


/**
 * Get an input filestream for the given filename.
 * 
 * @note Throws an exception if can't open the file
 * 
 * @param fileName The file name with directory path
 * @return An input filestream for the file
 */
std::ifstream getFileStreamIn(const std::string &fileName);