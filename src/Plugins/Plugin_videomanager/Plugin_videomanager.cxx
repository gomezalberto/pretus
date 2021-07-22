#include "Plugin_videomanager.h"
#include <QObject>

Q_DECLARE_METATYPE(ifind::Image::Pointer)
Plugin_VideoManager::Plugin_VideoManager(QObject *parent) : Plugin(parent)
{
    this->manager = VideoManager::New();
    {
        // create widget
        WidgetType * mWidget_ = new WidgetType;
        this->mWidget = mWidget_;
    }

    {
        // create image widget
        ImageWidgetType * mWidget_ = new ImageWidgetType;
        this->mImageWidget = mWidget_;
        this->mImageWidget->SetWidgetLocation(ImageWidgetType::WidgetLocation::visible);
    }

    /// make sure that when the video generates an image, this plugin emits that image
    QObject::connect(manager.get(), &VideoManager::ImageGenerated,
                     this, &Plugin_VideoManager::slot_imageReceived,Qt::DirectConnection);
    this->SetDefaultArguments();
}


void Plugin_VideoManager::Initialize(void){
    Plugin::Initialize();
    reinterpret_cast< ImageWidgetType *>(this->mImageWidget)->Initialize();
    this->manager->Initialize();
}

void Plugin_VideoManager::SetDefaultArguments(){

    // arguments are defined with: name, placeholder for value, argument type,  description, default value
    mArguments.push_back({"loop", "<val>",
                            QString( ArgumentType[1] ),
                            "loop around (1) or not (0).",
                            QString::number(this->manager->getLoopAround())});

    mArguments.push_back({"ff", "<fast forward factor>",
                            QString( ArgumentType[2] ),
                            "Fast forward factor, in (0, inf). 1 means native speed, >1 is faster, <1 is slower.",
                            QString::number(this->manager->FF_factor)});

    mArguments.push_back({"starttime", "<mm:ss>",
                            QString( ArgumentType[3] ),
                            "Initial time to start getting frames from the video.",
                            "00:00"});

    mArguments.push_back({"input", "<path to video>",
                            QString( ArgumentType[3] ),
                            "Take images from a video file.",
                            "N/A (compulsory)"});
}


void Plugin_VideoManager::SetCommandLineArguments(int argc, char* argv[]){
    Plugin::SetCommandLineArguments(argc, argv);
    InputParser input(argc, argv, this->GetCompactPluginName().toLower().toStdString());
    {const std::string &argument = input.getCmdOption("loop");
        if (!argument.empty()){
            this->manager->setLoopAround(atoi(argument.c_str()));
        }}
    {const std::string &argument = input.getCmdOption("ff");
        if (!argument.empty()){
            this->manager->FF_factor= atof(argument.c_str());
        }}
    {const std::string &argument = input.getCmdOption("starttime");
        if (!argument.empty()){
            this->manager->SetStringTime(argument.c_str());
        }}
    {const std::string &argument = input.getCmdOption("input");
        if (!argument.empty()){
            manager->setVideoFile(argument.c_str());
        }}
    // no need to add above since already in plugin
    {const std::string &argument = input.getCmdOption("framerate");
        if (!argument.empty()){
            manager->setFrameRate(atof(argument.c_str()));
        }}
    {const std::string &argument = input.getCmdOption("verbose");
        if (!argument.empty()){
            manager->verbose= atof(argument.c_str());
        }}
}

extern "C"
{
#ifdef WIN32
/// Function to return an instance of a new LiveCompoundingFilter object
__declspec(dllexport) Plugin* construct()
{
    return new Plugin_VideoManager();
}
#else
Plugin* construct()
{
    return new Plugin_VideoManager();
}
#endif // WIN32
}

