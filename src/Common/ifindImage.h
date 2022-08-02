#ifndef ifindImage_h_
#define ifindImage_h_

#include <exports/ifind2_common_Export.h>

#include <itkImage.h>
#include <itkMetaDataDictionary.h>
#include <itkMetaDataObject.h>
#include <itkImageToVTKImageFilter.h>

#include <vtkSmartPointer.h>
#include <chrono>

class vtkImageData;
class vtkMatrix4x4;

namespace ifind
{

static long int LocalTimeStamp(void)
{
    std::chrono::milliseconds local_timestamp =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
    return local_timestamp.count();
}


/**
 * @class Image
 * @author Alberto Gomez, Nicolas Toussaint
 * @date 12/05/2016
 * @brief Main Class container for an image object and its metadata.
 * @details
 *
 * The class ifind::Image is the main class container for an image object and its metadata.
 * It derives from itk::Image<unsigned int, 3> for convenience. The underlying image is the "background" B-mode layer.
 * Additionally, it can contain pointers to vtkImageData objects (overlays).
 * It provides access methods to each layer using GetVTKImage()
 *
 * It contains extendable metadata (GetMetaDataDictionary()) information concerning the acquisition parameters and the acquisition time stamps
 * provided by, e.g. the Transducer, the DNL object, the Force and the Position trackers.
 *
 * Use GetMetaData() for access
 *
 * The default metadata initialized at construction are the following (GetKeys()):
 *
 \code{.cpp}
     - AcquisitionFrameRate   - std::string (int);
     - CalibrationMatrix      - std::string (matrix 4x4);
     - DepthOfScanField       - std::string (unsigned int);
     - Debug                  - std::string (bool);
     - DNLLayerTimeTag        - std::string (uint64_t) time elapsed (in microseconds) since EPIQ booted at each layer acquisition
     - DNLTimestamp           - std::string            time elapsed (in milliseconds) since EPIQ EPOQ (1 January 1970)
     - EchoGain               - std::string (float)
     - FocusDepth             - std::string (float)
     - ForceData              - std::string (double)
     - ForceTimestamp         - std::string
     - ImageMode              - std::string (enum int)
     - LocalTimestamp         - std::string
     - NDimensions            - std::string (int)
     - PatientName            - std::string
     - ReorientMatrix         - std::string (matrix 4x4)
     - SectorAngle            - std::string (float)
     - SectorWidth            - std::string (int)
     - TrackerMatrix          - std::string (matrix 4x4)
     - TrackerTimestamp       - std::string
     - TransducerMatrix       - std::string (matrix 4x4)
     - TransducerNumber       - std::string (int)
     - TransducerTimestamp    - std::string
     - TransmissionFrameRate  - std::string (int)
  \endcode
  *
  * You can instantiate this class from a itk::Image using Graft().
  *
  * You can add overlays using GraftOverlay().
  *
  *
 */
class iFIND2COMMON_EXPORT Image: public itk::Image<uint8_t, 3>
{

public:
    /**
   * typedefs and enums
   */
    typedef ifind::Image Self;
    typedef itk::Image<itk::RGBPixel<uint8_t>, 3> ColorImageType;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;
    typedef itk::Image<uint8_t, 3> Superclass;
    typedef std::string StreamType;

    typedef itk::ImageToVTKImageFilter<Superclass> ConverterType;

    typedef itk::MetaDataDictionary DictionaryType;

    enum ImageMode : int
    {
        TwoD = 0,
        ThreeDCartesian,
        ThreeDFrustum,
        External
    };
    enum ImageFormat : int
    {
        ITK = 0,
        VTK
    };

    /**
   * @brief Constructors and Destructor
   */
    itkNewMacro  (Self);
    itkTypeMacro (ifind::Image, Superclass);

    /**
   * Fill the metadataset dictionary with this method
   *
   * Example :
   \code{.cpp}
     myimage->SetMetaData<std::string>( "id", std::to_string(myid) );
   \endcode
   */
    template <class type> inline void SetMetaData(std::string key, type value)
    {
        itk::MetaDataDictionary & dic = this->GetMetaDataDictionary();
        if(dic.HasKey(key))
            itk::EncapsulateMetaData<type>(dic, key, (type)value);
        else
            itk::EncapsulateMetaData<type>(dic, key, (type)value);
    }

