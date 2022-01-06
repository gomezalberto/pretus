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
#include <opencv2/opencv.hpp>
#include <itkOpenCVImageBridge.h>
#include <itkVectorIndexSelectionCastImageFilter.h>

FrameGrabberManager::FrameGrabberManager(QObject *parent) : Manager(parent){
    this->Cap = nullptr;
    this->latestAcquisitionTime = std::chrono::steady_clock::now();
    this->initialAcquisitionTime = std::chrono::steady_clock::now();
    this->TransmitFrameRate.set_capacity(60);
    this->mIsPaused = false;
    //gg::ColourSpace colour = gg::ColourSpace::I420;
    gg::ColourSpace colour = gg::ColourSpace::BGRA;
    gg::VideoFrame frame(colour);
    this->Frame = new gg::VideoFrame(frame);
    this->mDemoFile = "";
}

int FrameGrabberManager::Initialize(){

    if (this->mDemoFile.length() > 0){
        std::cout << "[FrameGrabberManager::Initialize] Initializing in demo mode using "<< this->mDemoFile<<"- no actual framegrabber needed"<<std::endl;
        return 0;
    }

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


    if (!this->mIsPaused && this->mDemoFile.length() == 0){
        //gg::ColourSpace colour = gg::ColourSpace::I420;
        gg::ColourSpace colour = gg::ColourSpace::BGRA;
        gg::VideoFrame frame(colour);

        {
            try {

                this->Cap->get_frame(frame);

            } catch (...) {
                std::cerr << "[Error] FrameGrabberManager::GetFrame() -  reading frame from framegrabber. "  << std::endl;
            }
        }

        assert( frame.data() != NULL );

        this->mutex_Frame.lock();
        this->Frame = &frame;
        this->mutex_Frame.unlock();
    }

    if (this->mDemoFile.length() > 0 ){
        if (this->mDemoFile.length()==1) {

            // This snippet of code can be used to generate demo files:
            {
                std::cout << "FrameGrabberManager::getFrameAsIfindImageData - "<< this->Frame->rows() << "x"<< this->Frame->cols() <<", total: "<< this->Frame->data_length()<<std::endl;
                /// Test to write data
                std::string filename("/tmp/epiphan_data_char.bin");
                ofstream outfile(filename, ios::out | ios::binary);
                outfile.write(reinterpret_cast< char *>(this->Frame->data()), this->Frame->data_length());
            }

        } else {
            // load from file

            std::streampos begin,end;
            ifstream myfile(this->mDemoFile, ios::in | ios::binary);
            if (myfile.is_open())
            {

                begin = myfile.tellg();
                myfile.seekg (0, ios::end);
                end = myfile.tellg();

                int nbytes = end-begin;
                char *data = new char[nbytes];
                myfile.seekg (0, ios::beg);
                myfile.read(data, nbytes);
                myfile.close();

                int cols = 1920;
                int rows = 1080;

                this->Frame->init_from_specs(reinterpret_cast< unsigned char *>(data), nbytes, cols, rows);
            }
        }
    }





    if (this->Frame->rows() == 0 || this->Frame->cols() == 0){
        return nullptr;
    }


    /// Convert to RGB
    const unsigned long numberOfPixels = this->Frame->cols() *this->Frame->rows();
    //const unsigned long numberOfPixelsUV = this->Frame->cols()/ 2.0 * this->Frame->rows() / 2.0 ;
    cv::Mat myuv(this->Frame->rows() + this->Frame->rows()/2, this->Frame->cols(), CV_8UC1, (unsigned char *) this->Frame->data());
    //cv::imwrite("/home/ag09/data/VITAL/lungs/epiphan/data/yuv.png", myuv);
    cv::Mat mrgb(this->Frame->rows(), this->Frame->cols(), CV_8UC3);
    //cv::Mat mgray(this->Frame->rows(), this->Frame->cols(), CV_8UC1, ( unsigned char *)this->Frame->data());
    cvtColor(myuv, mrgb, CV_YUV2BGR_I420); // CV_YUV2BGR_IYUV, CV_YUV2RGB_YV12
    //cv::imwrite("/home/ag09/data/VITAL/lungs/epiphan/data/rgb.png", mrgb);

    //std::cout << "[FrameGrabberManager::getFrameAsIfindImageData] converted"<<std::endl;

    /// Do the studio swing

    const ifind::Image::PixelType *p_end = &mrgb.data[0]+numberOfPixels*3;
    double factor0 = 1.0;
    double factor1 = 0.0;
    if (this->params.correct_studio_swing > 0) {
        factor0 = 255./(235.-this->params.correct_studio_swing);
        factor1 = this->params.correct_studio_swing;
    }
    for (ifind::Image::PixelType *p = &mrgb.data[0]; p <  p_end; ++p){
        ifind::Image::PixelType newval = static_cast<ifind::Image::PixelType>(std::floor( (static_cast<double>(*p)-factor1)*factor0));
        *p = std::min(std::max(newval,ifind::Image::PixelType(0)), ifind::Image::PixelType(255));
    }

    ifind::Image::Pointer image = ifind::Image::New();

    typedef itk::OpenCVImageBridge BridgeType;
    // Convert to colour image
    ifind::Image::ColorImageType::Pointer colourImage = ifind::Image::ColorImageType::New();
    colourImage = BridgeType::CVMatToITKImage<ifind::Image::ColorImageType>(mrgb);

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

    return image;

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
