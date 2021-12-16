#pragma once

#include <Manager.h>
#include <string>
#include <memory>
#include <ifindImage.h>
#include <vector>
#include <list>
#include <chrono>
#include <opencv2/opencv.hpp>


class PnPFrameGrabberManager : public Manager{
    Q_OBJECT

public:
    typedef PnPFrameGrabberManager            Self;
    typedef std::shared_ptr<Self>       Pointer;

    /** Constructor */
    static Pointer New(QObject *parent = 0) {
        return Pointer(new PnPFrameGrabberManager(parent));
    }

    int Initialize();

    struct Parameters {
        Parameters() {

            pixel_size[0] = 1;
            pixel_size[1] = 1;
            pixel_size[2] = 1;

            cam_id = 4;
            resolution = "1024.768"; // width.height
            //Resolution_factor = 1.0;
            CaptureFrameRate  = 30;
            n_components = 1;

            verbose = false;
            correct_studio_swing = 0;
        }

        double pixel_size[3];
        QString resolution;
        int cam_id;
        //float Resolution_factor;
        double CaptureFrameRate;
        bool verbose;
        int correct_studio_swing;
        /**
         * @brief number of components of raw data: 1 (gray) or 3 YUV
         */
        int n_components;
    };

    struct VideoSettings {
        VideoSettings(){
            buffersize = 1;
            framerate = 30;
            w = 1024;
            h = 768;
            fourcc = CV_FOURCC('M', 'J', 'P', 'G');
        }

        std::string toStdString(){
            std::stringstream ss;
            ss << "Video Settings:"<<std::endl;
            ss << "\tBuffer size: "<< buffersize<<std::endl;
            ss << "\tFrame rate: "<< framerate<<std::endl;
            ss << "\tResolution: "<< w <<"x"<<h<<std::endl;
            ss << "\tCodec: "<< fourcc<<std::endl;
            return ss.str();
        }
        int buffersize;
        int framerate;
        int w;
        int h;
        int fourcc;
    };

    Parameters params;


public Q_SLOTS:

    /**
   * @brief Send the latest dnl::Image signal
   *
   * This method has to be public so that it can be binded to the periodic callback
   */
    virtual void Send(void);

    virtual void slot_togglePlayPause(bool v);
    virtual void slot_updateFrameRate(QString f);
    virtual void slot_updateResolution(QString resolution);
    virtual void slot_updateEncoding(QString enc);

protected:
    PnPFrameGrabberManager(QObject *parent = 0);

private:

    cv::VideoCapture VideoSource;
    bool mIsPaused;
    std::mutex mutex_frame_buffer;
    std::mutex mutex_Frame;
    cv::Mat Frame;
    std::chrono::steady_clock::time_point latestAcquisitionTime;
    std::chrono::steady_clock::time_point initialAcquisitionTime;
    VideoSettings mVideoSettings;

    int updateVideoSettings(void);

};