    /**
   * Get a metadataset dictionary value with this method
   * This is not const on purpose so that twe can modify pointers if needed
   * The method *does not* throw any exception if the key is invalid. It warns the user in std::cerr.
   *
   * Example :
   \code{.cpp}
     std::string =  myimage->GetMetaData<std::string>( "id");
   \endcode
   */
    template <class type> inline type GetMetaData(const char* key, bool valid = true) const
    {
        itk::MetaDataDictionary dic = this->GetMetaDataDictionary();
        type toret;
        valid = itk::ExposeMetaData<type>(dic, key, toret);
        if(!valid){
            //std::cerr << "[ifindImage] ERROR: cannot find key: " << key << std::endl;
        }
        return toret;
    }


    /**
   * @brief Query the dictionary weither it contains a specific key
   * @param key the queried key
   * @return true if the metadata dictionary contains the key, false otherwise
   */
    bool HasKey(const char* key)
    { return this->GetMetaDataDictionary().HasKey(key); };

    /**
   * @brief Returns the list of metadata keys contained in the dictionary
   * @return
   */
    std::vector<std::string> GetKeys(void) const
    {
        std::vector<std::string> ret;
        ret = this->GetMetaDataDictionary().GetKeys();
        return ret;
    }

    /**
     * @brief Copies the pointer to the image data (only base layer) and leaves metadata untouched.
     * Basically does not really copy anything but the image data will have a pointer to the reference
     * image data. This can be useful to initialise images
     * @param data
     */
    virtual void VeryShallowCopy (const Image *data);

    /**
     * @brief Copies meta data from input while maintaining the pointer to the pixel data.
     * @param data
     */
    virtual void ShallowCopy (const Image *data);

    /**
     * @brief Merges meta data from input iwhile maintaining existing meta data. If there
     * is conflicting data, the input image writes over the current object.
     * @param data
     */
    virtual void ShallowMerge(const Image *data, bool mergeLayers=false);

    /**
     * @brief SetSpacingAllLAyers
     * @param spacing
     */
    virtual void SetSpacingAllLAyers(const SpacingType &spacing);

    /**
   * @brief Graft the information and PixelData from the input
   * @param data
   */
    virtual void Graft (const Superclass *data, std::string layername);
   /**
   * @brief Graft the information and PixelData from the input and put it as overlay
   * @param data
   */
    virtual void GraftOverlay (const Superclass *data, unsigned int index, std::string layername);

    /**
    * @brief Graft the information and PixelData from the input and put it as overlay
    * @param data
    */
     virtual void GraftOverlay (const vtkSmartPointer<vtkImageData> data, unsigned int index);

    /**
   * @brief Returns the number of layers (original image + overlays)
   * @return number of layers
   */
    unsigned int GetNumberOfLayers(void) const
    { return m_Layers.size(); }

    /**
   * @brief Returns the names of layers (original image + overlays)
   * @return vector with names of layers
   */
    std::vector<std::string> GetLayerNames(void) const
     { return m_LayerNames; }

    /**
   * @brief Returns the image (or overlay) in a VTK format
   * @param layer the layer ID
   *
   * If the @param layer is zero, then the method returns the output of the internal itk to vtk converter.
   * If @param layer is greater than zero, the corresponding overlay is returned.
   *
   */
    vtkSmartPointer<vtkImageData> GetVTKImage(int layer = 0) const;

    /**
   * @brief Returns the image (or overlay) in a VTK format
   * @param layer the layer ID
   *
   * If the @param layer is zero, then the method returns the output of the internal itk to vtk converter.
   * If @param layer is greater than zero, the corresponding overlay is returned.
   *
   */
    Superclass::Pointer GetOverlay(int layer = 0) const
    {
        return this->m_Converters[layer]->GetInput();
    }

    /**
   * @brief Returns the multiplication of all internal matrices
   */
    vtkSmartPointer<vtkMatrix4x4> GetTotalMatrix(void);

    /**
   * @brief Convenient method to return the metadata as a vtkMatrix4x4
   * @param key
   * @return the matrix corresponding to the key
   *
   * No check on the existence of the key is performed
   */
    vtkSmartPointer<vtkMatrix4x4> GetMetaDataMatrix (std::string key);


    StreamType GetStreamType();

    void SetStreamType(const StreamType &st);

protected:

    Image();
    ~Image(){}

    /**
   * @brief Prints the content of the ifind::Image (metadata and image content)
   * @param os
   * @param indent
   */
    void PrintSelf(std::ostream & os, itk::Indent indent) const;


private:

    Image(const Self&);
    void operator=(const Self&);

    std::vector<ConverterType::Pointer> m_Converters;
    std::vector<vtkSmartPointer<vtkImageData> > m_Layers;
    std::vector< std::string> m_LayerNames;

};

} // end of namespace

#endif // ifindImage_h_
