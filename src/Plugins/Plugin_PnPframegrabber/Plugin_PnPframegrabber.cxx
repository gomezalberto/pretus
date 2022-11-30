#include "Plugin_PnPframegrabber.h"
#include <QObject>
#include <QPushButton>
#include <QComboBox>

Q_DECLARE_METATYPE(ifind::Image::Pointer)
Plugin_PnPframegrabber::Plugin_PnPframegrabber(QObject *parent) : Plugin(parent)
{
    this->mIsInput = true;
    {
        ManagerType::Pointer manager_ = ManagerType::New();
        this->manager = manager_;
    }
    {
        // create widget
        WidgetType * mWidget_ = new WidgetType;
        std::vector<int> framerates = {5, 10, 15, 20, 25, 30, 60};
        mWidget_->setFrameRates(framerates);
        mWidget_->setSelectedFramerate(std::dynamic_pointer_cast< ManagerType >(this->manager)->params.CaptureFrameRate);

        std::vector<std::string> resolutions = {"1600x1200 (4:3)", "1280x960  (4:3)",
                                                "1024x768  (4:3)", "960x720 (4:3)", "800x600  (4:3)","640x480  (4:3)","320x240  (4:3)",
                                                "1920x1080 (16:9)", "1600x896 (16:9)","1360x768 (16:9)", "1280x720 (16:9)",
                                                "1280x1024 (other)", "720x576 (other)", "720x480 (other)",};
        mWidget_->setResolutions(resolutions);
        mWidget_->setSelectedResolution(std::dynamic_pointer_cast< ManagerType >(this->manager)->params.resolution);

        std::vector<std::string> encodings = {"Full", "MJPEG", "H.264"};
        mWidget_->setEncodings(encodings);
        mWidget_->setSelectedEncoding(QString("MJPEG"));

        this->mWidget = mWidget_;

        ManagerType *w = std::dynamic_pointer_cast< ManagerType >(this->manager).get();
        QObject::connect(mWidget_->mPausePlayButton,
                &QPushButton::toggled, w,
                &ManagerType::slot_togglePlayPause);

        QObject::connect(mWidget_->mPausePlayButton,
                &QPushButton::toggled, mWidget_,
                &WidgetType::slot_togglePlayPause);

        QObject::connect(mWidget_,
                &WidgetType::signal_newFrameRate, w,
                &ManagerType::slot_updateFrameRate);

        QObject::connect(mWidget_,
                &WidgetType::signal_newResolution, w,
                &ManagerType::slot_updateResolution);

        QObject::connect(mWidget_,
                &WidgetType::signal_newEncoding, w,
                &ManagerType::slot_updateEncoding);

    }
    {
        // create image widget
        ImageWidgetType * mWidget_ = new ImageWidgetType;
        this->mImageWidget = mWidget_;
        this->mImageWidget->SetWidgetLocation(ImageWidgetType::WidgetLocation::visible);

    }

    QObject::connect(this->manager.get(), &ManagerType::ImageGenerated,
                     this, &Plugin_PnPframegrabber::slot_imageReceived, Qt::DirectConnection);

    this->SetDefaultArguments();
}


void Plugin_PnPframegrabber::Initialize(void){
    Plugin::Initialize();
    if (this->mImageWidget != nullptr){
        reinterpret_cast< ImageWidgetType *>(this->mImageWidget)->Initialize();
    }
    std::dynamic_pointer_cast< ManagerType >(this->manager)->Initialize();
}

