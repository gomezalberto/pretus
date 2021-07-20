#include "ifindImageReader.h"

#include <itkImageFileReader.h>
#include <vtkMatrix4x4.h>

namespace ifind
{

  ImageReader::ImageReader()
  {
    this->m_Image = Image::New();
  }


  void ImageReader::Read (void)
  {
    std::vector<std::string> layers;
    this->EvaluateLayers(this->m_FileName.c_str(), layers);

    if (!layers.size())
      this->ReadLayer(this->m_FileName.c_str(), 0);
    
    else if (this->ReadHeader(this->m_FileName.c_str()))
      for (unsigned int i=0; i<layers.size(); i++)
        this->ReadLayer(layers[i].c_str(), i);
  }


  void ImageReader::ReadLayer (const char* filename, unsigned int layer)
  {
    typedef itk::ImageFileReader<Image::Superclass> ReaderType;
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(filename);
    reader->Update();
    /// impose the origin so that the centre of the image is 0,0,0
    Image::Superclass::Pointer image = reader->GetOutput();
    image->DisconnectPipeline();
    Image::Superclass::SizeType imagesize = image->GetLargestPossibleRegion().GetSize();
    Image::Superclass::SpacingType imagespacing = image->GetSpacing();
    Image::Superclass::PointType origin;
    origin[0] = - 0.5 * (imagesize[0] - 1) * imagespacing[0];
    origin[1] =   0.0;
    origin[2] = - 0.5 * (imagesize[2] - 1) * imagespacing[2];
    image->SetOrigin(origin);
    if (layer == 0)
    {
      this->m_Image->Graft(image);
      ifind::Image::DictionaryType dic = this->m_Image->GetMetaDataDictionary();
      ifind::Image::DictionaryType additionaldic = reader->GetImageIO()->GetMetaDataDictionary();
      std::vector<std::string> keys = additionaldic.GetKeys();
      for (unsigned int i=0; i<keys.size(); i++)
        dic[keys[i]] = additionaldic[keys[i]];
      this->m_Image->SetMetaDataDictionary(dic);
      std::string dimensionalitystring;
      bool valid = itk::ExposeMetaData<std::string>(dic, "NDimensions", dimensionalitystring);
      if (!valid || std::stoi(dimensionalitystring) < 2)
      {
        unsigned int dimensionality = imagesize[2] > 1 ? 3 : 2;
        this->m_Image->SetMetaData<std::string>("NDimensions", std::to_string(dimensionality));
        this->m_Image->SetMetaData<std::string>("ImageMode", std::to_string( dimensionality == 3 ? ifind::Image::ImageMode::ThreeDCartesian : ifind::Image::ImageMode::TwoD));
      }
    }
    else
      this->m_Image->GraftOverlay(image, layer);
  }


  void ImageReader::EvaluateLayers (const char* filename, std::vector<std::string>& layers)
  {
    std::string fn = filename;
    size_t found = fn.find_last_of("_layer");
    if (!found)
      return;

    std::string prefix = fn.substr(0, found+1);
    for (unsigned int i=0; i<16; i++)
    {
      std::string layerfilename = prefix + std::to_string(i) + ".mhd";
      if ( itksys::SystemTools::FileExists(layerfilename.c_str(), true) )
        layers.push_back(layerfilename);
    }
  }


