#pragma once

#include <Manager.h>
#include <string>
#include <memory>
#include <ifindImage.h>
#include <vector>
#include <list>
#include <videoframe.h>
#include <epiphansdk_video_source.h>
#include <chrono>
#include <boost/circular_buffer.hpp>

// define some custom EDIDs
class FrameGrabberManager : public Manager{
    Q_OBJECT

public:
    typedef FrameGrabberManager            Self;
    typedef std::shared_ptr<Self>       Pointer;

    /** Constructor */
    static Pointer New(QObject *parent = 0) {
        return Pointer(new FrameGrabberManager(parent));
    }

    int Initialize();

    struct Parameters {
        Parameters() {

            pixel_size[0] = 1;
            pixel_size[1] = 1;
            pixel_size[2] = 1;

            Device_name = "DVI2USB 3.0 ET";
            //Resolution_factor = 1.0;
            CaptureFrameRate  = 0; /// this will be as fast as can by default
            n_components = 3;

            verbose = false;
            correct_studio_swing = true;
        }

        double pixel_size[3];
        std::string Device_name;
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
    FrameGrabberManager(QObject *parent = 0);

private:
    int FrameRate;
    boost::circular_buffer<double> TransmitFrameRate;
    std::chrono::steady_clock::time_point last_transmit_t;

    gg::VideoSourceEpiphanSDK *Cap;
    bool mIsPaused;
    std::mutex mutex_frame_buffer;
    std::mutex mutex_Frame;
    gg::VideoFrame *Frame;

    /**
     * @brief GetFrame
     * @return nullptr if the re is no frame
     */
    std::vector<ifind::Image::Pointer> getFrameAsIfindImageData(void);

    ifind::Image::Pointer YUV2RGB(std::vector<ifind::Image::Pointer> &YUV );

    std::chrono::steady_clock::time_point latestAcquisitionTime;
    std::chrono::steady_clock::time_point initialAcquisitionTime;

    ifind::Image::Pointer Upsample(ifind::Image::Pointer in);

};
