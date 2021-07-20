#pragma once

#include "Plugin.h"

class QtVisualizationMainWindow;

class Plugin_GUI : public Plugin {
    Q_OBJECT

public:
    Plugin_GUI(QObject* parent = 0);

    QString GetPluginName(void){ return "GUI";}
    QString GetPluginDescription(void) {return "Displays imaging and non-imaging data in real time.";}

    void SetCommandLineArguments(int argc, char* argv[]);

    void Initialize(void);

    virtual void SetActivate(bool arg);

    virtual bool IntegratesWidgets();
    virtual void SetWidgets(QList<QtPluginWidgetBase *> &widgets);
    virtual void SetImageWidgets(QList<QtPluginWidgetBase *> &widgets);

public Q_SLOTS:
    virtual void slot_configurationReceived(ifind::Image::Pointer image);

protected:
    virtual void SetDefaultArguments();
private:
    std::shared_ptr<QtVisualizationMainWindow> mVisualizer;

    // need to keep a copy of the 'args' as the windows aren't initialised
    std::vector<std::string> mArgs;

    // These are the widgets that other plug-ins provide.
    // Location is defined in the plugin class
    QList<QtPluginWidgetBase *> mWidgets;
    QList<QtPluginWidgetBase *> mImageWidgets;
};
