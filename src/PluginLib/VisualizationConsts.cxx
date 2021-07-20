#include "VisualizationConsts.h"

#include <map>
#include <vector>
#include <string>


static std::map<int, std::string> GenerateMapOfDataTypes()
{
    std::map<int, std::string> dataTypes;
    dataTypes.insert(std::make_pair(1, "Echo"));
    dataTypes.insert(std::make_pair(2, "Color-Flow-Power"));
    dataTypes.insert(std::make_pair(3, "Color-Flow-Velocity"));
    dataTypes.insert(std::make_pair(4, "Color-Flow-Variance"));
    dataTypes.insert(std::make_pair(5, "PW-Spectral-Doppler"));
    dataTypes.insert(std::make_pair(6, "CW-Spectral-Doppler"));

    return dataTypes;
}

static std::map<int, std::string> sDataTypesStrings(GenerateMapOfDataTypes());

static std::vector<int> sStandardLayerOrder{ 0, 1, 2, 3, 4, 5 };

const std::map<int, std::string> &VisualizationConsts::DataTypeStrings()
{
    return sDataTypesStrings;
}

const std::vector<int> &VisualizationConsts::StandardLayerOrder()
{
    return sStandardLayerOrder;
}

