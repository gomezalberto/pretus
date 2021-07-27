#include "Plugin_planeDetection.h"
#include <generated/plugin_planeDetection_config.h>
#include <ifindImagePeriodicTimer.h>
#include <QObject>
#include <QSlider>

Q_DECLARE_METATYPE(ifind::Image::Pointer)
Plugin_planeDetection::Plugin_planeDetection(QObject *parent) : Plugin(parent)
{
    {
        WorkerType::Pointer worker_ = WorkerType::New();
        worker_->python_folder = std::string(planeDetection::getPythonFolder());
        this->worker = worker_;
    }
    this->mStreamTypes = ifind::InitialiseStreamTypeSetFromString("Input");
    this->setFrameRate(20); // by default 15fps
    this->Timer->SetDropFrames(true);

    {
        // create widget
        WidgetType * mWidget_ = new WidgetType;
        this->mWidget = mWidget_;

        WorkerType *w = std::dynamic_pointer_cast< WorkerType >(this->worker).get();
        QObject::connect(mWidget_->mSlider,
                &QSlider::valueChanged, w,
                &WorkerType::slot_bckThresholdValueChanged);

        QObject::connect(mWidget_->mSliderTA,
                &QSlider::valueChanged, w,
                &WorkerType::slot_temporalAverageValueChanged);
    }

    this->SetDefaultArguments();
}

void Plugin_planeDetection::Initialize(void){

    Plugin::Initialize();
    this->worker->Initialize();
    // Retrieve the list of classes and create a blank image with them as meta data.
    QStringList labels = std::dynamic_pointer_cast< WorkerType >(this->worker)->getLabels();
    ifind::Image::Pointer configuration = ifind::Image::New();
    //configuration->SetMetaData<QStringList>("StandardPlanes",labels);
    configuration->SetMetaData<std::string>("PythonInitialized",this->GetPluginName().toStdString());
    Q_EMIT this->ConfigurationGenerated(configuration);

    this->Timer->Start(this->TimerInterval);

}

void Plugin_planeDetection::slot_configurationReceived(ifind::Image::Pointer image){
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

void Plugin_planeDetection::SetDefaultArguments(){
// arguments are defined with: name, placeholder for value, argument type,  description, default value
mArguments.push_back({"taverage", "<val>",
                     QString( Plugin::ArgumentType[1] ),
                     "Number of frames used for a temporal average of the detection.",
                     QString::number(std::dynamic_pointer_cast< WorkerType >(this->worker)->temporalAverage)});

mArguments.push_back({"modelname", "<*.pth>",
                     QString( Plugin::ArgumentType[3] ),
                     "Model file name (without folder).",
                     QString(std::dynamic_pointer_cast< WorkerType >(this->worker)->modelname.c_str())});

mArguments.push_back({"bckth", "<val>",
                     QString( Plugin::ArgumentType[2] ),
                     "Min value for background to be considered; below this value, background will be ignored and second best picked. Range is [0.0, 1.0] If -1, ths flag is not used.",
                     QString::number(std::dynamic_pointer_cast< WorkerType >(this->worker)->background_threshold)});

mArguments.push_back({"savebck", "<0/1>",
                     QString( Plugin::ArgumentType[1] ),
                     "Whether to save background images to file (1, in this stream) or not (0).",
                     QString::number(std::dynamic_pointer_cast< WorkerType >(this->worker)->m_write_background)});


}

void Plugin_planeDetection::SetCommandLineArguments(int argc, char* argv[]){
    Plugin::SetCommandLineArguments(argc, argv);
    InputParser input(argc, argv, this->GetCompactPluginName().toLower().toStdString());

    {const std::string &argument = input.getCmdOption("taverage");
        if (!argument.empty()){
            std::dynamic_pointer_cast< WorkerType >(this->worker)->temporalAverage= atoi(argument.c_str());
        }}
    {const std::string &argument = input.getCmdOption("modelname");
        if (!argument.empty()){
            std::dynamic_pointer_cast< WorkerType >(this->worker)->modelname = argument.c_str();
        }}
    {const std::string &argument = input.getCmdOption("bckth");
        if (!argument.empty()){
            std::dynamic_pointer_cast< WorkerType >(this->worker)->background_threshold = atof(argument.c_str());
        }}
    {const std::string &argument = input.getCmdOption("savebck");
        if (!argument.empty()){
            std::dynamic_pointer_cast< WorkerType >(this->worker)->m_write_background = atoi(argument.c_str());
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
        return new Plugin_planeDetection();
    }
    #else
    Plugin* construct()
    {
        return new Plugin_planeDetection();
    }
    #endif // WIN32
}
