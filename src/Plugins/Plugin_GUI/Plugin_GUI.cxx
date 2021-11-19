#include "Plugin_GUI.h"
#include "QtVisualizationMainWindow.h"
#include <ifindImagePeriodicTimer.h>
#include <QObject>
#include <QDebug>
#include <QSlider>
#include <QPushButton>

Q_DECLARE_METATYPE(ifind::Image::Pointer)
Plugin_GUI::Plugin_GUI(QObject *parent)
    :
      Plugin(parent)
{
    // the plugin needs to accept all streams by default,
    // then the indivdual widgets will decide what to take
    this->mStreamTypes = ifind::InitialiseStreamTypeSetFromString("");
    this->setFrameRate(30);
    //this->Timer->SetDropFrames(true);

    //mVisualizer = std::make_shared<QtVisualizationMainWindow>();

    {
        // create widget
        WidgetType * mWidget_ = new WidgetType;
        this->mWidget = mWidget_;
    }

    mVisualizer = std::make_shared<QtVisualizationMainWindow>();
    this->SetDefaultArguments();
}

void Plugin_GUI::SetActivate(bool arg)
{
    if (!arg){
        mVisualizer->slot_Terminate();
    }
}

void Plugin_GUI::Initialize()
{
    // chances are the visualiser would not have been created when the args were set
    //mVisualizer = std::make_shared<QtVisualizationMainWindow>();
    mVisualizer->SetCommandLineArguments(mArgs);

    mVisualizer->SetWidgets(mWidgets);
    mVisualizer->SetImageWidgets(mImageWidgets);

    // make sure thatwhen the webcam generates an image, this plugin emits that image
    QObject::connect(this->Timer, &ifindImagePeriodicTimer::ImageReceived,
                     mVisualizer.get(), &QtVisualizationMainWindow::SendImageToWidget, Qt::DirectConnection); //, Qt::QueuedConnection) like this also blocks; // Qt::DirectConnection) -> like this blocks;

    // Connect widget and worker here
    WidgetType* ww = reinterpret_cast<WidgetType*>(mWidget);
    QObject::connect(ww->mSlider,
            &QSlider::valueChanged, mVisualizer.get(),
            &QtVisualizationMainWindow::SetViewScale);

   // QObject::connect(ww->mResetButton,
   //         &QPushButton::released, mVisualizer.get(),
   //         &QtVisualizationMainWindow::ResetViewScale);

    QObject::connect(ww->mResetButton,
            &QPushButton::released, this,
            &Plugin_GUI::slot_resetScale);

    mVisualizer->Initialize();
    this->Timer->Start(this->TimerInterval);
}

void Plugin_GUI::slot_resetScale(){
    WidgetType* ww = reinterpret_cast<WidgetType*>(mWidget);
    ww->mSlider->setValue(50);
}

void Plugin_GUI::slot_configurationReceived(ifind::Image::Pointer image){
    Plugin::slot_configurationReceived(image);
    /// Check if there is any need to update the gui
    if (image->HasKey("UpdateGUI")){
        this->mVisualizer->InitializeCentralPanel();
    }
}

bool Plugin_GUI::IntegratesWidgets(){
    return true;
}

void Plugin_GUI::SetWidgets(QList<QtPluginWidgetBase *> &widgets){
    this->mWidgets = widgets;
}

void Plugin_GUI::SetImageWidgets(QList<QtPluginWidgetBase *> &imageWidgets){
    this->mImageWidgets = imageWidgets;
}

void Plugin_GUI::SetDefaultArguments(){
    this->RemoveArgument("stream");
    this->RemoveArgument("layer");
    this->RemoveArgument("showimage");

    mArguments.push_back({"usecolors", "<val>",
                            QString( ArgumentType[0] ),
                            "Wether use colours for widgets or not.",
                            QString::number(this->mVisualizer->useColors())});
}



void Plugin_GUI::SetCommandLineArguments(int argc, char* argv[])
{
    Plugin::SetCommandLineArguments(argc, argv);
    InputParser input(argc, argv, this->GetCompactPluginName().toLower().toStdString());

    {const std::string &argument = input.getCmdOption("usecolors");
        if (!argument.empty()){
            this->mVisualizer->setUseColors( atoi(argument.c_str()));
        }}

    mArgs = std::vector<std::string>(argv, argv + argc);


    if (nullptr != mVisualizer) {
        mVisualizer->SetCommandLineArguments(mArgs);
    }
}

extern "C"
{
#ifdef WIN32
__declspec(dllexport) Plugin* construct()
{
    return new Plugin_GUI();
}
#else
Plugin* construct()
{
    return new Plugin_GUI();
}
#endif // WIN32
}


