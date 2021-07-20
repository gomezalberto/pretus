#include "ifindImageWriter.h"

#include <itkMetaImageIO.h>

namespace ifind
{

ImageWriter::ImageWriter()
{
}


std::vector<std::string> ImageWriter::GetFullFilename()
{
    return this->m_FileNames;
}

std::string ImageWriter::CreateFileName(ifind::Image::Pointer arg, unsigned int layer, bool withextension)
{
    std::ostringstream filename;
    filename << "image_transducer"
             << std::stoi(arg->GetMetaData<std::string>("TransducerNumber")) << "_"
             << std::stoi(arg->GetMetaData<std::string>("NDimensions")) << "D_"
             << arg->GetMetaData<std::string>("DNLTimestamp");

    if(layer >= 0)
        filename << "_layer" << layer;

    if (withextension)
        filename << ".mhd";

    return filename.str();
}

void ImageWriter::Write(Image::Pointer arg, bool headerOnly)
{
    this->m_Mutex.lock();

    this->m_FileNames.clear();
    for(unsigned int l = 0; l < arg->GetNumberOfLayers(); l++)
    {
        /// extract name information
        std::string fname = this->CreateFileName(arg, l, true);
        std::string filename = this->m_Path + std::string("/") + fname;
        this->m_FileNames.push_back(filename);
        /// convert layer into ITK format
        ConverterType::Pointer converter = ConverterType::New();
        converter->SetInput(arg->GetVTKImage(l));
        converter->Update();
        ImageType::Pointer layer = converter->GetOutput();
        layer->SetMetaDataDictionary(arg->GetMetaDataDictionary());
        /// write the image
        WriterType::Pointer writer = WriterType::New();
        writer->SetFileName(filename);
        writer->SetInput(layer);
        writer->Update();
    }

    this->m_Mutex.unlock();
}

void ImageWriter::WriteAs(Image::Pointer arg, std::string & filename, bool headerOnly)
{
    this->m_Mutex.lock();

    {

        /// convert layer into ITK format
        ConverterType::Pointer converter = ConverterType::New();
        converter->SetInput(arg->GetVTKImage(0));
        converter->Update();
        ImageType::Pointer layer = converter->GetOutput();
        layer->SetMetaDataDictionary(arg->GetMetaDataDictionary());
        /// write the image
        WriterType::Pointer writer = WriterType::New();
        writer->SetFileName(filename);
        writer->SetInput(layer);
        writer->Update();

    }
    this->m_Mutex.unlock();
}
} // end of namespace
