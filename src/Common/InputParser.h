#pragma once 
#include <string>
#include <vector>

/**
 * @brief The InputParser class
 * Based in the work from Iain
 * Borrowed form
 * https://stackoverflow.com/questions/865668/parsing-command-line-arguments-in-c
 */
class InputParser{
public:
    InputParser (int &argc, char **argv, std::string plugin_name);
    /// @author iain
    const std::string& getCmdOption(const std::string &option_name) const;
    /// @author iain
    bool cmdOptionExists(const std::string &option_name) const;
private:
    std::vector <std::string> tokens;
    std::string mPluginName;
};
