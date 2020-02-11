/**
 * @file: config_reader.cpp
 *
 * @brief: Read configuration files
 * 
 * @author: Jake Tuero
 * Date: Jan 2020
 * Contact: tuero@ualberta.ca
 */


#ifndef CONFIG_READER_H
#define CONFIG_READER_H


// Standard Libary/STL
#include <string>
#include <unordered_map>


namespace config {
    
    /**
     * Read the given configuration file name and return the contents of the file.
     * Assumes each line in the configuration file is formatted as "VAR_NAME VALUE",
     * where VAR_NAME is the variable name to be used for the value, and VALUE is the value.
     * Assumes values are of type double. For complex types, this can by overloaded as required.
     * 
     * Config files should be placed in the "./src/ai/config/" folder.
     * 
     * @param file The string name of the config file without directory, e.g. "mcts.config"
     * @return A map with strings of the variable name as keys, and doubles as the value given.
     */
    std::unordered_map<std::string, double> getConfigValues(std::string file);
}


#endif  // CONFIG_READER_H