  bool ImageReader::ReadHeader (const char* filename)
  {
    std::vector<std::string> layers;
    this->EvaluateLayers(filename, layers);
    unsigned int nlayers = layers.size();
    int dimensionality;
    double matrix_values[16];
    std::string fn = std::string(filename).substr(std::string(filename).find_last_of("/\\")+1);
    std::string transducernumber = "0";
    if (fn.length()>16){
        transducernumber = fn.substr(16,1);
    }
    this->m_Image->SetMetaData<std::string>("TransducerNumber", transducernumber);

    std::ifstream myfile;
    myfile.open(filename, fstream::in);
    if (!myfile.is_open())
    {
       std::cerr << "[ifind::ImageReader::ReadHeader()] WARNING, file cannot be opened for reading" << std::endl;
       return false;
    }

    /// Read meta data ///
    while (!myfile.eof())
    {
      /// read line
      char strdata[200];
      myfile.getline(strdata, 200);
      std::string line = strdata;
      std::istringstream is (line);
      std::string dump;

      if (itksys::SystemTools::StringStartsWith(line, "NDims ="))
      {
        is >> dump >> dump >> dimensionality; // read 'NDims' and '=' first
        this->m_Image->SetMetaData<std::string>("NDimensions", std::to_string(dimensionality));
        this->m_Image->SetMetaData<std::string>("ImageMode", std::to_string( dimensionality == 3 ? ifind::Image::ImageMode::ThreeDCartesian : ifind::Image::ImageMode::TwoD));
      }
      else if (itksys::SystemTools::StringStartsWith(line, "#TRANSDUCERMATRIX"))
      {
        std::string v, t;
        is >> dump; // read keyword first
        for (unsigned int i=0; i<16; i++)
        {
          is >> t;
          v += t + " ";
        }
        this->m_Image->SetMetaData<>("TransducerMatrix", v);
      }
      else if (itksys::SystemTools::StringStartsWith(line, "#TRACKERMATRIX"))
      {
        std::string v, t;
        is >> dump; // read keyword first
        for (unsigned int i=0; i<16; i++)
        {
          is >> t;
          v += t + " ";
        }
        this->m_Image->SetMetaData<>("TrackerMatrix", v);
      }
      else if (itksys::SystemTools::StringStartsWith(line, "#REORIENTMATRIX"))
      {
        std::string v, t;
        is >> dump; // read keyword first
        for (unsigned int i=0; i<16; i++)
        {
          is >> t;
          v += t + " ";
        }
        this->m_Image->SetMetaData<>("ReorientMatrix", v);
      }
      else if (itksys::SystemTools::StringStartsWith(line, "#FORCE"))
      {
        std::string v, t;
        is >> dump; // read keyword first
        for (unsigned int i=0; i<6; i++)
        {
          is >> t;
          v += t + " ";
        }
        this->m_Image->SetMetaData<>("ForceData", v);
      }
      else if (itksys::SystemTools::StringStartsWith(line, "#TIMESTAMP_LOCAL"))
      {
        std::string v;
        is >> dump; // read keyword first
        is >> v;
        this->m_Image->SetMetaData<>("LocalTimestamp", v);
      }
      else if (itksys::SystemTools::StringStartsWith(line, "#TIMESTAMP_DNL"))
      {
        std::string v;
        is >> dump; // read keyword first
        is >> v;
        this->m_Image->SetMetaData<>("DNLTimestamp", v);
      }
      else if (itksys::SystemTools::StringStartsWith(line, "#TIMESTAMP_DNLLAYERTIMELAG"))
      {
        std::string v, t;
        is >> dump; // read keyword first
        for (unsigned int i=0; i<nlayers; i++)
        {
          is >> t;
          v += t + " ";
        }
        this->m_Image->SetMetaData<>("DNLLayerTimeTag", v);
      }
      else if (itksys::SystemTools::StringStartsWith(line, "#TIMESTAMP_TRACKER"))
      {
        std::string v;
        is >> dump; // read keyword first
        is >> v;
        this->m_Image->SetMetaData<>("TrackerTimestamp", v);
      }
      else if (itksys::SystemTools::StringStartsWith(line, "#ACQFR"))
      {
        std::string v, t;
        is >> dump; // read keyword first
        for (unsigned int i=0; i<nlayers; i++)
        {
          is >> t;
          v += t + " ";
        }
        this->m_Image->SetMetaData<>("AcquisitionFrameRate", v);
      }
      else if (itksys::SystemTools::StringStartsWith(line, "#TXFR"))
      {
        is >> dump; // read keyword first
        std::string v, t;
        for (unsigned int i=0; i<nlayers; i++)
        {
          is >> t;
          v += t + " ";
        }
        this->m_Image->SetMetaData<>("TransmissionFrameRate", v);
      }
    }
    myfile.close();
    return true;
  }
} // end of namespace
