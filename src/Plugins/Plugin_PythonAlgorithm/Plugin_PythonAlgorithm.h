#pragma once

#include <Plugin.h>
#include "Worker_PythonAlgorithm.h"
#include "Widget_PythonAlgorithm.h"
#include <QtVTKVisualization.h>

class Plugin_PythonAlgorithm : public Plugin {
    Q_OBJECT

public:

    typedef Worker_PythonAlgorithm WorkerType;
    typedef Widget_PythonAlgorithm WidgetType;
    typedef QtVTKVisualization ImageWidgetType;

    Plugin_PythonAlgorithm(QObject* parent = 0);

    QString GetPluginName(void){ return "Python Algorithm";}
    QString GetPluginDescription(void) {return "Sample plug-in that does a simple Gaussian blurring.";}

    void SetCommandLineArguments(int argc, char* argv[]);

    /**
    * Initialize the plug-in: take in the command line arguments,
    * and any configuration passed on by previous plug-ins. Send
    * configuration options to downstream plug-ins.
    */
    void Initialize(void);


public Q_SLOTS:
    /**
     * Receive configuration information from upstream plug-ins
     */
    virtual void slot_configurationReceived(ifind::Image::Pointer image);

protected:
    virtual void SetDefaultArguments();
};
