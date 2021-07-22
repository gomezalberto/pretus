#pragma once

#include <Plugin.h>
#include "VideoManager.h"
#include "Widget_videomanager.h"
#include <QtVTKVisualization.h>

class Plugin_VideoManager : public Plugin {
	Q_OBJECT

public:
    typedef Widget_videomanager WidgetType;
    typedef QtVTKVisualization ImageWidgetType;
    Plugin_VideoManager(QObject* parent = 0);

    QString GetPluginName(void){ return "Video manager";}
    QString GetPluginDescription(void) {return "Reads video files from disk. File format depends on opencv installation.";}

    void SetCommandLineArguments(int argc, char* argv[]);

    void Initialize(void);

    virtual void SetActivate(bool arg){
        this->manager->SetActivate(arg);
    }

private:
    VideoManager::Pointer manager;
    virtual void SetDefaultArguments();
};
