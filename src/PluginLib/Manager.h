#pragma once
#include <QObject>
#include <QStringList>
#include <QString>
#include "ifindImage.h"
#include <mutex>

class iFIND2COMMON_EXPORT Manager : public QObject {

    Q_OBJECT

public:

    /*
    typedef Manager            Self;
    typedef std::shared_ptr<Self>       Pointer;

    static Pointer New(QObject *parent = 0) {
        return Pointer(new Manager(parent));
    }
    */

    /**
     * @brief Access the list of image files
     * @return file list
     */
    QStringList GetDataBase(void) const
    { return this->DataBase; }

    /**
     * @brief Query the active state
     * @return state
     */
    bool IsActive(void) const
    { return this->Active; }

    virtual void SetCurrentId(unsigned long arg){
        this->mutex_currentId.lock();
        this->CurrentId = arg;
        this->mutex_currentId.unlock();
    }

    unsigned long GetCurrentId(void) const{
        return this->CurrentId;
    }

    /**
     * @brief a loop that managers can call to enable the user to terminate the work via the command line
     * @return
     */
    void EnableCommandLineExitLoop();

    bool verbose;

public Q_SLOTS:

    /**
     * @brief Control the connectivity and activity of the plugin
     *
     * These methods are called when pressing 'start' and 'stop'
     */
    virtual void Activate(void){
        this->mutex_Active.lock();
        this->Active = true;
        this->mutex_Active.unlock();
    }

    /**
     * @brief Control the connectivity and activity of the plugin
     *
     * These methods are called when pressing 'start' and 'stop'
     */
    virtual void Deactivate(void){
        this->mutex_Active.lock();
        this->Active = false;
        this->mutex_Active.unlock();
    }

    virtual void SetActive(bool b){
        this->mutex_Active.lock();
        this->Active = b;
        this->mutex_Active.unlock();
    }

    /**
   * @brief Control the connectivity and activity of the plugin
   *
   * These methods are called when pressing 'start' and 'stop'
   */
    virtual void SetActivate(bool arg);

    /**
     * @brief Send the latest dnl::Image signal
     *
     * This method has to be public so that it can be binded to the periodic callback
     */
    virtual void Send(void) = 0;

    virtual void StartAcquisition(void);

    /**
   * @brief Set the database from which to pick images
   * @param arg list of itk compatible image files
   *
   * @warning The list should be ordered in the same way the manifold is ordered.
   */
    virtual void SetDataBase(QStringList arg);


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
     * @brief Request termination
     */
    void Terminate(void);



protected:

    void FindFiles(const QString &path, const QString &extension);
    Manager(QObject* parent = 0);
    QStringList DataBase;



private:
    int exitLoop();
    std::mutex mutex_Active;
    bool Active;
    std::mutex mutex_currentId;
    unsigned long CurrentId;
};
