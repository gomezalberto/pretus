#pragma once
#include <QMainWindow>
#include <Plugin.h>
#include <QtPluginWidgetBase.h>
#include <ifindImage.h>

class QtVTKVisualization;

class QtVisualizationMainWindow : public QMainWindow
{
    Q_OBJECT

public:

    QtVisualizationMainWindow(QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());

    static void Usage();
    void SetCommandLineArguments(const std::vector<std::string> &args);
    void SetWidgets(QList<QtPluginWidgetBase *> &widgets);
    void SetImageWidgets(QList<QtPluginWidgetBase *> &imageWidgets);

    bool useColors() const;
    void setUseColors(bool useColors);

public Q_SLOTS:
    virtual void slot_Terminate(void);
    virtual void SendImageToWidget(ifind::Image::Pointer image);
    virtual void Initialize();
    virtual void InitializeCentralPanel();
    virtual void SetViewScale(int viewScaleInt);

Q_SIGNALS:

    void SignalSendImageToWidget(ifind::Image::Pointer image);
    void SignalSetViewScale(float scale);

protected:

    virtual void resizeEvent(QResizeEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);

    // This is a raw pointer as using a weak pointer based upon a shared pointer
    // ends up null after the constructor, QT's pointers are a bit old-school
    // my apologies
    //QtVTKVisualization *mRenderWidget;
    QWidget *mCentralPanelWidget;
    QWidget *mLeftPanelWidget;
    QWidget *mRightPanelWidget;

    // These are the widgets that other plug-ins provide.
    // Location is defined in the plugin class
    QList<QtPluginWidgetBase *> mWidgets;
    QList<QtPluginWidgetBase *> mImageWidgets;

    QStringList mWidgetColors;
    bool mUseColors;



};
