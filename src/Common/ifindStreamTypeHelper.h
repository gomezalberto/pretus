#pragma once
#include "ifindImage.h"
#include <map>
#include <QStringList>

namespace ifind
{

    struct InsensitiveCompare {
        bool operator() (const std::string& a, const std::string& b) const {
#ifdef WIN32
            return _stricmp(a.c_str(), b.c_str()) < 0;
#else
            return strcasecmp(a.c_str(), b.c_str()) < 0;
#endif
        }
    };

    //typedef std::string StreamType;
    typedef std::set<Image::StreamType, InsensitiveCompare> StreamTypeSet;
    typedef std::map<Image::StreamType, std::vector<std::string> > StreamLayerNamesTypeSet;
    typedef std::map<Image::StreamType, int> StreamLayersTypeSet;
    static StreamTypeSet InitialiseStreamTypeSetFromString(const std::string &initStr)
    {
        StreamTypeSet streamTypesSet;

        // If the string is empty or is just a '-' (dash) then treat it as empty
        if (initStr.empty() ||
            strcmp(initStr.c_str(), "-") == 0)
        {
            return streamTypesSet;
        }

        // This can be done in the stl, but is much less clear
        QString qStreamtype = initStr.c_str();
        // remove spaces
        qStreamtype = qStreamtype.simplified().remove(' ');
        // split by commas
        QStringList qtStreamTypes = qStreamtype.split(",");

        for (auto qtStreamType : qtStreamTypes)
        {
            streamTypesSet.insert(qtStreamType.toStdString());
        }

        return streamTypesSet;
    }

    static std::string StreamTypeSetToString(const StreamTypeSet &streamTypes)
    {
        std::ostringstream stream;
        std::copy(streamTypes.begin(), streamTypes.end(), 
            std::ostream_iterator<std::string>(stream, ","));
        return stream.str();
    }

    static bool IsImageOfStreamTypeSet(Image::Pointer image, const StreamTypeSet &streamTypes)
    {
        if (streamTypes.empty()) {
            return true;
        }

        return (streamTypes.find(image->GetStreamType()) != streamTypes.end());
    }

    static bool IsImageOfStreamType(Image::Pointer image, const Image::StreamType &streamType)
    {
        StreamTypeSet streamTypeSet({ streamType });

        return (IsImageOfStreamTypeSet(image, streamTypeSet));
    }

} // end of namespace

