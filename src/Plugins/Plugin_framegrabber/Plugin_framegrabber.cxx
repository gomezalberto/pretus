#include "Plugin_framegrabber.h"
#include <QObject>

Q_DECLARE_METATYPE(ifind::Image::Pointer)
Plugin_framegrabber::Plugin_framegrabber(QObject *parent) : Plugin(parent)
{
    this->manager =FrameGrabberManager::New();
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

    QObject::connect(this->manager.get(), &FrameGrabberManager::ImageGenerated,
                     this, &Plugin_framegrabber::slot_imageReceived, Qt::DirectConnection);

    this->SetDefaultArguments();
}


void Plugin_framegrabber::Initialize(void){
    Plugin::Initialize();
    if (this->mImageWidget != nullptr){
        reinterpret_cast< ImageWidgetType *>(this->mImageWidget)->Initialize();
    }
    this->manager->Initialize();
}

void Plugin_framegrabber::SetDefaultArguments(){
    this->RemoveArgument("stream");
    this->RemoveArgument("layer");
    this->RemoveArgument("time");
    // arguments are defined with: name, placeholder for value, argument type,  description, default value
    mArguments.push_back({"studioswing", "<val>",
                          QString( Plugin::ArgumentType[0] ),
                          "Correct for studio swing (1) or not (0).",
                          QString::number(manager->params.correct_studio_swing)});

    mArguments.push_back({"resolution", "<val>",
                          QString( Plugin::ArgumentType[2] ),
                          "Value, in mm, of the pixel size (isotropic).",
                          QString::number(manager->params.pixel_size[0])});

}

void Plugin_framegrabber::SetCommandLineArguments(int argc, char* argv[]){
    Plugin::SetCommandLineArguments(argc, argv);
    InputParser input(argc, argv, this->GetCompactPluginName().toLower().toStdString());

    {const std::string &argument = input.getCmdOption("studioswing");
        if (!argument.empty()){
            manager->params.correct_studio_swing= atoi(argument.c_str());
        }}
    {const std::string &argument = input.getCmdOption("resolution");
        if (!argument.empty()){
            manager->params.pixel_size[0]= atof(argument.c_str());
            manager->params.pixel_size[1]= atof(argument.c_str());
            manager->params.pixel_size[0]= 1.0;
        }}
    // no need to add above since already in plugin
    {const std::string &argument = input.getCmdOption("framerate");
        if (!argument.empty()){
            manager->params.CaptureFrameRate = atof(argument.c_str());
        }}
    {const std::string &argument = input.getCmdOption("verbose");
        if (!argument.empty()){
            manager->params.verbose= atoi(argument.c_str());
        }}
}

extern "C"
{
#ifdef WIN32
/// Function to return an instance of a new LiveCompoundingFilter object
__declspec(dllexport) Plugin* construct()
{
    return new Plugin_framegrabber();
}
#else
Plugin* construct()
{
    return new Plugin_framegrabber();
}
#endif // WIN32
}


