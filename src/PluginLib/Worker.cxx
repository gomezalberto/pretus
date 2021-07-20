#include "Worker.h"
#include <itkResampleImageFilter.h>
#include <itkConstantPadImageFilter.h>
#include <itkRegionOfInterestImageFilter.h>
#include <itkImportImageFilter.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkNearestNeighborInterpolateImageFunction.h>
#include <itkExtractImageFilter.h>
#include <itkCropImageFilter.h>
#include <chrono>

int Worker::gil_init = 0;

void Worker::set_gil_init(int n){
    Worker::gil_init = n;
}

Worker::Worker(QObject *parent) : QObject(parent)
{
    /// Initialize python stuff
    this->PythonInitialized = false;
    this->FrameCount = 0;
}

Worker:: ~Worker(){
    /// nothing to do here
}

bool Worker::getPythonInitialized() const
{
    return this->PythonInitialized;
}

void Worker::setPythonInitialized(bool value)
{
    this->PythonInitialized = value;
}

void Worker::setPluginName(const QString &pluginName)
{
    mPluginName = pluginName;
}

QString Worker::pluginName() const
{
    return mPluginName;
}

void Worker::slot_Work(ifind::Image::Pointer image){

    std::chrono::steady_clock::time_point t_begin, t_end ;
    if (this->params.measureTime){
        t_begin = std::chrono::steady_clock::now();
    }
    if (image != nullptr){
        this->doWork(image);
    }
    if (this->params.measureTime){
        t_end = std::chrono::steady_clock::now();
        int duration = std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_begin).count();
        if (duration >0){
            std::cout << "Worker::slot_Work(): Worker " << mPluginName.toStdString() << " took "<< duration << " ms " << std::endl;
        }
    }
    Q_EMIT this->WorkFinished();
}

ifind::Image::Pointer Worker::AdjustImageSize(ifind::Image::Pointer image){
    const int NDIMS = 2;
    using ImageResampleType = itk::ResampleImageFilter<ifind::Image,ifind::Image>;
    using TransformPrecisionType = double;
    using TransformType = itk::IdentityTransform< TransformPrecisionType, NDIMS+1 >;

    ifind::Image::SizeType in_size, out_size;
    ifind::Image::SpacingType in_spacing;

    in_size = image->GetLargestPossibleRegion().GetSize();
    in_spacing = image->GetSpacing();
    out_size[NDIMS] = 1;
    for (int i=0; i<NDIMS; i++){
        out_size[i] = std::ceil(in_size[i]*(in_spacing[i]/this->params.out_spacing[i]));
    }

    ImageResampleType::Pointer resampler = ImageResampleType::New();
    resampler->SetInput(image);
    resampler->SetOutputSpacing(this->params.out_spacing);
    resampler->SetOutputOrigin(image->GetOrigin());
    resampler->SetSize(out_size);
    resampler->SetTransform( TransformType::New() );
    resampler->Update();
    ifind::Image::Pointer upsampledImage = resampler->GetOutput();

    /// padd or crop the image if needed
    //int out_extent[] = {0,0,0,0,0,0};

    auto in_size2 = upsampledImage->GetLargestPossibleRegion().GetSize();
    //auto in_origin = upsampledImage->GetOrigin();

    int border_left[NDIMS], border_right[NDIMS], index[NDIMS];

    using ImagePadType = itk::ConstantPadImageFilter<ifind::Image,ifind::Image>;

    this->lb.Fill(0);
    this->ub.Fill(0);
    this->lbi.Fill(0); // for inverse padding
    this->ubi.Fill(0); // for inverse padding


    this->sizeCrop.Fill(0);

    for (int i=0; i< NDIMS; i++){
        int border2 = this->params.out_size[i]-int(in_size2[i]);
        if (i==1 && this->params.origin == WorkerParameters::OriginPolicy::Top){
            border_left[i] = 0;
            border_right[i] = border2;
        } else {
            border_left[i] = std::floor(border2/2.0);
            border_right[i] = std::ceil(border2/2.0);
        }

        this->extendLeft[i] = std::max(border_left[i],0);
        this->cropLeft[i] = std::max(-border_left[i],0);
        this->extendRight[i] = std::max(border_right[i],0);
        this->cropRight[i] = std::max(-border_right[i],0);

        this->sizeCrop[i] = int(in_size2[i])-this->cropLeft[i]-this->cropRight[i];

        lb[i] = this->extendLeft[i];
        ub[i] = this->extendRight[i];
        lbi[i] = this->cropLeft[i];
        ubi[i] = this->cropRight[i];
    }

    // First crop
    ifind::Image::RegionType extractRegion;
    //ifind::Image::SizeType final_size(this->params.out_size);
    extractRegion.SetSize(0, this->sizeCrop[0]);
    extractRegion.SetSize(1, this->sizeCrop[1]);
    extractRegion.SetSize(2,1);
    extractRegion.SetIndex(0,this->cropLeft[0]);
    extractRegion.SetIndex(1,this->cropLeft[1]);
    extractRegion.SetIndex(2,0);

    using CropImageType = itk::RegionOfInterestImageFilter<ifind::Image,ifind::Image>;
    CropImageType::Pointer cropper = CropImageType::New();
    cropper->SetInput(upsampledImage);
    cropper->SetRegionOfInterest(extractRegion);
    cropper->Update();

    ifind::Image::Pointer cp = cropper->GetOutput();

    /// Now, if necessary, do padding
    ImagePadType::Pointer padder = ImagePadType::New();

    padder->SetInput(cropper->GetOutput());
    padder->SetConstant(0);
    padder->SetPadLowerBound(lb);
    padder->SetPadUpperBound(ub);
    padder->Update();

    return padder->GetOutput();

}

