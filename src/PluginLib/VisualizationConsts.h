#pragma once

#include <map>
#include <vector>
#include <string>

class VisualizationConsts
{
public:

    static const std::map<int, std::string> &DataTypeStrings();

    static const std::vector<int> &StandardLayerOrder();

    static const int TargetWindowWidth() { return 1920; }
    static const int TargetWindowHeight() { return 1080; }
    static const int InformationPanelWidth() { return 280; }

    static const int NormalTextPixelSize() { return 15; }
};
