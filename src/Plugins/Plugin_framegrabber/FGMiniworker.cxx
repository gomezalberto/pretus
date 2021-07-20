#include <FGMiniworker.h>
#include <ifindImageWriter.h>
#include <itkImageFileWriter.h>

void FGMiniworker::doWork(ifind::Image::Pointer im){
        std::cout<< "[FGMiniworker::doWork()] - image received, save to "<< filename <<std::endl;

        typedef itk::ImageFileWriter<ifind::Image> WriterType;

        /// write the image
        WriterType::Pointer writer = WriterType::New();
        writer->SetFileName(filename);
        writer->SetInput(im);
        writer->Update();

}

