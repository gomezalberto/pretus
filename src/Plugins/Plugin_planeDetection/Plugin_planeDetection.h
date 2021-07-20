#pragma once

#include <Plugin.h>
#include "Worker_planeDetection.h"
#include "Widget_planeDetection.h"

class Plugin_planeDetection : public Plugin {
    Q_OBJECT

public:
    /**
     * @brief Convenience alias
     */
    typedef Worker_planeDetection WorkerType;
    typedef Widget_planeDetection WidgetType;
    Plugin_planeDetection(QObject* parent = 0);

    QString GetPluginName(void){ return "Standard plane detection";}
    QString GetPluginDescription(void) {return "Standard plane detection using SonoNet by Baumgartner et al.";}

    void SetCommandLineArguments(int argc, char* argv[]);

    void Initialize(void);

protected:
    virtual void SetDefaultArguments();

public Q_SLOTS:
    virtual void slot_configurationReceived(ifind::Image::Pointer image);


};
