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
#include <itkVectorIndexSelectionCastImageFilter.h>
#include <algorithm>

PnPFrameGrabberManager::PnPFrameGrabberManager(QObject *parent) : Manager(parent){
    this->latestAcquisitionTime = std::chrono::steady_clock::now();
    this->initialAcquisitionTime = std::chrono::steady_clock::now();
    this->TransmitFrameRate.set_capacity(60);
    this->mIsPaused = false;

}

int PnPFrameGrabberManager::updateVideoSettings(void){

    if (! this->VideoSource.isOpened()){
        this->VideoSource.release();
    }

    this->VideoSource.open(this->params.cam_id,cv::CAP_V4L2);
    this->VideoSource.set(cv::CAP_PROP_BUFFERSIZE, mVideoSettings.buffersize);
    this->VideoSource.set(cv::CAP_PROP_FOURCC, mVideoSettings.fourcc);
    this->VideoSource.set(cv::CAP_PROP_FPS, mVideoSettings.framerate);
    this->VideoSource.set(cv::CAP_PROP_FRAME_WIDTH, mVideoSettings.w);
    this->VideoSource.set(cv::CAP_PROP_FRAME_HEIGHT, mVideoSettings.h);

    int real_framerate = this->VideoSource.get(cv::CAP_PROP_FPS);
    if (real_framerate != mVideoSettings.framerate){
            std::cout << "[Warning] PnPFrameGrabberManager::updateVideoSettings() - selected framerate ("<< mVideoSettings.framerate<<") is not supported by your device, using "<< real_framerate<<" fps"<< std::endl;
    }

    if (! this->VideoSource.isOpened()){
        std::cout << "[ERROR] PnPFrameGrabberManager::Initialize() - Could nt open video grabber on device "<< this->params.cam_id<<std::endl;
        return -1;
    }
    return 0;

}

int PnPFrameGrabberManager::Initialize(){

    // convert resolution string into values
    QStringList pieces = this->params.resolution.split( "." );
    mVideoSettings.w = pieces.value(0).toInt();
    mVideoSettings.h = pieces.value(1).toInt();
    mVideoSettings.framerate = this->params.CaptureFrameRate;

    this->updateVideoSettings();

    if (this->params.verbose){
        std::cout << "[VERBOSE] PnPFrameGrabberManager::Initialize() - Video settings:"<<std::endl;
        std::cout << this->mVideoSettings.toStdString()<<std::endl;
    }
    this->last_transmit_t = std::chrono::steady_clock::now();

    return 0;
}

void PnPFrameGrabberManager::slot_togglePlayPause(bool v){
    this->mIsPaused  =v;
}

void PnPFrameGrabberManager::slot_updateFrameRate(QString f){

    QStringList pieces =  f.split( " " );
    int framerate = pieces.value(0).toInt();
    mVideoSettings.framerate = framerate;
    this->updateVideoSettings();
    if (this->params.verbose){
        std::cout << "[VERBOSE] PnPFrameGrabberManager::slot_updateFrameRate() - Video settings:"<<std::endl;
        std::cout << this->mVideoSettings.toStdString()<<std::endl;
    }
}

void PnPFrameGrabberManager::slot_updateResolution(QString resolution){
    QStringList pieces0 = resolution.split( " " );

    QStringList pieces = pieces0.value(0).split( "x" );
    mVideoSettings.w = pieces.value(0).toInt();
    mVideoSettings.h = pieces.value(1).toInt();

    this->updateVideoSettings();
    if (this->params.verbose){
        std::cout << "[VERBOSE] PnPFrameGrabberManager::slot_updateResolution() - Video settings:"<<std::endl;
        std::cout << this->mVideoSettings.toStdString()<<std::endl;
    }
}

void PnPFrameGrabberManager::slot_updateEncoding(QString enc){
    if (enc.toLower() == "full"){
        mVideoSettings.fourcc = CV_FOURCC('I', '4', '2', '0');
    } else if  (enc.toLower() == "mjpeg"){
        mVideoSettings.fourcc = CV_FOURCC('M', 'J', 'P', 'G');
    } else if  (enc.toLower() == "h.264"){
        mVideoSettings.fourcc = CV_FOURCC('H', '2', '6', '4');
    }

    this->updateVideoSettings();
}

/**
 * @brief Gets a frame, applies the homography and cropping, and converts it to VTK
 */
