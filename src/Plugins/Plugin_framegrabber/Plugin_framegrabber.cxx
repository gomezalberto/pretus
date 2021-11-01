#include "Plugin_framegrabber.h"
#include <QObject>

Q_DECLARE_METATYPE(ifind::Image::Pointer)
Plugin_framegrabber::Plugin_framegrabber(QObject *parent) : Plugin(parent)
{
    this->mIsInput = true;
    {
        ManagerType::Pointer manager_ = ManagerType::New();
        this->manager = manager_;
    }
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

    QObject::connect(this->manager.get(), &ManagerType::ImageGenerated,
                     this, &Plugin_framegrabber::slot_imageReceived, Qt::DirectConnection);

    this->SetDefaultArguments();
}


void Plugin_framegrabber::Initialize(void){
    Plugin::Initialize();
    if (this->mImageWidget != nullptr){
        reinterpret_cast< ImageWidgetType *>(this->mImageWidget)->Initialize();
    }
    std::dynamic_pointer_cast< ManagerType >(this->manager)->Initialize();
}

void Plugin_framegrabber::SetDefaultArguments(){
    this->RemoveArgument("stream");
    this->RemoveArgument("layer");
    this->RemoveArgument("time");
    // arguments are defined with: name, placeholder for value, argument type,  description, default value
    mArguments.push_back({"studioswing", "<val>",
                          QString( Plugin::ArgumentType[0] ),
                          "Correct for studio swing (1) or not (0).",
                          QString::number(std::dynamic_pointer_cast< ManagerType >(this->manager)->params.correct_studio_swing)});

    mArguments.push_back({"resolution", "<val>",
                          QString( Plugin::ArgumentType[2] ),
                          "Value, in mm, of the pixel size (isotropic).",
                          QString::number(std::dynamic_pointer_cast< ManagerType >(this->manager)->params.pixel_size[0])});

    mArguments.push_back({"color", "<0/1>",
                          QString( Plugin::ArgumentType[0] ),
                          "USe color images (1) or not (0).",
                          QString::number(std::dynamic_pointer_cast< ManagerType >(this->manager)->params.n_components==3)});

}

void Plugin_framegrabber::SetCommandLineArguments(int argc, char* argv[]){
    Plugin::SetCommandLineArguments(argc, argv);
    InputParser input(argc, argv, this->GetCompactPluginName().toLower().toStdString());

    {const std::string &argument = input.getCmdOption("studioswing");
        if (!argument.empty()){
            std::dynamic_pointer_cast< ManagerType >(this->manager)->params.correct_studio_swing= atoi(argument.c_str());
        }}
    {const std::string &argument = input.getCmdOption("resolution");
        if (!argument.empty()){
            std::dynamic_pointer_cast< ManagerType >(this->manager)->params.pixel_size[0]= atof(argument.c_str());
            std::dynamic_pointer_cast< ManagerType >(this->manager)->params.pixel_size[1]= atof(argument.c_str());
            std::dynamic_pointer_cast< ManagerType >(this->manager)->params.pixel_size[0]= 1.0;
        }}
    {const std::string &argument = input.getCmdOption("color");
        if (!argument.empty()){
            if (atoi(argument.c_str()) == 1){
                std::dynamic_pointer_cast< ManagerType >(this->manager)->params.n_components = 3;
            } else {
                std::dynamic_pointer_cast< ManagerType >(this->manager)->params.n_components = 1;
            }

        }}
    // no need to add above since already in plugin
    {const std::string &argument = input.getCmdOption("framerate");
        if (!argument.empty()){
            std::dynamic_pointer_cast< ManagerType >(this->manager)->params.CaptureFrameRate = atof(argument.c_str());
        }}
    {const std::string &argument = input.getCmdOption("verbose");
        if (!argument.empty()){
            std::dynamic_pointer_cast< ManagerType >(this->manager)->params.verbose= atoi(argument.c_str());
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


