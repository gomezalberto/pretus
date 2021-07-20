#ifndef ifindImageREADER_H_
#define ifindImageREADER_H_

#include <exports/ifind2_common_Export.h>

#include "ifindImage.h"


namespace ifind{

  /**
   * @class ImageReader
   * @author Alberto Gomez, Nicolas Toussaint
   * @brief class for reading an image using ITK and exporting it as ifind::Image
   * 
   * @details
   * 
   * 
   * 
   */
  class iFIND2COMMON_EXPORT ImageReader {

  public:

    typedef std::shared_ptr<ImageReader> Pointer;
    static Pointer New()
    {
      return Pointer(new ImageReader());
    }
    ~ImageReader(){};

    /**
     * @brief set the input file name
     * @param filename
     * 
     * The filename should be a header readable via ITK
     * 
     * The header may contain the "old format" metadata using the '#' prefix, 
     * 
     * OR the "new format" using the normal ITK MetaDataDictionary.
     */
    void SetFileName(const char *filename)
    { this->m_FileName = filename; }

    /**
     * @brief Read the image stored in file via SetFileName()
     * 
     * The method populates the metadatadictionary according to the header and
     * fill the layers information and image data into the ifind::Image object
     */
    virtual void Read(void);

    /**
     * @brief Read the layer and graft it into the ifind::Image
     * @param filename input filename
     * @param layer the layer ID
     */
    virtual void ReadLayer(const char* filename, unsigned int layer);

    /**
     * @brief Read only the header of the argument filename
     * @param filename
     */
    virtual bool ReadHeader(const char* filename);

    /**
     * @brief get the ifind::Image after reading
     * @return ifind::Image object
     * 
     * Use after calling Read()
     */
    Image::Pointer GetifindImage()
    { return this->m_Image; }


  protected:

    ImageReader();

    /**
     * @brief Internal method to evaluate the number of layers
     * @param filename input filename (can be any of the layers' headers
     * @param layers this argument vector is filled with the layers' headers
     */
    virtual void EvaluateLayers(const char* filename, std::vector<std::string>& layers);

    std::string m_FileName;
    Image::Pointer m_Image;

  private:

  };
}

#endif // ifindImageREADER_H_
