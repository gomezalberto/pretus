#pragma once

#include <Plugin.h>
#include "Worker_CppAlgorithm.h"
#include "Widget_CppAlgorithm.h"
#include <QtVTKVisualization.h>

class Plugin_CppAlgorithm : public Plugin {
    Q_OBJECT

public:

    typedef Worker_CppAlgorithm WorkerType;
    typedef Widget_CppAlgorithm WidgetType;
    typedef QtVTKVisualization ImageWidgetType;

    Plugin_CppAlgorithm(QObject* parent = 0);

    QString GetPluginName(void){ return "Cpp Algorithm";}
    QString GetPluginDescription(void) {return "Sample plug-in that does a simple threshold based segmentation.";}

    void SetCommandLineArguments(int argc, char* argv[]);

    /**
    * Initialize the plug-in: take in the command line arguments,
    * and any configuration passed on by previous plug-ins. Send
    * configuration options to downstream plug-ins.
    */
    void Initialize(void);

protected:
    virtual void SetDefaultArguments();
};
