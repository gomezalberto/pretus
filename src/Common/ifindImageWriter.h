#ifndef ifindImageWriter_h_
#define ifindImageWriter_h_

#include <exports/ifind2_common_Export.h>

#include <mutex>

#include <itkVTKImageToImageFilter.h>
#include <itkImageFileWriter.h>

#include "ifindImage.h"

namespace ifind
{

  /**
   * @class ImageWriter
   * @author Alberto Gomez
   * @brief class for writing an image using ITK/VTK from an import of a ifind::Image
   */
  class  iFIND2COMMON_EXPORT ImageWriter {
  public:

    typedef std::shared_ptr<ImageWriter> Pointer;

  public:

    ImageWriter();
    ~ImageWriter(){};

    typedef ifind::Image::Superclass ImageType;
    typedef itk::ImageFileWriter<ImageType> WriterType;
    typedef itk::VTKImageToImageFilter<ImageType> ConverterType;

    void Write(Image::Pointer arg, bool headerOnly = false);
    void WriteAs(Image::Pointer arg, std::string & filename, bool headerOnly = false);

    void SetPath(std::string& arg)
    { this->m_Path = arg; }

    std::string GetPath(void)
    { return this->m_Path; }

    std::vector<std::string> GetFullFilename();

    static std::string CreateFileName(ifind::Image::Pointer arg, unsigned int layer = 0, bool withextension = false);


  private:

    std::string m_Path;
    std::vector<std::string> m_FileNames;

    std::mutex m_Mutex;
  };
} // end of namespace

#endif // ifindImageWriter_h_
