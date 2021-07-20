#pragma once

#include <Plugin.h>

#include <itkImageFileWriter.h>
#include <itkVTKImageToImageFilter.h>

#include <mutex>
#include <map>
#include <array>
#include <ifindStreamTypeHelper.h>
#include "Widget_imageFileWriter.h"

class Plugin_imageFileWriter : public Plugin {
    Q_OBJECT

public:

    typedef Widget_imageFileWriter WidgetType;
    typedef itk::ImageFileWriter<ifind::Image> WriterType;
    typedef itk::VTKImageToImageFilter<ifind::Image> ConverterType;

    Plugin_imageFileWriter(QObject* parent = 0);

    QString GetPluginName(void){ return "Image file writer";}

    QString GetPluginDescription(void){return "Save image data to the file system.";}

    void SetCommandLineArguments(int argc, char* argv[]);


    void Initialize(void);

    virtual void SetActivate(bool arg){/* Nothing to be done here*/};

    void SetOutputFolder(std::string& arg){ this->OutputFolder = arg; }
    std::string GetOutputFolder(void){ return this->OutputFolder; }

    virtual QtPluginWidgetBase *GetWidget();

public Q_SLOTS:
    /**
     * @brief This plug-in must work for all images in the order they come,
     * so overloading this function removes the timed functionality
     * @param image
     */
    virtual void slot_imageReceived(ifind::Image::Pointer image);

Q_SIGNALS:

    void ImageToBeSaved(ifind::Image::Pointer image);

protected:
    virtual void SetDefaultArguments();

private:
    std::string OutputFolder;

    ifind::StreamTypeSet m_streamtype_to_write;
    int m_folder_policy;
    /// if 0, no subdivision. Greater than 0, indicates the maximum n. images per folder
    int subdivide_folders;
    int first_subdivision;
    /// counts the n images written to the folder with name string, and current_number_of_splits of that folder
    std::map<std::string, std::array<int,2> > write_count;


    std::string CreateFileName(ifind::Image::Pointer arg, unsigned int layer = 0, bool withextension = false);
    void Write(ifind::Image::Pointer arg, bool headerOnly = false);
    std::mutex m_Mutex;

};
