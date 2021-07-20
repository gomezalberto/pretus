#include "ifindImagePeriodicTimer.h"

#include <chrono>
#include <thread>
#include <QTimer>

/// Declare the ifind::Image::Pointer as metatype so that it can be sent via signal / slot.
Q_DECLARE_METATYPE(ifind::Image::Pointer)

ifindImagePeriodicTimer::ifindImagePeriodicTimer(QObject *parent)
    : QObject(parent)
{
    /// Register ifind::Image::Pointer so that it can be sent via signal / slot
    qRegisterMetaType< ifind::Image::Pointer >();

    this->Image = nullptr;
    this->Sent = false;
    this->LatestTimeStamp = -1;
    this->Timer = std::make_shared<QTimer>();

    this->TimeStamps.resize(20, 0);
    this->ImageCounter = 0;
    this->N_SendRequests = 0;
    this->FrameRate = 25;
    this->Interval = 40;
    this->readyToShoot = true;
    this->DropFrames = false; /// by default, queue events
    this->Debug = false;
}


ifindImagePeriodicTimer::~ifindImagePeriodicTimer()
{
    this->Stop();
}


void ifindImagePeriodicTimer::Start(unsigned int msec)
{
    this->Interval = msec;
    this->FrameRate = static_cast<unsigned int>(1000.0/static_cast<double>(msec));

    //if (this->DropFrames){
    //    this->Timer->singleShot(static_cast<int>(msec), this, SLOT(Send()));
    //} else {
    QObject::connect(this->Timer.get(), SIGNAL(timeout()), this, SLOT(Send()));
    this->Timer->start(static_cast<int>(msec));
    //}
}


void ifindImagePeriodicTimer::Stop()
{
    this->Timer->stop();
}


void ifindImagePeriodicTimer::SetIfindImage(ifind::Image::Pointer image)
{
    this->Mutex.lock();

    /// update the image pointer
    this->Image = image;

    QStringList layertimetags = QString(image->GetMetaData<std::string>("DNLLayerTimeTag").c_str()).split(" ");
    if (layertimetags.size())
    {
        /// insert the timestamp into the list
        uint64_t ts = std::stoull(layertimetags[0].toStdString());
        this->TimeStamps.insert(this->TimeStamps.begin(), ts);
        /// pop the last timestamp off the list
        this->TimeStamps.pop_back();
    }
    /// assign the current framerate to the image
    this->Image->SetMetaData<std::string>("FrameRate", std::to_string(this->FrameRate));

    /// increment the counter
    this->ImageCounter++;
    if (this->ImageCounter >= this->TimeStamps.size())
    {
        /// Update the framerate every N images
        this->UpdateFrameRate();
        this->ImageCounter = 0;
    }

    /// notify that this image hasn't been sent yet
    this->Sent = false;

    std::chrono::milliseconds local_timestamp =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
    this->LatestTimeStamp =  local_timestamp.count();

    this->Mutex.unlock();
}


unsigned int ifindImagePeriodicTimer::UpdateFrameRate(void)
{
    /// query the first and last timestamps in microiseconds
    uint64_t t1 = this->TimeStamps[this->TimeStamps.size() - 1];
    uint64_t t2 = this->TimeStamps[0];
    this->FrameRate = (unsigned int)(1000000.0 * (this->TimeStamps.size() - 1) / (float)(t2 - t1));
    return this->FrameRate;
}

bool ifindImagePeriodicTimer::GetDebug() const
{
    return Debug;
}

void ifindImagePeriodicTimer::SetDebug(bool value)
{
    Debug = value;
}

bool ifindImagePeriodicTimer::GetDropFrames() const
{
    return this->DropFrames;
}

void ifindImagePeriodicTimer::SetDropFrames(bool value)
{
    this->DropFrames = value;
}

void ifindImagePeriodicTimer::Send(void)
{
    this->N_SendRequests++;

    if (this->readyToShoot){

        /// check wether the image is present and wether it has been sent already
        this->Mutex.lock();
        if (this->Image == nullptr){
            this->Mutex.unlock();
            return;
        }
        bool sent = this->Sent;
        ifind::Image::Pointer image = ifind::Image::New();
        image->ShallowCopy(this->Image);
        //        ifind::Image::Pointer image = this->Image;
        this->Image = nullptr;
        this->Mutex.unlock();

        /// do nothing if there is no image or if it has been sent already
        if (image!=nullptr && !sent)
        {
            if (this->DropFrames){
                this->readyToShoot = false;
            }
            /// emit a signal with the latest image and update the sent flag
            //std::cout << "[ifindImagePeriodicTimer::Send] sending image from stream : "<< image->GetStreamType()<<std::endl;
            Q_EMIT ImageReceived(image);
            this->Mutex.lock();
            this->Sent = true;
            this->Mutex.unlock();
        }
    } else {
        if (this->Debug){
            std::cout << "[ifindImagePeriodicTimer::Send] processing request: "<< this->N_SendRequests<<" skipped"<<std::endl;
        }
    }

}

void ifindImagePeriodicTimer::ReadyToShootOn(){
    this->readyToShoot = true;
};

bool ifindImagePeriodicTimer::IsActive(void)
{
    if (!this->Image)
        return false;
    long int t_image = this->LatestTimeStamp;
    std::chrono::milliseconds local_timestamp =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
    long int t_local = local_timestamp.count();
    return t_local - t_image < 3000;
}
