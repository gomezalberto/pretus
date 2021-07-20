#include "Plugin.h"
#include <ifindImagePeriodicTimer.h>
#include <iostream>
#include <sstream>
#include <QCheckBox>

/// Declare the ifind::Image::Pointer as metatype so that it can be sent via signal / slot.
Q_DECLARE_METATYPE(ifind::Image::Pointer)

const char * const Plugin::ArgumentType[] = {"BOOL", "INT", "FLOAT", "STRING" };

Plugin::Plugin(QObject *parent)
    : QObject(parent)
{
    /// Register dnl::Image::Pointer so that it can be sent via signal / slot
    qRegisterMetaType< ifind::Image::Pointer >();
    this->mVerbose = false;
    this->Active = false;
    this->setFrameRate(20); // method also sets timer interval
    this->Timer = new ifindImagePeriodicTimer(this); // as a child of this, automatic cleanup
    this->mStreamTypes = ifind::InitialiseStreamTypeSetFromString("-"); // by default, all streams
    this->isTimed = true;
    this->mWidget = nullptr;
    this->mImageWidget = nullptr;

    this->SetDefaultArguments();

    /**
   * connect the timer to the processor: this should be done in each plugin
   * QObject::connect(this->Timer, SIGNAL(ImageReceived(ifind::Image::Pointer)),
   *                   Processor, SLOT(Do Processing(ifind::Image::Pointer)));
   */

}

void Plugin::Initialize(void){

    if (this->mWidget != nullptr){
        this->mWidget->setPluginName(this->GetCompactPluginName());

        /// Need to connect the plugin signal, instead of the worker signal, because
        /// the plugin will beforehand add the stream type.
        QObject::connect(this, &Plugin::ImageGenerated,
                         this->mWidget, &QtPluginWidgetBase::SendImageToWidget);

    }



    if (this->mImageWidget != nullptr){
        if (this->mImageWidget->GetWidgetLocation() == QtPluginWidgetBase::WidgetLocation::hidden){
            //this->mImageWidget = nullptr; // creates segfault
        } else {
            //std::cout << "\tPlugin::Initialized setting the widget for "<< this->GetPluginName().toStdString() <<std::endl;
            this->mImageWidget->setPluginName(this->GetCompactPluginName());

            /// Need to connect the plugin signal, instead of the worker signal, because
            /// the plugin will beforehand add the stream type.
            QObject::connect(this, &Plugin::ImageGenerated,
                             this->mImageWidget, &QtPluginWidgetBase::SendImageToWidget);
        }
    }
    if ( (this->mWidget != nullptr) &&
         (this->mImageWidget != nullptr)){
        if (this->mWidget->mViewImageCheckbox != nullptr){
            QObject::connect(this->mWidget->mViewImageCheckbox, &QCheckBox::stateChanged,
                             this->mImageWidget, &QtPluginWidgetBase::SetImageWidgetVisibility);
            QObject::connect(this->mWidget->mViewImageCheckbox, &QCheckBox::stateChanged,
                             this, &Plugin::slot_updateGUI);

        }
    }

    if (worker == nullptr){
        /// this happens most likely because the plug-in is a source
        /// and has a manager, not a worker! of if this plug-in does not need an Initialise method.
        return;
    }

    worker->setPluginName(this->GetCompactPluginName());

    QObject::connect(this->worker.get(), &Worker::ConfigurationGenerated,
                     this, &Plugin::slot_passConfiguration);




    ///---------------
    ///  If not a direct connection, the slot is executed in the receivers thread
    if (this->isTimed){
        bool connected = QObject::connect(this->Timer, &ifindImagePeriodicTimer::ImageReceived,
                                          this->worker.get(), &Worker::slot_Work, Qt::QueuedConnection); /// Cannot be a direct connection! but in the workstation, if it is not, it does not work !:-(, Qt::DirectConnection);
        assert(connected);
    } else {
        QObject::connect(this, &Plugin::SendImageToWorker,
                         this->worker.get(), &Worker::slot_Work); /// Must be a direct connection!
    }
    QObject::connect(this->worker.get(), &Worker::WorkFinished,
                     this->Timer, &ifindImagePeriodicTimer::ReadyToShootOn); /// Possibly can be a direct connection
    QObject::connect(this->worker.get(), &Worker::ImageProcessed,
                     this, &Plugin::slot_imageProcessed);  /// Possibly can be a direct connection

    ///--------------
    QObject::connect(&this->workerThread, &QThread::finished,
                     this->worker.get(), &QObject::deleteLater, Qt::QueuedConnection);

    this->worker.get()->moveToThread(&this->workerThread);
    workerThread.start();
}

