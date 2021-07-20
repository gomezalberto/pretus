#ifndef ifindImagePeriodicTimer_h
#define ifindImagePeriodicTimer_h

#include <exports/ifind2_common_Export.h>

#include <QObject>
#include <QMutex>
#include <QTimer>

#include <ifindImage.h>

class QTimer;

/**
 * @class ifindImagePeriodicTimer
 * @author Nicolas Toussaint and Alberto Gomez
 * @brief Timer to control the rate at which a ifind::Image signal is sent

 * @details
 *
 * This class can be used to control the frequency at which a ifind::Image signal is passed over.
 * Set the frequency of the output signal using SetInterval(unsigned int interval) (in milliseconds)
 * Feed the timer regularly with the latest ifind::Image using SetIfindImage(ifind::Image::Pointer image)
 * 
 * Start the timer using Start(). This will launch a separate thread that will trigger the output signal 
 * ImageReceived(ifind::Image::Pointer image) at the desired frequency
 * 
 * Example code:
 \code{.cpp}
 
  #include "dnlImageHandler.h"
  #include "ifindImagePeriodicTimer.h"
  #include "dnlImageWidget.h"
  
  dnlImageHandler* imagehandler = new dnlImageHandler();
  ifindImagePeriodicTimer* timer = new ifindImagePeriodicTimer();
  dnlImageWidget* visualizer = new dnlImageWidget();
  
  QObject::connect(imagehandler, SIGNAL(DnlImageReceived(ifind::Image::Pointer)),
                   timer, SLOT(SetDnlImage(ifind::Image::Pointer)));
  QObject::connect(timer, SIGNAL(DnlImageReceived(ifind::Image::Pointer)),
                   visualizer, SLOT(DnlImageReceived(ifind::Image::Pointer)));
  
  timer->SetInterval(100);
  timer->Start();
  
 \endcode
 * 
 */
class iFIND2COMMON_EXPORT ifindImagePeriodicTimer : public QObject
{
  Q_OBJECT

public:

  ifindImagePeriodicTimer(QObject* parent = 0);
  ~ifindImagePeriodicTimer();

  /**
   * @brief Starts the periodic callback
   * 
   * This launches the periodic callback system.
   * 
   * @see iFindPeriodicCallback
   */
  void Start(unsigned int msec);

  /**
   * @brief Stops the periodic callback
   * 
   * This terminates the periodic callback system.
   * 
   * @see iFindPeriodicCallback
   */
  void Stop(void);

  /**
   * @brief returs true if the internal ifind::Image has changed in the last 3 seconds
   */
  bool IsActive(void);

  ifind::Image::Pointer GetIfindImage(void)
  { return this->Image; }

  unsigned int GetFrameRate(){ return this->FrameRate; }
  unsigned int GetInterval(){ return this->Interval; }

    bool GetDropFrames() const;
    void SetDropFrames(bool value);

    bool GetDebug() const;
    void SetDebug(bool value);

public Q_SLOTS:

  /**
   * @brief Set the next ifind::Image to send
   * 
   * @param image ifind::Image to be sent
   */
  void SetIfindImage(ifind::Image::Pointer image);
  /**
   * @brief Send the latest ifind::Image signal
   * 
   * This method has to be public so that it can be binded to the periodic callback
   */
   void Send(void);

    void ReadyToShootOn();

Q_SIGNALS:

  /**
   * @brief The signal emitted after reception of a DNLTopLevelObject and meta data population
   * 
   * The signal is emmited periodically using SetInterval(unsigned int interval) only if the image has changed
   * 
   * @param image the fully populated ifind::Image
   */
  void ImageReceived(ifind::Image::Pointer image);


protected:

  /**
   * @brief Update the frame rate of the image (indicated in the metadata) using
   * the last N images timestamps
   */
  unsigned int UpdateFrameRate(void);


private:

    ifind::Image::Pointer Image;
    QMutex Mutex;
    bool Sent;
    unsigned int Interval;
    long int LatestTimeStamp;

    std::shared_ptr<QTimer> Timer;

    std::vector<uint64_t> TimeStamps;

    unsigned int ImageCounter;
    unsigned int N_SendRequests;
    unsigned int FrameRate;
    bool readyToShoot;
    bool Debug;
    /**
   * @brief if set to true, frames will be dropped to keep up with
   * desired frame rate; if false, events will be queued.
   */
    bool DropFrames;
};

#endif
