#include "PnPFrameGrabberManager.h"
#include <itkOpenCVImageBridge.h>
#include <chrono>
#include <thread>
#include <boost/filesystem.hpp>
#include <boost/progress.hpp>
#include <QTimer>
#include <chrono>
#include <itkShiftScaleImageFilter.h>
#include <QApplication>

PnPFrameGrabberManager::PnPFrameGrabberManager(QObject *parent) : Manager(parent){
    this->latestAcquisitionTime = std::chrono::steady_clock::now();
    this->initialAcquisitionTime = std::chrono::steady_clock::now();
}

int PnPFrameGrabberManager::Initialize(){


    this->VideoSource.open(this->params.cam_id,cv::CAP_V4L2);
    this->VideoSource.set(cv::CAP_PROP_BUFFERSIZE, 1);
    this->VideoSource.set(cv::CAP_PROP_FOURCC, CV_FOURCC('M', 'J', 'P', 'G'));
    this->VideoSource.set(cv::CAP_PROP_FPS, this->params.CaptureFrameRate);
    // convert resolution string into values
    int w, h;
    QStringList pieces = this->params.resolution.split( "." );
    w = pieces.value(0).toInt();
    h = pieces.value(1).toInt();
    this->VideoSource.set(cv::CAP_PROP_FRAME_WIDTH, w);
    this->VideoSource.set(cv::CAP_PROP_FRAME_HEIGHT, h);


    if (! this->VideoSource.isOpened()){
        std::cout << "[ERROR] PnPFrameGrabberManager::Initialize() - Could nt open video grabber on device "<< this->params.cam_id<<std::endl;
        return -1;
    }

    if (this->params.verbose){
        std::cout << "[VERBOSE] PnPFrameGrabberManager::Initialize() - Video settings:"<<std::endl;
        //        print('fps: {}'.format(fps))
        //        print('resolution: {}x{}'.format(w,h)) # default 640 x 480
        //        print('mode: {}'.format(decode_fourcc(fcc))) # default 640 x 480
        //        print('Buffer size: {}'.format(bs)) # default 640 x 480
    }


    return 0;
}



/**
 * @brief Gets a frame, applies the homography and cropping, and converts it to VTK
 */
void PnPFrameGrabberManager::Send(void){


    if (this->VideoSource.isOpened()){
        this->mutex_Frame.lock();
        try {
            this->VideoSource >> this->Frame; // get a new frame from camera
        } catch (const cv::Exception& e) {
            std::cerr << "[Error] VideoManager::Send() -  reading frame from file. Reason: " << e.msg << std::endl;
            return;
        }
    }


    if (this->IsActive())
    {

        unsigned int wait =0;
        /*
        if (this->params.CaptureFrameRate>0){
            this->params.CaptureFrameRate = (this->params.CaptureFrameRate > 0) && (this->params.CaptureFrameRate < 200)? this->params.CaptureFrameRate : 30;
            wait = (unsigned int)(1000.0/(float)(this->params.CaptureFrameRate));
        }
        */
        QTimer::singleShot(wait, this, SLOT(Send()));

    } else {

        QApplication::quit();
    }

    {
        /// check if the frame is ok
        if (this->Frame.rows == 0 || this->Frame.cols == 0){
            std::cerr << "[Error] PnPFrameGrabberManager::Send() -  frame is empty."<< std::endl;
            this->mutex_Frame.unlock();
            this->VideoSource.release();
            QApplication::quit();
        }


        this->mTransmitedFramesCount++;
        ifind::Image::Pointer image = ifind::Image::New();

        typedef itk::OpenCVImageBridge BridgeType;
        image->Graft(BridgeType::CVMatToITKImage<ifind::Image>(this->Frame));
        this->mutex_Frame.unlock();

        /// Add basic meta data
        {
            std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
            double elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->latestAcquisitionTime).count();
            double currentFrameRate = 1000.0/elapsed_ms;
            this->latestAcquisitionTime = now;
            if (currentFrameRate < 0.1){
                currentFrameRate = this->params.CaptureFrameRate;
            }

            // measure time from beginnning
            double milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->initialAcquisitionTime).count();
            int hours = milliseconds/1000/3600;
            int minutes = (milliseconds/1000 -hours*3600)/60;
            float seconds = milliseconds/1000.0 -hours*3600 - minutes*60;
            QString timestring;
            timestring.sprintf("%02d:%02d:%02.3f", hours, minutes, seconds);

            image->SetMetaData<std::string>("StreamTime", timestring.toStdString());

            std::string timestamp = std::to_string(std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch()).count());

            image->SetMetaData<std::string>("DNLTimestamp", timestamp);
            image->SetMetaData<std::string>("AcquisitionSystem", "FrameGrabber");
            image->SetMetaData<std::string>("NDimensions", "2");
            image->SetMetaData<std::string>("ImageMode", std::to_string(ifind::Image::ImageMode::External));
            image->SetMetaData<>("AcquisitionFrameRate", QString::number(currentFrameRate).toStdString());
            image->SetMetaData<>("TransmissionFrameRate", QString::number(this->params.CaptureFrameRate).toStdString());
            image->SetMetaData<>("TransmitedFrameCount", QString::number(this->mTransmitedFramesCount).toStdString());
        }
        image->SetSpacing(params.pixel_size);
        if (this->mTransmitedStreamType.size()>0){
            image->SetStreamType(this->mTransmitedStreamType);
        }
        Q_EMIT this->ImageGenerated(image);
    }
}

