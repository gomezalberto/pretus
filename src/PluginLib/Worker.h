#pragma once

#ifdef WIN32
#include <exports/PluginExport.h>
#else
#define PLUGIN_EXPORT
#endif // WIN32

#include <QObject>
#include <ifindImage.h>
#include <QThread>
#include <mutex>
#include <memory>
class QSettings;
class ifindImagePeriodicTimer;

/**
 * @class Worker
 * @author Alberto Gomez
 * @brief A template class derived from QObject for Plugin's worker architecture
 *
 * @details
 *
 * The class provides a template for the creation of a Worker that would eventually
 * carry out the functionality of a plugin
 * The class is pure virtual and the following methods must be overwritten:
 *
 * - virtual void slot_Work(ifind::Image::Pointer image) : slot that is triggered when a new ifind::Image::Pointer is received
 *
 * The class provides other default functions that may be overwritten for specific purposes:
 *
 * -  AddLatestOutputToImage() : modify output after processing is done
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
class PLUGIN_EXPORT Worker : public QObject
{
    Q_OBJECT

public:

    typedef Worker            Self;
    typedef std::shared_ptr<Self>       Pointer;


    typedef itk::Image<unsigned char, 3> GrayImageType;
    typedef itk::Image<unsigned char, 2> GrayImageType2D;

    struct WorkerParameters
    {
        enum OriginPolicy : int {Centre=0, Top=1};
        WorkerParameters() {
            out_spacing[0]=0.5;
            out_spacing[1]=0.5;
            out_spacing[2]=1.0;

            out_size[0]=256;
            out_size[1]=256;
            out_size[2]=1;

            origin = OriginPolicy::Top;
            verbose = false;
            measureTime = false;
            inputLayer = 0;
        }

        double out_spacing[3];
        int out_size[3];
        OriginPolicy origin;
        bool verbose;
        bool measureTime;
        int inputLayer;
    };

    WorkerParameters params;

    Worker(QObject* parent = 0);
    ~Worker();

    /**
   * @brief Initialize the Worker. Not all plug-ins need to re-implement this.
   */
    virtual void Initialize(void){};

    bool getPythonInitialized() const;
    void setPythonInitialized(bool value);


    static void set_gil_init(int n);

    QString pluginName() const;
    void setPluginName(const QString &pluginName);

public Q_SLOTS:

    /**
   * @brief Pure virtual slot for reception of a new ifind::Image
   * do not modify
   * @param image
   */
    virtual void slot_Work(ifind::Image::Pointer image);

Q_SIGNALS:

   /**
   * To tell the plugin (or someone else) that an image has been processed
   **/
    void ImageProcessed(ifind::Image::Pointer image);

    /**
     * @brief To notify that the thread has finished
     */
    void WorkFinished();

    /**
     * @brief The signal can be emitted any time and will propagate through the
     * pipeline
     * @param image (blank) with configuration parameters as meta data
     */
    void ConfigurationGenerated(ifind::Image::Pointer image);


protected:

    /**
     * @brief needed for the python plug-ins only
     */
    static int gil_init;

    /**
       * @brief needed for the python plug-ins only
       */
    bool PythonInitialized ;

    QString mPluginName;

    /**
     * @brief counts the number of processed images; only used in verbose mode
     */
    int FrameCount;
    std::mutex mutex_FrameCount;

    ifind::Image::Pointer AdjustImageSize(ifind::Image::Pointer image);
    GrayImageType::Pointer UnAdjustImageSize(GrayImageType::Pointer image, ifind::Image::Pointer reference);
    GrayImageType2D::Pointer crop_ifind_2D_image_data(GrayImageType2D::Pointer image);
    GrayImageType2D::Pointer get2dimage(ifind::Image::Pointer image);
    GrayImageType::Pointer get3dimagefrom2d(GrayImageType2D::Pointer image);
    GrayImageType2D::Pointer get2dimagefrom3d(ifind::Image::Pointer image);

    ifind::Image::SizeType sizeCrop;
    ifind::Image::SizeType cropLeft, cropRight;
    ifind::Image::SizeType extendLeft, extendRight;
    GrayImageType::SizeType lb,ub, lbi, ubi;

    /**
     * @brief This is the function that actually does the work
     * @param image
     */
    virtual void doWork(ifind::Image::Pointer image) = 0;
};
