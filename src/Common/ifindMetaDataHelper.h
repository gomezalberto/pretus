#pragma once
#include "ifindImage.h"

#include <regex>

namespace ifind
{
    class iFIND2COMMON_EXPORT MetaDataHelper
    {
    public:
        static std::string NameSeparator();
        static std::string ValueSeparator();

        static std::string MakeLabel(
            const std::string &pluginName,
            const std::string &sourceName,
            const std::string &propertyName);

        static std::vector<std::string> SplitVectorStr(const std::string &vectorStr);

        static std::string ConcatenateVectorStr(const std::vector <std::string> &vectorStr);

        static void InitialiseVectorStr(
            ifind::Image::Pointer image,
            const std::string &pluginName,
            const std::string &sourceName,
            const std::string &propertyName,
            const std::string &initialValueStr,
            const unsigned int nValues);

        static bool UpdateVectorStr(
            ifind::Image::Pointer image,
            const std::string &pluginName,
            const std::string &sourceName,
            const std::string &propertyName,
            const std::string &valueStr,
            const unsigned int iValue);
    };
} // end of namespace

