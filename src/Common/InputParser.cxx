#include "InputParser.h"
#include <algorithm>

InputParser::InputParser (int &argc, char **argv, std::string plugin_name){
    for (int i=1; i < argc; ++i)
        this->tokens.push_back(std::string(argv[i]));
    mPluginName = plugin_name;
}
/// @author iain
const std::string& InputParser::getCmdOption(const std::string &option_name) const{

    std::string option = "--" + mPluginName + "_" + option_name;

    std::vector<std::string>::const_iterator itr;
    itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
    if (itr != this->tokens.end() && ++itr != this->tokens.end()){
        return *itr;
    }
    static const std::string empty_string("");
    return empty_string;
}
/// @author iain
bool InputParser::cmdOptionExists(const std::string &option_name) const{

    std::string option = "--" + mPluginName + "_" + option_name;

    return std::find(this->tokens.begin(), this->tokens.end(), option)
            != this->tokens.end();
}

