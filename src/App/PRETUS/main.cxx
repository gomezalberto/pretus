#include <QObject>
#include <QKeyEvent>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QSharedPointer>
#include <QtWidgets/QApplication>
#include <QCheckBox>
#include <QSettings>
#include <iostream>
#include <string.h>
#include <stdio.h>

// with c++11 boost
#include <boost/signals2.hpp>
#include <Plugin.h>
#include <QtPluginWidgetBase.h>

/// For plug-ins
#include <my_config.h>
#include <dlfcn.h>

#ifdef WIN32
const static QChar sPluginOrderSeparatingCharacter('*');
const static QString sPluginLibExtension("Plugin_*.dll");
#else
const static QChar sPluginOrderSeparatingCharacter('>');
const static QString sPluginLibExtension("LibPlugin_*.so");
#endif

typedef QList<std::shared_ptr<Plugin>> PluginQList;

PluginQList LoadPlugins(int argc, char* argv[], const QString &plugin_folder);
PluginQList ParsePipeline(const QString &pipeline, const PluginQList &plugin_list);

/**
 * @brief Basic usage, which needs no modification. Specifc arguments for the processor
 * are passed on and defined at the processor.
 */

void PrintUsage( char* argv[], const PluginQList &plugin_list){
    std::cout << argv[0] << " [-h]"<<std::endl;
    std::cout << "Optional arguments:"<<std::endl;
    std::cout <<"\t-h \tShow this help."<<std::endl;
    std::cout << std::endl;
    std::cout << "\t-pipeline <val> :"<<std::endl;
    std::cout << "\t\t-pipeline \"index>index...>index\" [-plugin_option value -plugin_option value ...]" << std::endl;
    std::cout << "\tor..." << std::endl;
    std::cout << "\t\t-pipeline \"plugin_name>plugin_name\" -plugin_option value -plugin_option value" << std::endl;
    std::cout << "\t\t\tWhere 'plugin_name' comparison is case and space insensitive" << std::endl;
    std::cout << std::endl;
    std::cout << "\tExamples of pipeline strings"<<std::endl;
    std::cout << "\t" << argv[0] << " -pipeline \"3>1>0\""<<std::endl;
    std::cout << "\t" << argv[0] << " -pipeline \"FileManager>GUI\" --filemanager_input \"some_path\"" << std::endl;

    int j=0;
    for (auto plugin : plugin_list)
    {
        std::cout << std::endl;
        std::cout <<"("<< j++ << ") Plugin Name: '" << plugin->GetPluginName().toStdString() << "'" << std::endl;
        plugin->Usage();
    }
    std::cout << "---------------------------" << std::endl;
}

int main (int argc, char* argv[])
{

    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts); /// Needed to open other QWindows in plug-ins
    QApplication application(argc, argv);


    QString plugin_folder = getPluginFolder();
    QSettings settings("King's College London", "PRETUS");
    if (QFile(settings.fileName()).exists() == false ) {
        // create the config file.
        settings.beginGroup("MainApp");
        settings.setValue("plugin_folder", plugin_folder);
        //settings.setValue("pos", pos());
        settings.endGroup();
        std::cout << "Configuration file created in "<< settings.fileName().toStdString() <<std::endl;
    } else {
         settings.beginGroup("MainApp");
         plugin_folder = settings.value("plugin_folder").toString();
         //settings.setValue("pos", pos());
         settings.endGroup();
         std::cout << "Reading configuration from "<< settings.fileName().toStdString() <<std::endl;
    }

    const QStringList arguments = QCoreApplication::arguments();

    PluginQList plugin_list = LoadPlugins(argc, argv, plugin_folder);

    const int i_pipeline_str = arguments.indexOf("-pipeline");

    if (arguments.empty() ||
            arguments.contains("-h", Qt::CaseInsensitive) ||
            i_pipeline_str < 0 ||
            i_pipeline_str > (arguments.length() - 2))
    {
        PrintUsage(argv, plugin_list);
        return 0;
    }

    const QString & pipeline = arguments[i_pipeline_str + 1];

    // -------------------------------------------------
    //  Initi plug-ins and Connect callbacks and signals
    // -------------------------------------------------
    PluginQList ordered_plugins = ParsePipeline(pipeline, plugin_list);
    if (ordered_plugins.empty())
    {
        std::cerr << "Plugin order invalid or misspelled plug-in name" << std::endl;
        return 0;
    }

    QList<QtPluginWidgetBase *> widgets;
    QList<QtPluginWidgetBase *> imageWidgets;

    int n_input_plugins = 0;

    QListIterator<std::shared_ptr<Plugin>> plugin_iter(ordered_plugins);
    while (plugin_iter.hasNext())
    {
        auto current_plugin = plugin_iter.next();

        // we need both this plugin and the next one to connect
        if (plugin_iter.hasNext())
        {
            auto next_plugin = plugin_iter.peekNext();
            std::cout << current_plugin->GetPluginName().toStdString() <<
                         " -> " << next_plugin->GetPluginName().toStdString() << std::endl;
            QObject::connect(
                        current_plugin.get(), SIGNAL(ConfigurationGenerated(ifind::Image::Pointer)),
                        next_plugin.get(), SLOT(slot_configurationReceived(ifind::Image::Pointer)));

            QObject::connect(
                        current_plugin.get(), SIGNAL(ImageGenerated(ifind::Image::Pointer)),
                        next_plugin.get(), SLOT(slot_imageReceived(ifind::Image::Pointer)),
                        Qt::DirectConnection);
        }

        if (current_plugin->IsInput() == true){
            if (n_input_plugins > 0){
                ifind::Image::StreamType new_transmitted_type = "Input" + QString::number(n_input_plugins).toStdString();
                current_plugin->setTransmittedStreamType(new_transmitted_type);
            }
            n_input_plugins++;
        }

        // if this plug-in has a widget, add it
        //if (current_plugin->GetWidget() != nullptr){
        widgets.append(current_plugin->GetWidget());
        //}

        // if this plugin has a renderer (to display images), add it.
        //if (current_plugin->GetImageWidget() != nullptr){
        imageWidgets.append(current_plugin->GetImageWidget());
        //}

    }

    /// Check if a plug-in takes widgets, and if so, pass the widget list to it.
    for (auto plugin : ordered_plugins) {
        if (plugin->IntegratesWidgets() == true){
            plugin->SetWidgets(widgets);
            plugin->SetImageWidgets(imageWidgets);
        }
    }

    for (auto plugin : ordered_plugins) {
        plugin->Initialize();
    }

    std::cout <<"Start acquisition"<<std::endl;
    for (auto plugin : ordered_plugins) {
        plugin->SetActivate(true);
    }

    application.exec();

    std::cout << "The application will try to finish properly"<<std::endl;
    return 0;
}


