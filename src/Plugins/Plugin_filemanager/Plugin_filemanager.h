#pragma once

#include <Plugin.h>
#include "FileManager.h"
#include "Widget_filemanager.h"
#include <QtVTKVisualization.h>

class Plugin_filemanager : public Plugin {
	Q_OBJECT

public:
    typedef Widget_filemanager WidgetType;
    typedef QtVTKVisualization ImageWidgetType;
    Plugin_filemanager(QObject* parent = 0);

    QString GetPluginName(void){ return "File manager";}
    QString GetPluginDescription(void){return "Reads and transmits images from a folder hierarchy.";}


    void SetCommandLineArguments(int argc, char* argv[]);

    void Initialize(void);

    virtual void SetActivate(bool arg){
        this->manager->SetActivate(arg);
    }

private:
    FileManager::Pointer manager;
    virtual void SetDefaultArguments();
};