void Plugin::slot_updateGUI(){
    ifind::Image::Pointer config= ifind::Image::New();
    config->SetMetaData<std::string>("UpdateGUI","true");
    Q_EMIT this->ConfigurationGenerated(config);
}

bool Plugin::IsActive() const
{
    return this->Active;
}

double Plugin::getFrameRate() const
{
    return this->FrameRate;
}

QtPluginWidgetBase *Plugin::GetWidget(){
    return this->mWidget;
}

QtPluginWidgetBase *Plugin::GetImageWidget(){
    return this->mImageWidget;
}

bool Plugin::IntegratesWidgets(){
    return false;
}

void Plugin::setFrameRate(double value)
{
    /// These values are made effective in the initialization
    this->FrameRate = value;
    this->TimerInterval = static_cast<unsigned int>(1000.0/value);
}

unsigned int Plugin::getTimerInterval() const
{
    return this->TimerInterval;
}

void Plugin::setTimerInterval(unsigned int value)
{
    this->TimerInterval = value;
    this->FrameRate = 1000.0/static_cast<double>(value);
}

void Plugin::slot_imageProcessed(ifind::Image::Pointer image){
    image->SetStreamType(this->GetCompactPluginName().toStdString());
    Q_EMIT this->ImageGenerated(image);
}

void Plugin::slot_imageReceived(ifind::Image::Pointer image){
    /// Send the image for processing. This image may or may not
    /// be processed depending on the timer's frame rate

    if (ifind::IsImageOfStreamTypeSet(image, mStreamTypes)){
        if (this->isTimed){
            this->Timer->SetIfindImage(image);
        } else {
            this->SendImageToWorker(image);
        }
    }

    /// Send the image through the pipeline
    Q_EMIT this->ImageGenerated(image);
}

void Plugin::slot_passConfiguration(ifind::Image::Pointer config){
    Q_EMIT this->ConfigurationGenerated(config);
};

void Plugin::slot_configurationReceived(ifind::Image::Pointer image){
    /// By default, pass the configuration to the next plug-in
    Q_EMIT this->ConfigurationGenerated(image);
}

void Plugin::RemoveArgument(const QString &name){

    std::vector<QStringList> copy;

    for ( int i=0; i < mGenericArguments.size(); i++ ){
        if (mGenericArguments[i][0] != name){
            copy.push_back(mGenericArguments[i]);
        }
    }
    mGenericArguments = copy;

    std::vector<QStringList> copy2;

    for ( int i=0; i < mArguments.size(); i++ ){
        if (mArguments[i][0] != name){
            copy2.push_back(mArguments[i]);
        }
    }
    mArguments = copy2;

}

void Plugin::SetDefaultArguments(){
    // arguments are defined with: name, placeholder for value, argument type,  description, default value
    mGenericArguments.push_back({"framerate", "<val>",
                                 QString( ArgumentType[2] ),
                                 "Frame rate at which the plugin does the work.",
                                 "20"});
    mGenericArguments.push_back({"verbose", "<val>",
                                 QString( ArgumentType[0] ),
                                 "Whether to print debug information (1) or not (0).",
                                 "0"});
    mGenericArguments.push_back({"time", "<val>",
                                 QString( ArgumentType[0] ),
                                 "Whether to measure execution time (1) or not (0).",
                                 "0"});
    mGenericArguments.push_back({"showimage", "<val>",
                                 QString( ArgumentType[1] ),
                                 "Whether to display realtime image outputs in the central window  (1) or not (0).",
                                 "<1 for input plugins, 0 for the rest>"});
    mGenericArguments.push_back({"showwidget", "<val>",
                                 QString( ArgumentType[1] ),
                                 "Whether to display widget with plugin information (1-4) or not (0). Location is 1- top left, 2- top right, 3-bottom left, 4-bottom right.",
                                 "visible, default location depends on widget."});
}

