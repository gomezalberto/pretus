#include "Plugin_PythonAlgorithm.h"
#include <generated/plugin_PythonAlgorithm_config.h>
#include <ifindImagePeriodicTimer.h>
#include <QObject>
#include <QSlider>

Q_DECLARE_METATYPE(ifind::Image::Pointer)
Plugin_PythonAlgorithm::Plugin_PythonAlgorithm(QObject *parent) : Plugin(parent)
{
    // Construct the worker
    {
        WorkerType::Pointer worker_ = WorkerType::New();
        worker_->python_folder = std::string(PythonAlgorithm::getPythonFolder());
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
                &WorkerType::slot_sigmaValueChanged);
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
        default_params.SetBaseLayer(1); // use the input image as background image
        mWidget_->SetParams(default_params);

    }

    this->SetDefaultArguments();
}

void Plugin_PythonAlgorithm::Initialize(void){

    Plugin::Initialize();
    reinterpret_cast< ImageWidgetType *>(this->mImageWidget)->Initialize();
    this->worker->Initialize();

    ifind::Image::Pointer configuration = ifind::Image::New();
    configuration->SetMetaData<std::string>("PythonInitialized",this->GetPluginName().toStdString());
    Q_EMIT this->ConfigurationGenerated(configuration);

    this->Timer->Start(this->TimerInterval);
}

void Plugin_PythonAlgorithm::slot_configurationReceived(ifind::Image::Pointer image){
    if (image->HasKey("PythonInitialized")){
        std::string whoInitialisedThePythonInterpreter = image->GetMetaData<std::string>("PythonInitialized");
        std::cout << "[WARNING from "<< this->GetPluginName().toStdString() << "] Python interpreter already initialized by \""<< whoInitialisedThePythonInterpreter <<"\", no initialization required."<<std::endl;
        this->worker->setPythonInitialized(true);
    }

    if (image->HasKey("Python_gil_init")){
        std::cout << "[WARNING from "<< this->GetPluginName().toStdString() << "] Python Global Interpreter Lock already set by a previous plug-in."<<std::endl;
        this->worker->set_gil_init(1);
    }
    /// Pass on the message in case we need to "jump" over plug-ins
    Q_EMIT this->ConfigurationGenerated(image);
}

void Plugin_PythonAlgorithm::SetDefaultArguments(){

    // arguments are defined with: name, placeholder for value, argument type,  description, default value
    mArguments.push_back({"sigma", "<val>",
                          QString( ArgumentType[2] ),
                          "Sigma (in mm) for Gaussian blurring.",
                          QString::number(std::dynamic_pointer_cast< WorkerType >(this->worker)->mFsigma )});
    mArguments.push_back({"delay", "<val>",
                          QString( ArgumentType[2] ),
                          "Delay (in sec) for artificially slowing doen the execution.",
                          QString::number(std::dynamic_pointer_cast< WorkerType >(this->worker)->mDelay )});

}

void Plugin_PythonAlgorithm::SetCommandLineArguments(int argc, char* argv[]){
    Plugin::SetCommandLineArguments(argc, argv);
    InputParser input(argc, argv, this->GetCompactPluginName().toLower().toStdString());
    {const std::string &argument = input.getCmdOption("sigma");
        if (!argument.empty()){
            std::dynamic_pointer_cast< WorkerType >(this->worker)->mFsigma= atof(argument.c_str());
            reinterpret_cast<WidgetType*>(this->mWidget)->mSlider->setValue(atof(argument.c_str()));
        }}
    {const std::string &argument = input.getCmdOption("delay");
        if (!argument.empty()){
            std::dynamic_pointer_cast< WorkerType >(this->worker)->mDelay= atof(argument.c_str());
        }}
    // no need to add above since already in plugin
    {const std::string &argument = input.getCmdOption("verbose");
        if (!argument.empty()){
            this->worker->params.verbose= atoi(argument.c_str());
        }}
}


extern "C"
{
#ifdef WIN32
/// Function to return an instance of a new LiveCompoundingFilter object
__declspec(dllexport) Plugin* construct()
{
    return new Plugin_PythonAlgorithm();
}
#else
Plugin* construct()
{
    return new Plugin_PythonAlgorithm();
}
#endif // WIN32
}
