/**
 * @file: config_reader.cpp
 *
 * @brief: Read configuration files
 * 
 * @author: Jake Tuero
 * Date: Jan 2020
 * Contact: tuero@ualberta.ca
 */


// Standard Libary/STL
#include <fstream>
#include <string>
#include <unordered_map>

// Include
#include "logger.h"


namespace config {

    static const std::string CONFIG_DIR = "./src/ai/config/";
    
    /**
     * Read the given configuration file name and return the contents of the file.
     * Assumes each line in the configuration file is formatted as "VAR_NAME VALUE",
     * where VAR_NAME is the variable name to be used for the value, and VALUE is the value.
     * Assumes values are of type double. For complex types, this can by overloaded as required.
     * 
     * Config files should be placed in the "./src/ai/config/" folder.
     */
    std::unordered_map<std::string, double> getConfigValues(std::string file) {
        std::unordered_map<std::string, double> values;
        std::ifstream configFileStream(CONFIG_DIR + file, std::ifstream::in);

        // Bad file stream
        if (!configFileStream.good()) {
            PLOGE_(logger::FileLogger) << "Unable to open config file: " << CONFIG_DIR << file;
            PLOGE_(logger::ConsoleLogger) << "Unable to open config file: " << CONFIG_DIR << file;
            throw std::exception();
        }

        std::string varName;
        double value;

        // Read input from file
        while (configFileStream >> varName >> value) {
            values[varName] = value;
        }

        return values;
    }
}
