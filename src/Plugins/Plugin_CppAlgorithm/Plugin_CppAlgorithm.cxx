#include "Plugin_CppAlgorithm.h"
#include <ifindImagePeriodicTimer.h>
#include <QObject>
#include <QSlider>

Q_DECLARE_METATYPE(ifind::Image::Pointer)
Plugin_CppAlgorithm::Plugin_CppAlgorithm(QObject *parent) : Plugin(parent)
{
    // Construct the worker
    {
        WorkerType::Pointer worker_ = WorkerType::New();
        this->worker = worker_;
    }
    this->mStreamTypes = ifind::InitialiseStreamTypeSetFromString("Input");
    this->setFrameRate(20); // by default 15fps
    this->Timer->SetDropFrames(true);

    {
        // create widget
        WidgetType * mWidget_ = new WidgetType;
        this->mWidget = mWidget_;

        // Connect widget and worker here
        WorkerType *w = std::dynamic_pointer_cast< WorkerType >(this->worker).get();
        QObject::connect(mWidget_->mSlider,
                &QSlider::valueChanged, w,
                &WorkerType::slot_thresholdValueChanged);
    }
    {
        // create image widget
        ImageWidgetType * mWidget_ = new ImageWidgetType;
        this->mImageWidget = mWidget_;
        this->mImageWidget->SetStreamTypes(ifind::InitialiseStreamTypeSetFromString(this->GetCompactPluginName().toStdString()));
        this->mImageWidget->SetWidgetLocation(ImageWidgetType::WidgetLocation::hidden); // by default, do not show

        // set image viewer default options:
        // overlays, colormaps, etc
        ImageWidgetType::Parameters default_params = mWidget_->Params();
        default_params.SetBaseLayer(0); // use the input image as background image
        default_params.SetDisplayMultiLayers(1); // show 1 layer on top of the background
        default_params.SetLutId(5);
        default_params.SetShowColorbar(false);
        mWidget_->SetParams(default_params);
    }

    this->SetDefaultArguments();
}

void Plugin_CppAlgorithm::Initialize(void){

    Plugin::Initialize();
    reinterpret_cast< ImageWidgetType *>(this->mImageWidget)->Initialize();
    this->worker->Initialize();

//    ifind::Image::Pointer configuration = ifind::Image::New();
//    Q_EMIT this->ConfigurationGenerated(configuration);

    this->Timer->Start(this->TimerInterval);
}

void Plugin_CppAlgorithm::slot_configurationReceived(ifind::Image::Pointer image){

    /// Pass on the message in case we need to "jump" over plug-ins
    Q_EMIT this->ConfigurationGenerated(image);
}

void Plugin_CppAlgorithm::SetDefaultArguments(){

    // arguments are defined with: name, placeholder for value, argument type,  description, default value
    mArguments.push_back({"threshold", "<val>",
                            QString( ArgumentType[1] ),
                            "Intensity threshold for segmentation.",
                            QString::number(std::dynamic_pointer_cast< WorkerType >(this->worker)->mThreshold )});

}

void Plugin_CppAlgorithm::SetCommandLineArguments(int argc, char* argv[]){
    Plugin::SetCommandLineArguments(argc, argv);
    InputParser input(argc, argv, this->GetCompactPluginName().toLower().toStdString());
    {const std::string &argument = input.getCmdOption("threshold");
        if (!argument.empty()){
            std::dynamic_pointer_cast< WorkerType >(this->worker)->mThreshold = atof(argument.c_str());
            reinterpret_cast<WidgetType*>(this->mWidget)->mSlider->setValue(atof(argument.c_str()));
        }}
}

extern "C"
{
    #ifdef WIN32
    /// Function to return an instance of a new LiveCompoundingFilter object
    __declspec(dllexport) Plugin* construct()
    {
        return new Plugin_CppAlgorithm();
    }
    #else
    Plugin* construct()
    {
        return new Plugin_CppAlgorithm();
    }
    #endif // WIN32
}