void Plugin_PnPframegrabber::SetDefaultArguments(){
    this->RemoveArgument("stream");
    this->RemoveArgument("layer");
    this->RemoveArgument("time");
    this->RemoveArgument("framerate");

    // arguments are defined with: name, placeholder for value, argument type,  description, default value
    mArguments.push_back({"studioswing", "<val>",
                          QString( Plugin::ArgumentType[1] ),
                          "Correct for studio swing (>0) or not (0). The value (typically 16) indicates how much to correct for.",
                          QString::number(std::dynamic_pointer_cast< ManagerType >(this->manager)->params.correct_studio_swing)});

    mArguments.push_back({"resolution", "width.height",
                          QString( Plugin::ArgumentType[3] ),
                          "Number of pixels of the video stream, separated by a dot. Accepted values are, in 16:9: 1920.1080, 1360.768, 1280.720; in 4:3: 1600.1200, 1280.960, 1024.768, 800.600, 640.480; and other: 1280.1024, 720.576, 720.480",
                          std::dynamic_pointer_cast< ManagerType >(this->manager)->params.resolution});

    mArguments.push_back({"pixelsize", "<val>",
                          QString( Plugin::ArgumentType[2] ),
                          "Value, in mm, of the pixel size (isotropic).",
                          QString::number(std::dynamic_pointer_cast< ManagerType >(this->manager)->params.pixel_size[0])});

    mArguments.push_back({"color", "<0/1>",
                          QString( Plugin::ArgumentType[0] ),
                          "USe color images (1) or not (0).",
                          QString::number(std::dynamic_pointer_cast< ManagerType >(this->manager)->params.n_components==3)});

    mArguments.push_back({"camid", "<val>",
                          QString( Plugin::ArgumentType[1] ),
                          "camera id.",
                          QString::number(std::dynamic_pointer_cast< ManagerType >(this->manager)->params.cam_id)});

    mArguments.push_back({"framerate", "<val>",
                         QString( ArgumentType[2] ),
                         "Frame rate at which the framegrabber captures data.",
                         QString::number(std::dynamic_pointer_cast< ManagerType >(this->manager)->params.CaptureFrameRate)});


}

void Plugin_PnPframegrabber::SetCommandLineArguments(int argc, char* argv[]){
    Plugin::SetCommandLineArguments(argc, argv);
    InputParser input(argc, argv, this->GetCompactPluginName().toLower().toStdString());

    {const std::string &argument = input.getCmdOption("studioswing");
        if (!argument.empty()){
            std::dynamic_pointer_cast< ManagerType >(this->manager)->params.correct_studio_swing= atoi(argument.c_str());
        }}
    {const std::string &argument = input.getCmdOption("pixelsize");
        if (!argument.empty()){
            std::dynamic_pointer_cast< ManagerType >(this->manager)->params.pixel_size[0]= atof(argument.c_str());
            std::dynamic_pointer_cast< ManagerType >(this->manager)->params.pixel_size[1]= atof(argument.c_str());
            std::dynamic_pointer_cast< ManagerType >(this->manager)->params.pixel_size[0]= 1.0;
        }}
    {const std::string &argument = input.getCmdOption("resolution");
        if (!argument.empty()){
            std::dynamic_pointer_cast< ManagerType >(this->manager)->params.resolution= QString(argument.c_str());
            std::dynamic_pointer_cast< ManagerType >(this->manager)->slot_updateResolution( QString(argument.c_str()) );
            WidgetType* ww = reinterpret_cast<WidgetType*>(mWidget);
            ww->setSelectedResolution(QString(argument.c_str()));

        }}
    {const std::string &argument = input.getCmdOption("color");
        if (!argument.empty()){
            if (atoi(argument.c_str()) == 1){
                std::dynamic_pointer_cast< ManagerType >(this->manager)->params.n_components = 3;
            } else {
                std::dynamic_pointer_cast< ManagerType >(this->manager)->params.n_components = 1;
            }
        }}
    {const std::string &argument = input.getCmdOption("framerate");
        if (!argument.empty()){
            std::dynamic_pointer_cast< ManagerType >(this->manager)->params.CaptureFrameRate = atof(argument.c_str());
        }}
    // no need to add above since already in plugin

    {const std::string &argument = input.getCmdOption("verbose");
        if (!argument.empty()){
            std::dynamic_pointer_cast< ManagerType >(this->manager)->params.verbose= atoi(argument.c_str());
        }}
    {const std::string &argument = input.getCmdOption("camid");
        if (!argument.empty()){
            std::dynamic_pointer_cast< ManagerType >(this->manager)->params.cam_id= atoi(argument.c_str());
        }}
}

extern "C"
{
#ifdef WIN32
/// Function to return an instance of a new LiveCompoundingFilter object
__declspec(dllexport) Plugin* construct()
{
    return new Plugin_PnPframegrabber();
}
#else
Plugin* construct()
{
    return new Plugin_PnPframegrabber();
}
#endif // WIN32
}