PluginQList LoadPlugins(int argc, char* argv[], const QString &plugin_folder)
{
    // Check if there are plug-ins available
    PluginQList plugin_list;
    QStringList pluginsdirlist(QString(plugin_folder).split(";"));

    int total_plugin_count = 0;

    for ( const auto& current_dir_name : pluginsdirlist  )
    {

        QDir pluginsdir(current_dir_name);


        QStringList plugin_paths;
        QDirIterator it(pluginsdir.absolutePath(),
                        QStringList(sPluginLibExtension),
                        QDir::Files, QDirIterator::Subdirectories);

        while (it.hasNext()){
            plugin_paths.append(it.next());
        }

        if (plugin_paths.count()>0){
            std::cout << "Loading plug-ins from "<< pluginsdir.absolutePath().toStdString() <<std::endl;
        }

        for (int idx=0; idx<plugin_paths.count(); idx++){

            std::cout << "\t"<< total_plugin_count << " [Plugin] loading " << plugin_paths.at(idx).toStdString() << "..." << std::flush;
            // load the plugin's from shared library file,
            // imspired from https://teknoman117.wordpress.com/2012/08/05/c-plugins-with-boostfunction-on-linux/
            void *handle = NULL;
            // open the library in RTLD_GLOBAL mode in order for the python symbols to be loaded at a later stage
            // as per: http://stackoverflow.com/questions/8302810/undefined-symbol-in-c-when-loading-a-python-shared-library
            if(!(handle = dlopen(plugin_paths.at(idx).toUtf8().constData(), RTLD_NOW | RTLD_GLOBAL)))
            {
                std::cerr << "A \t[Plugin] ERROR: "<< idx << " " << dlerror() <<std::endl;
                continue;
            }
            dlerror();

            typedef boost::function<Plugin* ()> PluginConstructorType;

            // get the pluginConstructor function
            PluginConstructorType construct = (Plugin* (*)(void)) dlsym(handle, "construct");

            char *error = NULL;
            if((error = dlerror()))
            {
                std::cerr << "B [Plugin] ERROR: " << dlerror();
                dlclose(handle);
                continue;
            }


            // construct a plugin
            std::shared_ptr<Plugin> plugin(construct());
            //plugin->SetSettings(this->Settings);
            plugin_list.append(plugin);

            std::cout << "\t" << plugin->GetPluginName().toStdString()<< "("<< total_plugin_count <<") loaded";
            plugin->SetCommandLineArguments(argc, argv);

            std::cout <<std::endl;
            total_plugin_count++;
        }
    }
    return plugin_list;
}

int FindPluginIndexByName(const QString& name, const PluginQList &plugin_list)
{
    for (int i = 0; i < plugin_list.size(); ++i)
    {
        auto stripped_plugin_name = plugin_list[i]->GetPluginName().remove(QChar(' '));
        if (0 == QString::compare(stripped_plugin_name, name, Qt::CaseInsensitive))
        {
            return i;
        }
    }

    return -1;
}

PluginQList ParsePipeline(const QString &pipeline, const PluginQList &plugin_list)
{
    QStringList pipeline_items = pipeline.split(sPluginOrderSeparatingCharacter);
    PluginQList ordered_plugins;

    for (auto pipeline_item : pipeline_items)
    {
        bool string_is_int = false;
        int i_plugin(pipeline_item.toInt(&string_is_int));

        if (!string_is_int)
        {
            i_plugin = FindPluginIndexByName(pipeline_item, plugin_list);
        }

        // check that this is a valid plug-in
        // if not clear the list and return
        if (i_plugin < 0 ||
                i_plugin >= plugin_list.size())
        {
            return PluginQList();
        }

        ordered_plugins.append(plugin_list[i_plugin]);
    }

    return ordered_plugins;
}

