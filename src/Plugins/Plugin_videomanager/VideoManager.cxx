#include "VideoManager.h"
#include <ifindImageReader.h>
#include <QTimer>
#include <QDebug>
#include <QElapsedTimer>
#include <itkOpenCVImageBridge.h>
//#include <opencv2/videoio.hpp>
#include <QApplication>
#include <QString>
#include <QStringList>


VideoManager::VideoManager(QObject *parent){
    this->FrameRate = 25;
    this->LoopAround = true;
    this->FrameCount = 0;
    this->VideoFile = "";
    this->initial_time_msec = 0;
    this->FF_factor = 1.10; // This seems to be needed to compensate for the time during the frame processing. With this value, the playback seems real time. Might not be the case in a faster pc though.
}

void VideoManager::SetStringTime(std::string timeString){
   int msec = 0;

   QString qtimeString(timeString.c_str());
   QStringList sl = qtimeString.split(":");
   int minutes = sl[0].toInt();
   int seconds = sl[1].toInt();
   msec = minutes*60*1000 + seconds*1000;

   this->initial_time_msec = msec;

}

int VideoManager::Initialize(){

    if (this->VideoFile.size() <=0){
        return -1;
    }

    std::cout << "VideoManager::Initialize() - loading video "<< this->VideoFile.toStdString()<<"...";

    this->VideoSource.open(this->VideoFile.toStdString());
    if(!this->VideoSource.isOpened()){  // check if we succeeded
        std::cout << " could not load video"<< std::endl;
        return -1;
    }

    this->FrameRate = this->VideoSource.get(CV_CAP_PROP_FPS);
    int nframes = this->VideoSource.get(CV_CAP_PROP_FRAME_COUNT);
    this->VideoSource.set(CV_CAP_PROP_POS_MSEC, this->initial_time_msec);
    this->t_0 = std::chrono::steady_clock::now();

    /// Also change the frame count
    this->FrameCount = int(double(this->initial_time_msec)/1000.0*this->FrameRate);

    std::cout << " loaded, FPS = "<< this->FrameRate<<", frames = "<<nframes<< std::endl;
    return 0;
}


void VideoManager::Send(void)
{

    // while (this->VideoSource.isOpened()){
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
        unsigned int wait = (unsigned int)(1000.0/(float)(this->FrameRate))/this->FF_factor;
        QTimer::singleShot(wait, this, SLOT(Send()));
    } else {
        QApplication::quit();
    }

    {
        if (this->Frame.rows == 0 || this->Frame.cols == 0){
            if (this->LoopAround != true){
                std::cerr << "[Error] VideoManager::Send() -  frame is empty. Finishing acquisition after "<< this->FrameCount/double(this->FrameRate) <<" seconds"<< std::endl;
                this->mutex_Frame.unlock();
                QApplication::quit();
            } else {
                /// The video has finished, let's start it again
                if (this->verbose){
                    std::cout << "[WARNING] VideoManager::Send() -  The video has finished, let's start agian from the beginning"<< std::endl;
                }
                //this->VideoSource.set(CV_CAP_PROP_POS_MSEC, this->initial_time_msec);
                this->VideoSource.set(CV_CAP_PROP_POS_MSEC, 0); // start from begining. COUld be start from the initial timestamp as above.
                this->mutex_Frame.unlock();
            }
            return;
        }
        this->FrameCount++;
        ifind::Image::Pointer image = ifind::Image::New();

        typedef itk::OpenCVImageBridge BridgeType;
        image->Graft(BridgeType::CVMatToITKImage<ifind::Image>(this->Frame));
        this->mutex_Frame.unlock();

        //image->Graft(this->convertCVMatToIfindImageData(this->Frame));
        /// Add basic meta data
        {

            //int milliseconds= std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::steady_clock::now()-this->t_0+std::chrono::milliseconds(this->initial_time_msec)).count();
            int milliseconds = this->VideoSource.get(CV_CAP_PROP_POS_MSEC);
            int hours = milliseconds/1000/3600;
            int minutes = (milliseconds/1000 -hours*3600)/60;
            float seconds = milliseconds/1000.0 -hours*3600 - minutes*60;

            QString timestring;
            timestring.sprintf("%02d:%02d:%02.3f", hours, minutes, seconds);
            if (this->verbose){
                std::cout << "[VERBOSE] VideoManager::Send() - Frame: "<< this->FrameCount << ", Play time: "<< timestring.toStdString()<<std::endl;
            }

            std::string timestamp = std::to_string(std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch()).count());
            image->SetMetaData<std::string>("DNLTimestamp", timestamp);
            image->SetMetaData<std::string>("VideoTime", timestring.toStdString());
            image->SetMetaData<std::string>("AcquisitionSystem", this->VideoFile.toStdString());
            image->SetMetaData<std::string>("NDimensions", "2");
            image->SetMetaData<std::string>("ImageMode", std::to_string(ifind::Image::ImageMode::External));
            image->SetMetaData<>("AcquisitionFrameRate", QString::number(0).toStdString());
            image->SetMetaData<>("TransmissionFrameRate", QString::number(0).toStdString());
            image->SetMetaData<>("FrameCount", QString::number(this->FrameCount).toStdString());
        }
        Q_EMIT this->ImageGenerated(image);
    }


    // this->VideoSource.release();

}


bool VideoManager::GetFrame(void){

    try {
        this->VideoSource >> this->Frame; // get a new frame from camera
    } catch (const cv::Exception& e) {
        std::cerr << "[Error] VideoManager::GetFrame() -  reading frame from file. Reason: " << e.msg << std::endl;
    }

    if (this->Frame.rows == 0 || this->Frame.cols == 0){
        return false;
    }
    return true;
}


QString VideoManager::getVideoFile() const
{
    return VideoFile;
}

void VideoManager::setVideoFile(const QString &value)
{
    this->VideoFile = value;
}

bool VideoManager::getLoopAround() const
{
    return LoopAround;
}

void VideoManager::setLoopAround(bool value)
{
    LoopAround = value;
}

int VideoManager::getFrameRate() const
{
    return FrameRate;
}

void VideoManager::setFrameRate(int value)
{
    FrameRate = value;
}



ifind::Image::Pointer VideoManager::convertCVMatToIfindImageData(const cv::Mat &sourceCVImage) {

    assert( sourceCVImage.data != NULL );

    typedef itk::OpenCVImageBridge BridgeType;
    ifind::Image::Pointer ifindImage = BridgeType::CVMatToITKImage<ifind::Image>(sourceCVImage);

    return ifindImage;

}
