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
            resolution = "1024.786"; // width.height
            //Resolution_factor = 1.0;
            CaptureFrameRate  = 30;
            n_components = 1;

            verbose = false;
            correct_studio_swing = false;
        }

        double pixel_size[3];
        QString resolution;
        int cam_id;
        //float Resolution_factor;
        double CaptureFrameRate;
        bool verbose;
        bool correct_studio_swing;
        /**
         * @brief number of components of raw data: 1 (gray) or 3 YUV
         */
        int n_components;
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

};