void Plugin::SetCommandLineArguments(int argc, char* argv[]){   
    InputParser input(argc, argv, this->GetCompactPluginName().toLower().toStdString());
    {const std::string &argument = input.getCmdOption("framerate");
        if (!argument.empty()){
            this->setFrameRate(atof(argument.c_str()));
        }}
    {const std::string &argument = input.getCmdOption("verbose");
        if (!argument.empty()){
            this->mVerbose = atoi(argument.c_str());
        }}
    {const std::string &argument = input.getCmdOption("time");
        if (!argument.empty()){
            this->worker->params.measureTime = atoi(argument.c_str());
        }}
    {const std::string &argument = input.getCmdOption("showimage");
        if (!argument.empty() && (this->GetImageWidget() != nullptr)){
            if (atoi(argument.c_str()) > 0 ){
                this->GetImageWidget()->SetWidgetLocation(QtPluginWidgetBase::WidgetLocation::visible);
            } else if (atoi(argument.c_str()) == 0 ){
                this->GetImageWidget()->SetWidgetLocation(QtPluginWidgetBase::WidgetLocation::hidden);
            } else {
                /// @todo implement overlays
                this->GetImageWidget()->SetWidgetLocation(QtPluginWidgetBase::WidgetLocation::hidden);
                //this->GetImageWidget()->SetWidgetLocation(QtPluginWidgetBase::WidgetLocation::overlaid);
            }

        }}
    {const std::string &argument = input.getCmdOption("showwidget");
        if (!argument.empty() && (this->GetWidget() != nullptr)){
            if (atoi(argument.c_str()) <= 0 ){
                this->GetWidget()->SetWidgetLocation(QtPluginWidgetBase::WidgetLocation::hidden);
            } else if (atoi(argument.c_str()) == 1 ){
                this->GetWidget()->SetWidgetLocation(QtPluginWidgetBase::WidgetLocation::top_left);
            } else if (atoi(argument.c_str()) == 2 ){
                this->GetWidget()->SetWidgetLocation(QtPluginWidgetBase::WidgetLocation::top_right);
            } else if (atoi(argument.c_str()) == 3 ){
                this->GetWidget()->SetWidgetLocation(QtPluginWidgetBase::WidgetLocation::bottom_left);
            } else if (atoi(argument.c_str()) == 1 ){
                this->GetWidget()->SetWidgetLocation(QtPluginWidgetBase::WidgetLocation::bottom_right);
            }

        }}
}

void Plugin::Usage(void){
    std::cout << std::endl;
    std::cout << "# PLUGIN "<< this->GetPluginName().toStdString()<<std::endl;
    std::cout << "   "<< this->GetPluginDescription().toStdString() <<std::endl;
    /// @todo: add a call for the summary of each plugin
    for (int i=0; i<mGenericArguments.size(); i++){

        std::stringstream argname;
        argname << "\t"<< ("--" + this->GetCompactPluginName().toLower() + "_" + mGenericArguments[i][0]).toStdString() << " "
        << mGenericArguments[i][1].toStdString() << " [ type: "<< mGenericArguments[i][2].toStdString() <<"]";
        std::stringstream argdescription;
        argdescription <<  "\t" << mGenericArguments[i][3].toStdString() << " (Default: "<< mGenericArguments[i][4].toStdString()<<")";

        this->printIndented(argname, argdescription, 80);
    }

    if (mArguments.size()>0){
        std::cout << "   Plugin-specific arguments:"<<std::endl;
        for (int i=0; i<mArguments.size(); i++){

        std::stringstream argname;
        argname << "\t"<< ("--" + this->GetCompactPluginName().toLower() + "_" + mArguments[i][0]).toStdString() << " "
        << mArguments[i][1].toStdString() << " [ type: "<< mArguments[i][2].toStdString() <<"]";
        std::stringstream argdescription;
        argdescription << "\t" << mArguments[i][3].toStdString() << " (Default: "<< mArguments[i][4].toStdString()<<")"<<std::endl;
        this->printIndented(argname, argdescription, 80);
        }
    }
}

void Plugin::printIndented(std::stringstream &argname, std::stringstream &argdescription, unsigned int max_line_length){
    std::cout << argname.str();

    /// now make it fold
    const int initial_length= argname.str().length();
    const std::string line_prefix(initial_length, ' ');

    std::istringstream text_iss(argdescription.str());
    std::string word;
    unsigned characters_written = 0;
    //std::cout << line_prefix; // not in the first line
    std::cout << "\t"; // not in the first line
    while (text_iss >> word) {

        if (word.size() + characters_written > max_line_length) {
            std::cout << "\n" << line_prefix << "\t\t";
            characters_written = 0;
        }

        std::cout << word << " ";
        characters_written += word.size() + 1;
    }
    std::cout << std::endl;
}
