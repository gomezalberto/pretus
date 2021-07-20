#include "ifindMetaDataHelper.h"
#include <numeric>
#include <regex>

static std::string sNameSeparator("_");
static std::string sValueSeparator(",");

namespace ifind
{
    std::string MetaDataHelper::NameSeparator()
    {
        return sNameSeparator;
    }

    std::string MetaDataHelper::ValueSeparator()
    {
        return sValueSeparator;
    }

    std::string MetaDataHelper::MakeLabel(
        const std::string &pluginName,
        const std::string &sourceName,
        const std::string &propertyName)
    {
        return (pluginName + sNameSeparator + sourceName + sNameSeparator + propertyName);
    }

    std::vector<std::string> MetaDataHelper::SplitVectorStr(const std::string &vectorStr)
    {
        std::regex re(sValueSeparator);
        std::sregex_token_iterator
            first{ vectorStr.begin(), vectorStr.end(), re, -1 },
            last;
        return { first, last };
    }

    std::string MetaDataHelper::ConcatenateVectorStr(const std::vector <std::string> &vectorStr)
    {
        return std::accumulate(std::begin(vectorStr), std::end(vectorStr), std::string(),
            [](const std::string &ss, const std::string &s)
        {
            return ss.empty() ? s : ss + sValueSeparator + s;
        });
    }

    void MetaDataHelper::InitialiseVectorStr(
        ifind::Image::Pointer image,
        const std::string &pluginName,
        const std::string &sourceName,
        const std::string &propertyName,
        const std::string &initialValueStr,
        const unsigned int nValues)
    {
        if (nValues < 1U) {
            return;
        }

        std::string valuesStr = initialValueStr;
        auto i = 1U;
        while (i++ < nValues) {
            valuesStr += sValueSeparator + initialValueStr;
        }

        image->SetMetaData< std::string >(
            MakeLabel(pluginName, sourceName, propertyName),
            valuesStr);
    }

    bool MetaDataHelper::UpdateVectorStr(
        ifind::Image::Pointer image,
        const std::string &pluginName,
        const std::string &sourceName,
        const std::string &propertyName,
        const std::string &valueStr,
        const unsigned int iValue)
    {
        const auto metaDataLabel(MakeLabel(pluginName, sourceName, propertyName));

        auto allValuesStr = image->GetMetaData<std::string>(metaDataLabel.c_str());

        if (allValuesStr.empty()) {
            return false;
        }

        auto allValuesVec = SplitVectorStr(allValuesStr);

        if (allValuesVec.size() <= iValue) {
            return false;
        }

        allValuesVec[iValue] = valueStr;

        auto updatedAllValuesStr = ConcatenateVectorStr(allValuesVec);

        image->SetMetaData<std::string>(metaDataLabel, updatedAllValuesStr);
        return true;
    }

} // end of namespace

