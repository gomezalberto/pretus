#pragma once

#include <Manager.h>
#include <opencv2/opencv.hpp>
#include <memory>
#include <chrono>
#include <boost/circular_buffer.hpp>

class VideoManager : public Manager {
    Q_OBJECT

public:

    typedef VideoManager            Self;
    typedef std::shared_ptr<Self>       Pointer;

    /** Constructor */
    static Pointer New(QObject *parent = 0) {
        return Pointer(new VideoManager(parent));
    }

    int Initialize();

    int getFrameRate() const;
    void setFrameRate(int value);

    bool getLoopAround() const;
    void setLoopAround(bool value);

    QString getVideoFile() const;
    void setVideoFile(const QString &value);
    void SetStringTime(std::string timeString);

    double FF_factor;

public Q_SLOTS:

    /**
   * @brief Send the latest dnl::Image signal
   *
   * This method has to be public so that it can be binded to the periodic callback
   */
    virtual void Send(void);

    virtual void slot_frameValueChanged(int v);

Q_SIGNALS:

    void FileNumberChanged(int arg);

protected:
    VideoManager(QObject *parent = 0);

private:

    /**
     * @brief GetFrame
     * @return false if the re is no frame
     */
    bool GetFrame(void);
    ifind::Image::Pointer convertCVMatToIfindImageData(const cv::Mat &sourceCVImage);


    int FrameRate;
    boost::circular_buffer<double> TransmitFrameRate;
    bool LoopAround;
    QString VideoFile;
    cv::Mat Frame;
    std::mutex mutex_Frame;
    int initial_time_msec;
    std::chrono::steady_clock::time_point t_0;
    std::chrono::steady_clock::time_point last_transmit_t;


    cv::VideoCapture VideoSource;


};