Worker::GrayImageType::Pointer Worker::UnAdjustImageSize(GrayImageType::Pointer image, ifind::Image::Pointer reference){
    const int NDIMS = 2;
    using TransformPrecisionType = double;
    using TransformType = itk::IdentityTransform< TransformPrecisionType, NDIMS+1 >;
    GrayImageType::SizeType imagesize = image->GetLargestPossibleRegion().GetSize();

    GrayImageType::Pointer responsemap_uncropped, responsemap_unpadded;

    ///  Undo padding, if any, by cropping
    if ( imagesize[0]!=this->sizeCrop[0] || imagesize[1]!=this->sizeCrop[1] || this->extendLeft[0]!=0 || this->extendLeft[1]!=0){

        using CropImageType2 = itk::RegionOfInterestImageFilter<GrayImageType,GrayImageType>;
        CropImageType2::Pointer cropper2 = CropImageType2::New();

        GrayImageType::RegionType extractRegion;
        //ifind::Image::SizeType final_size(this->params.out_size);
        extractRegion.SetSize(0, this->sizeCrop[0]);
        extractRegion.SetSize(1, this->sizeCrop[1]);
        extractRegion.SetSize(2,1);
        extractRegion.SetIndex(0,this->extendLeft[0]);
        extractRegion.SetIndex(1,this->extendLeft[1]);
        extractRegion.SetIndex(2,0);

        cropper2->SetInput(image);
        cropper2->SetRegionOfInterest(extractRegion);
        // cropper2->SetNumberOfWorkUnits(20);
        cropper2->Update();

        responsemap_unpadded = cropper2->GetOutput();
    } else {
        responsemap_unpadded = image;
    }

    /// Undo cropping, if any, by padding
    if ( imagesize[0]!=this->sizeCrop[0] || imagesize[1]!=this->sizeCrop[1] || this->extendLeft[0]!=0 || this->extendLeft[1]!=0){
        using ImagePadType2 = itk::ConstantPadImageFilter<GrayImageType,GrayImageType>;
        ImagePadType2::Pointer padder2 = ImagePadType2::New();
        padder2->SetInput(responsemap_unpadded);
        padder2->SetConstant(0);
        padder2->SetPadLowerBound(lbi);
        padder2->SetPadUpperBound(ubi);
        //padder2->SetNumberOfWorkUnits(20);
        padder2->Update();
        responsemap_uncropped =  padder2->GetOutput();
    } else {
        responsemap_uncropped = responsemap_unpadded;
    }

    /// now upsample. This slows down processing significantly

    using ImageResampleType2 = itk::ResampleImageFilter<GrayImageType, GrayImageType>;
    //using InterpolatorType = itk::LinearInterpolateImageFunction<GrayImageType, double>;
    using InterpolatorType = itk::NearestNeighborInterpolateImageFunction<GrayImageType, double>;
    InterpolatorType::Pointer interpolator = InterpolatorType::New();
    ImageResampleType2::Pointer resampler2 = ImageResampleType2::New();
    resampler2->SetInput(responsemap_uncropped);
    resampler2->SetOutputSpacing(reference->GetSpacing());
    resampler2->SetOutputOrigin(reference->GetOrigin());
    resampler2->SetSize(reference->GetLargestPossibleRegion().GetSize());
    resampler2->SetTransform( TransformType::New() );
    //resampler2->SetNumberOfWorkUnits(20);
    resampler2->SetInterpolator(interpolator);
    resampler2->Update();
    return resampler2->GetOutput();
}

/**
 * @brief Crop image to exclude the GE screen formating and also to have approximately the correct aspect ratio.
 * Later in the python code it will be resized to a hard coded value with that aspect ratio.
 * @param image
 * @param percentage_xy the percentage of the image to keep in the x and y direction
 * @param offset_y The offset of the image to crop in the depth direction (0: no offset, 1: maximum offset)
 * @return Pointer to the itk::Image
 */
