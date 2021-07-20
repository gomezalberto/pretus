#include "Plugin_filemanager.h"
#include <QObject>

Q_DECLARE_METATYPE(ifind::Image::Pointer)
Plugin_filemanager::Plugin_filemanager(QObject *parent) : Plugin(parent)
{
    this->manager = FileManager::New();
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
    this->manager->params.LoopAround = false;

    /// make sure thatwhen the webcam generates an image, this plugin emits that image
    QObject::connect(this->manager.get(), &FileManager::ImageGenerated,
                     this, &Plugin_filemanager::slot_imageReceived, Qt::DirectConnection);
    this->SetDefaultArguments();
}

void Plugin_filemanager::Initialize(void){
    Plugin::Initialize();
    reinterpret_cast< ImageWidgetType *>(this->mImageWidget)->Initialize();
    if (this->manager->params.checkMhdConsistency){
        this->manager->CheckMhdConsistency();
    }
}

void Plugin_filemanager::SetDefaultArguments(){

    // arguments are defined with: name, placeholder for value, argument type,  description, default value
    mArguments.push_back({"loop", "<val>",
                            QString( ArgumentType[1] ),
                            "loop around (1) or not (0).",
                            QString::number(this->manager->params.LoopAround)});

    mArguments.push_back({"asraw", "<val>",
                            QString( ArgumentType[1] ),
                            "Load the data as raw (1), without any preprovcessing available in the header (0).",
                            QString::number(this->manager->params.AsRaw)});

    mArguments.push_back({"checkMhdConsistency", "<val>",
                            QString( ArgumentType[1] ),
                            "Check that data is consistent and ignore inconsistent files (1) or not (0).",
                            QString::number(this->manager->params.checkMhdConsistency)});
    mArguments.push_back({"input", "<path to folder>",
                            QString( ArgumentType[3] ),
                            "Take images from a folder.",
                            "Currentfolder"});
}

void Plugin_filemanager::SetCommandLineArguments(int argc, char* argv[]){
    Plugin::SetCommandLineArguments(argc, argv);
    InputParser input(argc, argv, this->GetCompactPluginName().toLower().toStdString());
    {const std::string &argument = input.getCmdOption("loop");
        if (!argument.empty()){
            this->manager->params.LoopAround = atoi(argument.c_str());
        }}
    {const std::string &argument = input.getCmdOption("asraw");
        if (!argument.empty()){
            this->manager->params.AsRaw = atoi(argument.c_str());
        }}
    {const std::string &argument = input.getCmdOption("checkMhdConsistency");
        if (!argument.empty()){
            this->manager->params.checkMhdConsistency = atoi(argument.c_str());
        }}
    {const std::string &argument = input.getCmdOption("input");
        if (!argument.empty()){
            manager->SetInputFolder(argument.c_str());
        }}
    // no need to add above since already in plugin
    {const std::string &argument = input.getCmdOption("framerate");
        if (!argument.empty()){
            manager->params.FrameRate  = atof(argument.c_str());
        }}
    {const std::string &argument = input.getCmdOption("verbose");
        if (!argument.empty()){
            manager->params.verbose= atof(argument.c_str());
        }}

}

extern "C"
{
    #ifdef WIN32
    /// Function to return an instance of a new LiveCompoundingFilter object
    __declspec(dllexport) Plugin* construct()
    {
        return new Plugin_filemanager();
    }
    #else
    Plugin* construct()
    {
        return new Plugin_filemanager();
    }
    #endif // WIN32
}