void PnPFrameGrabberManager::Send(void){


    if (this->VideoSource.isOpened() && mIsPaused==false){
        this->mutex_Frame.lock();
        try {
            this->VideoSource >> this->Frame; // get a new frame from camera
        } catch (const cv::Exception& e) {
            std::cerr << "[Error] VideoManager::Send() -  reading frame from file. Reason: " << e.msg << std::endl;
            this->mutex_Frame.unlock();
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
        int w=this->Frame.cols, h = this->Frame.rows;
        /// check if the frame is ok
        if (this->Frame.rows == 0 || this->Frame.cols == 0){
            std::cerr << "[Error] PnPFrameGrabberManager::Send() -  frame is empty."<< std::endl;
            this->mutex_Frame.unlock();
            this->VideoSource.release();
            QApplication::quit();
        }

        if (this->Frame.rows != mVideoSettings.h || this->Frame.cols != mVideoSettings.w){
            std::cout << "[Warning] PnPFrameGrabberManager::Send() - selected resolution ("<< mVideoSettings.w<< "x"<< mVideoSettings.h<<") is not supported by your device, using "<< this->Frame.cols<< "x"<< this->Frame.rows<< std::endl;
            QString res = QString::number(this->Frame.cols) + "x" + QString::number(this->Frame.rows) + " (other)";
            this->slot_updateResolution(res);
        }


        /// copy the buffer
        const unsigned long numberOfPixels = this->Frame.cols *this->Frame.rows * 3;
       // ifind::Image::PixelType frame_data[numberOfPixels];
       // std::memcpy(&frame_data, this->Frame.data, sizeof(ifind::Image::PixelType)*numberOfPixels);

        /// Do the studio swing
        //const ifind::Image::PixelType *p_end = &frame_data[0]+numberOfPixels;
        const ifind::Image::PixelType *p_end = &this->Frame.data[0]+numberOfPixels;
        double factor0 = 1.0;
        double factor1 = 0.0;
        if (this->params.correct_studio_swing > 0) {
            factor0 = 255./(235.-this->params.correct_studio_swing);
            factor1 = this->params.correct_studio_swing;
        }
        for (ifind::Image::PixelType *p = &this->Frame.data[0]; p <  p_end; ++p){
            ifind::Image::PixelType newval = static_cast<ifind::Image::PixelType>(std::floor( (static_cast<double>(*p)-factor1)*factor0));
            *p = std::min(std::max(newval,ifind::Image::PixelType(0)), ifind::Image::PixelType(255));
        }


        this->mTransmitedFramesCount++;
        ifind::Image::Pointer image = ifind::Image::New();

        typedef itk::OpenCVImageBridge BridgeType;
        // Convert to colour image
        ifind::Image::ColorImageType::Pointer colourImage = ifind::Image::ColorImageType::New();
        colourImage = BridgeType::CVMatToITKImage<ifind::Image::ColorImageType>(this->Frame);

        // extract components
        using IndexSelectionType = itk::VectorIndexSelectionCastImageFilter<ifind::Image::ColorImageType, ifind::Image>;
        {             // First component (R) as base layer
            IndexSelectionType::Pointer indexSelectionFilter = IndexSelectionType::New();
            indexSelectionFilter->SetIndex(0);
            indexSelectionFilter->SetInput(colourImage);
            indexSelectionFilter->Update();
            image->Graft(indexSelectionFilter->GetOutput(), "R");

        }
        {             // G
            IndexSelectionType::Pointer indexSelectionFilter = IndexSelectionType::New();
            indexSelectionFilter->SetIndex(1);
            indexSelectionFilter->SetInput(colourImage);
            indexSelectionFilter->Update();
            image->GraftOverlay(indexSelectionFilter->GetOutput(), image->GetNumberOfLayers(), "G");
        }
        {             // B
            IndexSelectionType::Pointer indexSelectionFilter = IndexSelectionType::New();
            indexSelectionFilter->SetIndex(2);
            indexSelectionFilter->SetInput(colourImage);
            indexSelectionFilter->Update();
            image->GraftOverlay(indexSelectionFilter->GetOutput(), image->GetNumberOfLayers(), "B");
        }
        //image->Graft(BridgeType::CVMatToITKImage<ifind::Image>(this->Frame));
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
            auto current_transmit_t = std::chrono::steady_clock::now();
            int duration = std::chrono::duration_cast<std::chrono::milliseconds>(current_transmit_t - this->last_transmit_t).count();
            this->TransmitFrameRate.push_back(1000.0 / double(duration));
            double total_fr= 0;
            for (boost::circular_buffer<double>::const_iterator cit = this->TransmitFrameRate.begin(); cit != this->TransmitFrameRate.end(); ++cit){
                total_fr+= *cit;
            }
            auto current_fr = total_fr / this->TransmitFrameRate.size();
            image->SetMetaData<>("MeasuredFrameRate", QString::number(current_fr).toStdString());
            this->last_transmit_t = current_transmit_t;
            image->SetMetaData<>("Width", QString::number(w).toStdString());
            image->SetMetaData<>("Height", QString::number(h).toStdString());

            image->SetMetaData<bool>("IsColor", true);
        }
        image->SetSpacing(params.pixel_size);
        if (this->mTransmitedStreamType.size()>0){
            image->SetStreamType(this->mTransmitedStreamType);
        }
        Q_EMIT this->ImageGenerated(image);
    }
}