Worker::GrayImageType2D::Pointer Worker::crop_ifind_2D_image_data(GrayImageType2D::Pointer image)
{
    GrayImageType2D::SizeType image_size = image->GetLargestPossibleRegion().GetSize();

    GrayImageType2D::SizeType crop_l, crop_u;

    /*
    crop_l[0] = static_cast<unsigned int>( 0.170 * image_size[0] );
    crop_u[0] = static_cast<unsigned int>( 0.170 * image_size[0] );
    crop_l[1] = static_cast<unsigned int>( 0.130 * image_size[1] );
    crop_u[1] = static_cast<unsigned int>( 0.02  * image_size[1] );
    */

    crop_l[0] = static_cast<unsigned int>( 0.22 * image_size[0] );
    crop_u[0] = static_cast<unsigned int>( 0.22 * image_size[0] );
    crop_l[1] = static_cast<unsigned int>( 0.15 * image_size[1] );
    crop_u[1] = static_cast<unsigned int>( 0.15  * image_size[1] );

    typedef itk::CropImageFilter <GrayImageType2D, GrayImageType2D> CropImageFilterType;

    CropImageFilterType::Pointer cropfilter = CropImageFilterType::New();

    cropfilter->SetInput(image);
    cropfilter->SetUpperBoundaryCropSize(crop_u);
    cropfilter->SetLowerBoundaryCropSize(crop_l);
    //cropfilter->SetNumberOfThreads(40); // this is deprecated, using the syntax below
    //cropfilter->SetNumberOfWorkUnits(40);
    cropfilter->Update();
    return cropfilter->GetOutput();
}

Worker::GrayImageType2D::Pointer Worker::get2dimage(ifind::Image::Pointer image)
{
    ifind::Image::Superclass::SizeType s = image->GetLargestPossibleRegion().GetSize();
    if (s[2] > 1)
        return get2dimagefrom3d(image);

    GrayImageType2D::Pointer im = GrayImageType2D::New();
    GrayImageType2D::PointType imorig; imorig[0] = image->GetOrigin()[0]; imorig[1] = image->GetOrigin()[1];
    GrayImageType2D::SpacingType imspacing; imspacing[0] = image->GetSpacing()[0]; imspacing[1] = image->GetSpacing()[1];
    GrayImageType2D::SizeType imsize; imsize[0] = s[0]; imsize[1] = s[1];
    GrayImageType2D::RegionType imregion(imsize);
    im->SetRegions(imregion);
    im->SetPixelContainer(image->GetPixelContainer());
    im->SetOrigin(imorig);
    im->SetSpacing(imspacing);
    im->SetMetaDataDictionary(image->GetMetaDataDictionary());
    return im;
}

/**
 */
Worker::GrayImageType2D::Pointer Worker::get2dimagefrom3d(ifind::Image::Pointer image)
{
    typedef itk::ExtractImageFilter<ifind::Image::Superclass, GrayImageType2D> ExtracterType;
    ExtracterType::Pointer extracter = ExtracterType::New();
    //extracter->SetNumberOfWorkUnits(40);
    extracter->SetInput(image.GetPointer());
    extracter->InPlaceOn();
    extracter->SetDirectionCollapseToSubmatrix();
    ifind::Image::Superclass::RegionType inputRegion = image->GetLargestPossibleRegion();
    ifind::Image::Superclass::SizeType  esize = inputRegion.GetSize();
    ifind::Image::Superclass::IndexType start = inputRegion.GetIndex();
    start[2] = static_cast<int>(static_cast<double>(esize[2])/2.);
    esize[2] = 0;
    ifind::Image::Superclass::RegionType desiredRegion;
    desiredRegion.SetSize(  esize  );
    desiredRegion.SetIndex( start );
    extracter->SetExtractionRegion( desiredRegion );
    extracter->Update();

    GrayImageType2D::Pointer ret = extracter->GetOutput();
    ret->SetMetaDataDictionary(image->GetMetaDataDictionary());
    return ret;
}

Worker::GrayImageType::Pointer Worker::get3dimagefrom2d(GrayImageType2D::Pointer image){

    GrayImageType::PointType imorig; imorig[0] = image->GetOrigin()[0]; imorig[1] = image->GetOrigin()[1]; imorig[2] = 0;
    GrayImageType::SpacingType imspacing; imspacing[0] = image->GetSpacing()[0]; imspacing[1] = image->GetSpacing()[1]; imspacing[2] = 1;
    GrayImageType2D::SizeType s = image->GetLargestPossibleRegion().GetSize();
    GrayImageType::SizeType imsize; imsize[0] = s[0]; imsize[1] = s[1]; imsize[2] = 1;
    GrayImageType::RegionType imregion(imsize);
    GrayImageType::Pointer im = GrayImageType::New();
    im->SetRegions(imregion);
    im->SetPixelContainer(image->GetPixelContainer());
    im->SetOrigin(imorig);
    im->SetSpacing(imspacing);
    im->SetMetaDataDictionary(image->GetMetaDataDictionary());
    return im;
}
