#pragma once

#include <Plugin.h>
#include "FrameGrabberManager.h"
#include "Widget_framegrabber.h"
#include <QtVTKVisualization.h>

class Plugin_framegrabber : public Plugin {
    Q_OBJECT

public:

    typedef FrameGrabberManager ManagerType;
    typedef Widget_framegrabber WidgetType;
    typedef QtVTKVisualization ImageWidgetType;
    Plugin_framegrabber(QObject* parent = 0);

    QString GetPluginName(void){ return "Frame grabber";}
    QString GetPluginDescription(void) {return "Reads real-time imaging from a video source using the Epiphan DVI2USB 3.0  grabber.";}

    void SetCommandLineArguments(int argc, char* argv[]);

    void Initialize(void);

    virtual void SetActivate(bool arg){
        this->manager->SetActivate(arg);
    }

protected:
    virtual void SetDefaultArguments();

};
