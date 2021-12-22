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
#include <itkImageDuplicator.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkResampleImageFilter.h>

FrameGrabberManager::FrameGrabberManager(QObject *parent) : Manager(parent){
    this->Cap = nullptr;
    this->latestAcquisitionTime = std::chrono::steady_clock::now();
    this->initialAcquisitionTime = std::chrono::steady_clock::now();
    this->TransmitFrameRate.set_capacity(60);
    this->mIsPaused = false;
    gg::ColourSpace colour = gg::ColourSpace::I420;
    gg::VideoFrame frame(colour);
    this->Frame = new gg::VideoFrame(frame);
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


void FrameGrabberManager::slot_togglePlayPause(bool v){
    this->mIsPaused  =v;
}

/**
 * @brief Gets a frame, applies the homography and cropping, and converts it to VTK
 */
void FrameGrabberManager::Send(void){

    /// Go for next round and continue!
    if (this->IsActive() )
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

    auto image = this->getFrameAsIfindImageData(); // as RGB

    ///-----------------------------------

    if (image != nullptr){




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
            image->SetMetaData<bool>("IsColor", true);
        }
        image->SetSpacing(params.pixel_size);
        if (this->mTransmitedStreamType.size()>0){
            image->SetStreamType(this->mTransmitedStreamType);
        }
        Q_EMIT this->ImageGenerated(image);
    }
}



ifind::Image::Pointer FrameGrabberManager::getFrameAsIfindImageData(void ) {


    if (!this->mIsPaused){
        gg::ColourSpace colour = gg::ColourSpace::I420;
        gg::VideoFrame frame(colour);

        {
            try {

                this->Cap->get_frame(frame);

            } catch (...) {
                std::cerr << "[Error] FrameGrabberManager::GetFrame() -  reading frame from framegrabber. "  << std::endl;
            }
        }
        this->mutex_Frame.lock();
        this->Frame = &frame;
        this->mutex_Frame.unlock();
    }

    assert( frame.data() != NULL );

    if (this->Frame->rows() == 0 || this->Frame->cols() == 0){
        return nullptr;
    }

    /// copy the buffers
    const unsigned long numberOfPixels = this->Frame->cols() *this->Frame->rows();
    const unsigned long numberOfPixelsUV = this->Frame->cols()/ 2.0 * this->Frame->rows() / 2.0 ;

    ifind::Image::PixelType Y_channel[numberOfPixels], U_channel[numberOfPixelsUV], V_channel[numberOfPixelsUV];
    ifind::Image::PixelType R_channel[numberOfPixels], G_channel[numberOfPixels], B_channel[numberOfPixels];
    std::memcpy(&Y_channel, this->Frame->data(), sizeof(ifind::Image::PixelType)*numberOfPixels);
    std::memcpy(&U_channel, &this->Frame->data()[numberOfPixels], sizeof(ifind::Image::PixelType)*numberOfPixelsUV);
    std::memcpy(&V_channel, &this->Frame->data()[numberOfPixels+numberOfPixelsUV], sizeof(ifind::Image::PixelType)*numberOfPixelsUV);

    /// convert to RGB
    ifind::Image::PixelType *y = &Y_channel[0], *u = &U_channel[0], *v = &V_channel[0];
    ifind::Image::PixelType *r = &R_channel[0], *g = &G_channel[0], *b = &B_channel[0];
    const ifind::Image::PixelType *y_end = &Y_channel[0]+numberOfPixels;
    unsigned int npixel = 0, npixel_ = 0;
    unsigned int i, j, i_, j_; // indices from the large image

    for (; y <  y_end; ++y, ++r, ++g, ++b){
        // do NN interpolation for u and v
        i = npixel / this->Frame->rows();
        j = npixel - i * this->Frame->rows();
        i_ = i / 2;
        j_ = j / 2;
        npixel_ = j_ + this->Frame->rows()/2;
        //std::cout << "i, j "<< i << ", "<< j<<std::endl;
        auto u_= u[npixel_];
        auto v_= v[npixel_];
        *r = *y +0*u_ + 1.14*v_;
        *g = *y -0.396*u_ + -0.581*v_;
        *b = *y + 2.029*u_ + 0*v_;
    }


    /// Do the studio swing

    double factor0 = 1.0;
    double factor1 = 0.0;
    if (this->params.correct_studio_swing == true) {
        factor0 = 255./(235.-16.);
        factor1 = 16.0;
    }
    const ifind::Image::PixelType *r_end = &R_channel[0]+numberOfPixels;
    r = &R_channel[0], g = &G_channel[0], b = &B_channel[0];
    for (; r <  r_end; ++r){
        *r = static_cast<ifind::Image::PixelType>(std::floor( (static_cast<double>(*r)-factor1)*factor0));
        *g = static_cast<ifind::Image::PixelType>(std::floor( (static_cast<double>(*g)-factor1)*factor0));
        *b = static_cast<ifind::Image::PixelType>(std::floor( (static_cast<double>(*b)-factor1)*factor0));
    }

    constexpr unsigned int Dimension = ifind::Image::ImageDimension;

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

    size[0] = this->Frame->cols(); // size along X
    size[1] = this->Frame->rows() ; // size along Y
    const bool importImageFilterWillOwnTheBuffer = false;
    ifind::Image::Pointer RGB;
    /// Import R
    {
        ImportFilterType::Pointer importFilter = ImportFilterType::New();
        region.SetSize(size);
        importFilter->SetRegion(region);

        /// @todo: change these
        const itk::SpacePrecisionType spacing[Dimension] = { 1.0, 1.0, 1.0 };
        importFilter->SetOrigin(origin);
        importFilter->SetSpacing(spacing);

        importFilter->SetImportPointer(&R_channel[0], numberOfPixels, importImageFilterWillOwnTheBuffer);
        //importFilter->SetImportPointer(&frame.data()[0], numberOfPixels, importImageFilterWillOwnTheBuffer);
        importFilter->Update();
        RGB = ifind::Image::New();
        RGB->Graft(importFilter->GetOutput(), "R");
        RGB->DisconnectPipeline();
    }
    {
        // No longer ensuring integrity here - buffer not copied
        ImportFilterType::Pointer importFilter = ImportFilterType::New();
        region.SetSize(size);
        importFilter->SetRegion(region);

        /// @todo: change these
        const itk::SpacePrecisionType spacing[Dimension] = { 1.0, 1.0, 1.0 };
        importFilter->SetOrigin(origin);
        importFilter->SetSpacing(spacing);

        importFilter->SetImportPointer(&G_channel[0], numberOfPixels, importImageFilterWillOwnTheBuffer);
        importFilter->Update();
        RGB->GraftOverlay(importFilter->GetOutput(), RGB->GetNumberOfLayers(), "G");
        RGB->DisconnectPipeline();
    }
    {
        // No longer ensuring integrity here - buffer not copied
        ImportFilterType::Pointer importFilter = ImportFilterType::New();
        region.SetSize(size);
        importFilter->SetRegion(region);

        /// @todo: change these
        const itk::SpacePrecisionType spacing[Dimension] = { 1.0, 1.0, 1.0 };
        importFilter->SetOrigin(origin);
        importFilter->SetSpacing(spacing);

        importFilter->SetImportPointer(&B_channel[0], numberOfPixels, importImageFilterWillOwnTheBuffer);
        importFilter->Update();
        RGB->GraftOverlay(importFilter->GetOutput(), RGB->GetNumberOfLayers(), "B");
        RGB->DisconnectPipeline();
    }

    return RGB;
}

