#pragma once

#ifdef WIN32
#include <exports/PluginExport.h>
#else
#define PLUGIN_EXPORT
#endif // WIN32

#include <QObject>
#include <QThread>

#include <ifindImage.h>
#include <ifindStreamTypeHelper.h>

#include "Worker.h"
#include "QtPluginWidgetBase.h"
#include <vector>
#include <InputParser.h>
class QSettings;
class ifindImagePeriodicTimer;


/**
 * @class Plugin
 * @author Nicolas Toussaint and Alberto Gomez
 * @brief A template class derived from QObject for filter architecture
 *
 * @details
 *
 * The class provides a template for the creation of a plugin that would eventually be loaded diynamically by the  application.
 * The class is pure virtual and the following methods should be overwritten:
 *
 * - GetPluginName() : to provide a unique name to the plugin (can contain spaces)
 *
 * - ImageReceived() : slot that is triggered when a new ifind::Image::Pointer is received
 *
 * This class features a signal ImageGenerated(ifind::Image::Pointer) when a new image is available to be sent.
 *
 * The user needs to implement the external C constructor at the end of your cpp/cxx file
 *
 \begincode

extern "C"
{
  /// Function to return an instance of a new MyFilter object
  dnlImageFilter* construct()
  {
    return new MyFilter();
  }
}

 \endcode
 *
 * @see dnl::Image dnlApplication
 */
class PLUGIN_EXPORT Plugin : public QObject
{
  Q_OBJECT

public:

  static const char * const ArgumentType[];

  Plugin(QObject* parent = 0);
  ~Plugin(){};

  /**
   * @brief Pure virtual method to provide the filter's name
   */
  virtual QString GetPluginName(void) = 0;

  /**
   * @brief Pure virtual method to provide plug-ins description
   * @return
   */
  virtual QString GetPluginDescription(void) = 0;

  QString GetCompactPluginName(void) {
      return this->GetPluginName().simplified().remove(' ');
  }

  /**
   * @brief to be re-implemented by the filters that need to take arguments
   * @param argc
   * @param argv
   */
  virtual void SetCommandLineArguments(int argc, char* argv[]);

  /**
   * @brief To be re-implemented by plug-ins which have a specific usage (normally take command line arguments)
   */
  virtual void Usage(void);

  /**
   * @brief Returns widget for this plug-in.
   * This will have the basic ui for the plug-in, and will be integrated
   * by the main visualization plugin
   * @return
   */
  virtual QtPluginWidgetBase *GetWidget();

  /**
   * @brief Returns a visualization panel to show an image
   * @return
   */
  virtual QtPluginWidgetBase *GetImageWidget();

  virtual bool IntegratesWidgets();

  virtual void SetWidgets(QList<QtPluginWidgetBase *> &widgets){};

  virtual void SetImageWidgets(QList<QtPluginWidgetBase *> &widgets){};

  virtual void SetInputStream(ifind::StreamTypeSet &stream);

  /**
  @brief [deprecated] used when the filters are activated
  The Activate() method is called as construction.
  */
  virtual void Activate(void){
      this->SetActivate(true);
  };
  virtual void Deactivate(void){
      this->SetActivate(false);
  };


  /**
   * @brief Initialize the plugin. Not all plug-=oins need to re-implement this.
   */
  virtual void Initialize(void);

  virtual bool IsActive() const;

  virtual void SetActivate(bool arg){};

  double getFrameRate() const;
  void setFrameRate(double value);

  unsigned int getTimerInterval() const;
  void setTimerInterval(unsigned int value);

public Q_SLOTS:

  /**
   * @brief virtual slot for reception of a new dnl::Image
   *
   * @param image
   */
  virtual void slot_imageReceived(ifind::Image::Pointer image);

  /**
   * @brief slot to receive an image that has been processed by the worker and needs to be passed on to the next plug-in
   * @param image
   */
  virtual void slot_imageProcessed(ifind::Image::Pointer image);

  virtual void slot_configurationReceived(ifind::Image::Pointer image);
  /**
   * @brief This slot can be used if a source other than the plug-in (e.g. the
   * worker) wants to update some information on the fly
   * @param image
   */
  virtual void slot_passConfiguration(ifind::Image::Pointer config);

  /**
   * @brief slot_updateGUI
   * generates a config image to tell the GUI to update.
   */
  virtual void slot_updateGUI();

  void RemoveArgument(const QString &name);

Q_SIGNALS:

    /**
     * @brief The signal emitted after reception of a DNLTopLevelObject and meta data population
     *
     * The signal is emmited periodically using SetInterval(unsigned int interval) only if the image has changed
     *
     * @param image the fully populated dnl::Image
     */
    void ImageGenerated(ifind::Image::Pointer image);

    /**
     * @brief signal to send an image to the worker without the timer
     * @param image
     */
    void SendImageToWorker(ifind::Image::Pointer image);

    /**
     * @brief The signal emitted once during initialization
     * @param image (blank) with configuration parameters as meta data
     */
    void ConfigurationGenerated(ifind::Image::Pointer image);

    /**
     * @brief Convenience signal to request a worker to do a job
     * @param image
     */
    void Requestprocessing(ifind::Image::Pointer image);

protected:

    virtual void SetDefaultArguments();
    /**
     * @brief required to run in the background, if required
     */
    QThread workerThread;

    ifind::StreamTypeSet mStreamTypes;

    bool Active;

    std::vector< QStringList > mArguments;
    std::vector< QStringList > mGenericArguments;

    /**
     * @brief FrameRate and time interval are alternative representations of the same thing. Choose only one of the two
     */
    double FrameRate;

    /**
     * @brief FrameRate and time interval are alternative representations of the same thing. Choose only one of the two
     */
    unsigned int TimerInterval;

    ifindImagePeriodicTimer* Timer;
    Worker::Pointer worker;
    QtPluginWidgetBase *mWidget;

    /**
     * @brief basic image renderer. This can be overwriten or expanded by plug-ins
     */
    QtPluginWidgetBase *mImageWidget;

    /**
     * @brief true if this plugin works at a certain frame rate, false if
     * it works as quickly as a new image arrives.
     */
    bool isTimed;

    bool mVerbose;

private:

    void printIndented(std::stringstream &argname, std::stringstream &argdescription, unsigned int max_line_length);



};
