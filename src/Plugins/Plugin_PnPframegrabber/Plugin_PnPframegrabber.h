#pragma once

#include <Plugin.h>
#include "PnPFrameGrabberManager.h"
#include "Widget_PnPframegrabber.h"
#include <QtVTKVisualization.h>

class Plugin_PnPframegrabber : public Plugin {
    Q_OBJECT

public:

    typedef PnPFrameGrabberManager ManagerType;
    typedef Widget_PnPframegrabber WidgetType;
    typedef QtVTKVisualization ImageWidgetType;
    Plugin_PnPframegrabber(QObject* parent = 0);

    QString GetPluginName(void){ return "PnP Frame grabber";}
    QString GetPluginDescription(void) {return "Reads real-time imaging from a video source using a Plug & PLay capture card through V4L / OBS.";}

    void SetCommandLineArguments(int argc, char* argv[]);

    void Initialize(void);

    virtual void SetActivate(bool arg){
        this->manager->SetActivate(arg);
    }

protected:
    virtual void SetDefaultArguments();

};
