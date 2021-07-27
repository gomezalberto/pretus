#include "FrameGrabberManager.h"
#include <chrono>
#include <thread>
#include <itkImportImageFilter.h>
#include <itkResampleImageFilter.h>
#include <itkIdentityTransform.h>
#include <vtkMetaImageWriter.h>
#include <boost/filesystem.hpp>
#include <boost/progress.hpp>
#include <vtkJPEGWriter.h>
#include <QTimer>
#include <chrono>
#include <itkImageFileWriter.h>
#include <itkShiftScaleImageFilter.h>
#include <QApplication>

FrameGrabberManager::FrameGrabberManager(QObject *parent) : Manager(parent){
    this->Cap = nullptr;
    this->latestAcquisitionTime = std::chrono::steady_clock::now();
    this->initialAcquisitionTime = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    double elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->latestAcquisitionTime).count();
    double currentFrameRate = 1000.0/elapsed_ms;

}

int FrameGrabberManager::Initialize(){

    this->Cap = new gg::VideoSourceEpiphanSDK(this->params.Device_name.data(), V2U_GRABFRAME_FORMAT_I420);
    FrmGrab_SetMaxFps(this->Cap->get_frame_grabber(), 30);

    V2U_VideoMode vm;
    if (!FrmGrab_DetectVideoMode((FrmGrabber*)this->Cap->get_frame_grabber(), &vm))
    {
        std::cerr << "[FrameGrabberManager::Initialize] No signal detected"<<std::endl;
        return -1;
    }
    return 0;
}



/**
 * @brief Gets a frame, applies the homography and cropping, and converts it to VTK
 */
void FrameGrabberManager::Send(void){

    /// Go for next round and continue!
    if (this->IsActive())
    {

        //int wait = 0;
        unsigned int wait =0;
        if (this->params.CaptureFrameRate>0){
            this->params.CaptureFrameRate = (this->params.CaptureFrameRate > 0) && (this->params.CaptureFrameRate < 200)? this->params.CaptureFrameRate : 30;
            wait = (unsigned int)(1000.0/(float)(this->params.CaptureFrameRate));
        }
        QTimer::singleShot(wait, this, SLOT(Send()));

    } else {

        QApplication::quit();
    }

    auto YUV = this->getFrameAsIfindImageData();

    ///-----------------------------------
    ifind::Image::Pointer Y = YUV[0];
    if (Y != nullptr){

        //        if (params.verbose){
        //            for (int i=0; i<params.n_components; i++){
        //                typedef itk::ImageFileWriter<ifind::Image> WriterType;

        //                std::stringstream ss;
        //                ss << "/tmp/image_"<< i <<"_"<< this->params.framecount<<".mhd";
        //                std::cout << "[FrameGrabberManager::Send()] writing to file "<< ss.str()<<std::endl;
        //                /// write the image
        //                WriterType::Pointer writer = WriterType::New();
        //                writer->SetFileName(ss.str());
        //                writer->SetInput(YUV[i]);
        //                writer->Update();
        //            }
        //        }

        this->mTransmitedFramesCount++;
        if (params.verbose){
            std::cout << "FrameGrabberManager::Send() frame count "<< this->mTransmitedFramesCount<<std::endl;
        }

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

            Y->SetMetaData<std::string>("StreamTime", timestring.toStdString());

            std::string timestamp = std::to_string(std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch()).count());

            Y->SetMetaData<std::string>("DNLTimestamp", timestamp);
            Y->SetMetaData<std::string>("AcquisitionSystem", "FrameGrabber");
            Y->SetMetaData<std::string>("NDimensions", "2");
            Y->SetMetaData<std::string>("ImageMode", std::to_string(ifind::Image::ImageMode::External));
            Y->SetMetaData<>("AcquisitionFrameRate", QString::number(currentFrameRate).toStdString());
            Y->SetMetaData<>("TransmissionFrameRate", QString::number(this->params.CaptureFrameRate).toStdString());
            Y->SetMetaData<>("TransmitedFrameCount", QString::number(this->mTransmitedFramesCount).toStdString());
        }
        Y->SetSpacing(params.pixel_size);
        Q_EMIT this->ImageGenerated(Y);
    }
}


