#include "Plugin_GUI.h"
#include "QtVisualizationMainWindow.h"
#include <ifindImagePeriodicTimer.h>
#include <QObject>
#include <QDebug>
#include <QSlider>

Q_DECLARE_METATYPE(ifind::Image::Pointer)
Plugin_GUI::Plugin_GUI(QObject *parent)
    :
      Plugin(parent)
{
    // the plugin needs to accept all streams by default,
    // then the indivdual widgets will decide what to take
    this->mStreamTypes = ifind::InitialiseStreamTypeSetFromString("");
    this->setFrameRate(25);
    //this->Timer->SetDropFrames(true);

    //mVisualizer = std::make_shared<QtVisualizationMainWindow>();

    {
        // create widget
        WidgetType * mWidget_ = new WidgetType;
        this->mWidget = mWidget_;
    }


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
    mVisualizer = std::make_shared<QtVisualizationMainWindow>();
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

    mVisualizer->Initialize();
    this->Timer->Start(this->TimerInterval);
}

void Plugin_GUI::slot_configurationReceived(ifind::Image::Pointer image){
    /// Check if there is any need to update the gui
    if (image->HasKey("UpdateGUI")){
        this->mVisualizer->InitializeCentralPanel();
    }

    /// Pass on the message in case we need to "jump" over plug-ins
    Q_EMIT this->ConfigurationGenerated(image);
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
    this->RemoveArgument("showwidget");
}



void Plugin_GUI::SetCommandLineArguments(int argc, char* argv[])
{
    Plugin::SetCommandLineArguments(argc, argv);
    InputParser input(argc, argv, this->GetCompactPluginName().toLower().toStdString());

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