ifind::Image::Pointer FrameGrabberManager::Upsample(ifind::Image::Pointer in){

    ifind::Image::SizeType size = in->GetLargestPossibleRegion().GetSize();

    using T_Transform = itk::IdentityTransform<double, 3>;

    // If ITK resampler determines there is something to interpolate which is
    // usually the case when upscaling (!) then we must specify the interpolation
    // algorithm. In our case, we want bicubic interpolation. One way to implement
    // it is with a third order b-spline. So the type is specified here and the
    // order will be specified with a method call later on.
    using T_Interpolator = itk::LinearInterpolateImageFunction<ifind::Image, double>;

    // The resampler type itself.
    using T_ResampleFilter = itk::ResampleImageFilter<ifind::Image, ifind::Image>;
    // Instantiate the transform and specify it should be the id transform.
    T_Transform::Pointer _pTransform = T_Transform::New();
    _pTransform->SetIdentity();

    // Instantiate the b-spline interpolator and set it as the third order
    // for bicubic.
    T_Interpolator::Pointer _pInterpolator = T_Interpolator::New();

    // Instantiate the resampler. Wire in the transform and the interpolator.
    T_ResampleFilter::Pointer _pResizeFilter = T_ResampleFilter::New();
    _pResizeFilter->SetTransform(_pTransform);
    _pResizeFilter->SetInterpolator(_pInterpolator);

    // Set the output origin. You may shift the original image "inside" the
    // new image size by specifying something else than 0.0, 0.0 here.

    const double vfOutputOrigin[3] = { 0.0, 0.0, 0.0 };
    _pResizeFilter->SetOutputOrigin(vfOutputOrigin);


    // Fetch original image spacing.
    const ifind::Image::SpacingType & vfInputSpacing = in->GetSpacing();
    // Will be {1.0, 1.0} in the usual
    // case.

    double vfOutputSpacing[3];
    vfOutputSpacing[0] = vfInputSpacing[0] / 2.0;
    vfOutputSpacing[1] = vfInputSpacing[1] / 2.0;
    vfOutputSpacing[2] = 1.0;

    // Set the output spacing. If you comment out the following line, the original
    // image will be simply put in the upper left corner of the new image without
    // any scaling.
    _pResizeFilter->SetOutputSpacing(vfOutputSpacing);

    // Set the output size as specified on the command line.

    itk::Size<3> vnOutputSize = { { size[0] * 2, size[1] * 2, 1 } };
    _pResizeFilter->SetSize(vnOutputSize);

    // Specify the input.

    _pResizeFilter->SetInput(in);
    _pResizeFilter->Update();
    return _pResizeFilter->GetOutput();

}