std::vector<ifind::Image::Pointer> FrameGrabberManager::getFrameAsIfindImageData(void ) {


    gg::ColourSpace colour = gg::ColourSpace::I420;
    gg::VideoFrame frame(colour);
    {
        try {

            this->Cap->get_frame(frame);

        } catch (...) {
            std::cerr << "[Error] FrameGrabberManager::GetFrame() -  reading frame from framegrabber. "  << std::endl;
        }
    }

    assert( frame.data() != NULL );

    if (frame.rows() == 0 || frame.cols() == 0){
        std::vector<ifind::Image::Pointer> YUV(params.n_components, nullptr);
        return YUV;
    }

    /// copy the buffer
    const unsigned long numberOfPixels = frame.cols() *frame.rows() *1.02; /// I do not know why I need to add osme extra pixels but otherwise it fills weird values at the end
    ifind::Image::PixelType frame_data[numberOfPixels];
    std::memcpy(&frame_data, frame.data(), sizeof(ifind::Image::PixelType)*numberOfPixels);

    /// Do the studio swing
    const ifind::Image::PixelType *p_end = &frame_data[0]+numberOfPixels;
    double factor0 = 1.0;
    double factor1 = 0.0;
    if (this->params.correct_studio_swing == true) {
        factor0 = 255./(235.-16.);
        factor1 = 16.0;
    }
    for (ifind::Image::PixelType *p = &frame_data[0]; p <  p_end; ++p){
        *p = static_cast<ifind::Image::PixelType>(std::floor( (static_cast<double>(*p)-factor1)*factor0));
    }

    constexpr unsigned int Dimension = ifind::Image::ImageDimension;
    std::vector<ifind::Image::Pointer> YUV(params.n_components);
    /// For an n-pixel I420 frame: Y×8×n U×2×n V×2×n (so here taking only the Y channel)

    /// common to all
    using ImportFilterType = itk::ImportImageFilter<ifind::Image::PixelType, Dimension>;
    const itk::SpacePrecisionType origin[Dimension] = { 0.0, 0.0, 0.0 };
    ImportFilterType::SizeType size;
    size[2] = 1 ; // size along Z (one slice)
    ImportFilterType::IndexType start;
    start.Fill(0);
    ImportFilterType::RegionType region;
    region.SetIndex(start);

    unsigned long buffer_idx = 0;

    /// Import Y
    ImportFilterType::Pointer importFilter = ImportFilterType::New();

    size[0] = frame.cols(); // size along X
    size[1] = frame.rows() ; // size along Y

    region.SetSize(size);
    importFilter->SetRegion(region);

    /// @todo: change these
    const itk::SpacePrecisionType spacing[Dimension] = { 1.0, 1.0, 1.0 };
    importFilter->SetOrigin(origin);
    importFilter->SetSpacing(spacing);

    const bool importImageFilterWillOwnTheBuffer = false;
    importFilter->SetImportPointer(&frame_data[0], numberOfPixels, importImageFilterWillOwnTheBuffer);
    //importFilter->SetImportPointer(&frame.data()[0], numberOfPixels, importImageFilterWillOwnTheBuffer);
    importFilter->Update();
    ifind::Image::Pointer Y = ifind::Image::New();
    Y->Graft(importFilter->GetOutput());
    Y->DisconnectPipeline();
    YUV[0] = Y;

    buffer_idx+=numberOfPixels;

    /// now get the U and V components
    if (params.n_components ==3) {
        // No longer ensuring integrity here - buffer not copied
        ImportFilterType::Pointer importFilter = ImportFilterType::New();

        size[0] = frame.cols()/2; // size along X
        size[1] = frame.rows()/2; // size along Y

        region.SetSize(size);

        importFilter->SetRegion(region);

        /// @todo: change these
        const itk::SpacePrecisionType spacing[Dimension] = { 1.0, 1.0, 1.0 };
        importFilter->SetOrigin(origin);
        importFilter->SetSpacing(spacing);

        const unsigned int numberOfPixels = size[0] * size[1];
        {
            ifind::Image::PixelType *localBuffer = &frame.data()[buffer_idx];

            const bool importImageFilterWillOwnTheBuffer = false;
            importFilter->SetImportPointer(localBuffer, numberOfPixels, importImageFilterWillOwnTheBuffer);
            importFilter->Update();
            YUV[1] = ifind::Image::New();
            YUV[1]->Graft(importFilter->GetOutput());
            buffer_idx+=numberOfPixels;
        }
        {
            ifind::Image::PixelType *localBuffer = &frame.data()[buffer_idx];

            const bool importImageFilterWillOwnTheBuffer = false;
            importFilter->SetImportPointer(localBuffer, numberOfPixels, importImageFilterWillOwnTheBuffer);
            importFilter->Update();
            YUV[2] = ifind::Image::New();
            YUV[2]->Graft(importFilter->GetOutput());
            buffer_idx+=numberOfPixels;
        }
    }
    return YUV;

}

