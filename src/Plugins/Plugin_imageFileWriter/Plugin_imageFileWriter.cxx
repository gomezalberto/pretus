#include "Plugin_imageFileWriter.h"
#include <ifindImagePeriodicTimer.h>
#include <thread>
#include <QDir>
#include <QDebug>
#include <QObject>

static const std::string sDefaultStreamTypeToWrite("Input");

Q_DECLARE_METATYPE(ifind::Image::Pointer)
Plugin_imageFileWriter::Plugin_imageFileWriter(QObject *parent) : Plugin(parent)
{
    this->OutputFolder = "";
    this->m_streamtype_to_write = ifind::InitialiseStreamTypeSetFromString(sDefaultStreamTypeToWrite);
    this->m_folder_policy = 0; /// folder by organ
    this->subdivide_folders = 0;
    this->first_subdivision = 0;
    this->mSaveImages = true;

    {
        // create widget
        WidgetType * mWidget_ = new WidgetType;

        QObject::connect(this, &Plugin_imageFileWriter::ImageToBeSaved,
                         mWidget_, &WidgetType::slot_imageWritten);
        QObject::connect(mWidget_->mCheckBoxSaveFiles, &QCheckBox::stateChanged,
                         this, &Plugin_imageFileWriter::slot_toggleSaveImages);

        this->mWidget = mWidget_;
    }
    this->SetDefaultArguments();
}

void Plugin_imageFileWriter::Initialize(void){
    Plugin::Initialize();
    ifind::Image::Pointer configuration = ifind::Image::New();
    configuration->SetMetaData<QString>("SavingImagesToFile_ON","True");
    this->mWidget->SetStreamTypes(this->m_streamtype_to_write);

    Q_EMIT this->ConfigurationGenerated(configuration);
}

void Plugin_imageFileWriter::slot_toggleSaveImages(bool b){
    //std::cout << "Plugin_imageFileWriter::slot_toggleSaveImages(bool b) "<< b <<std::endl;
    this->mSaveImages=b;
}

void Plugin_imageFileWriter::slot_imageReceived(ifind::Image::Pointer image){

    //std::cout << "Plugin_imageFileWriter::slot_imageReceived() : write image "<< image->GetMetaData<std::string>("DNLTimestamp") << std::endl;

    /// Send the image through the pipeline

    if (ifind::IsImageOfStreamTypeSet(image, m_streamtype_to_write))
    {
        if (image->HasKey("DO_NOT_WRITE") || this->mSaveImages==false){
            // image should not be written.
            //std::cout << "Plugin_imageFileWriter::slot_imageReceived() : Do not save"<<std::endl;

        } else {
            std::thread* writerthread = new std::thread(&Plugin_imageFileWriter::Write, this, image, false);
            writerthread->detach();

            Q_EMIT this->ImageToBeSaved(image);
        }
    }

    Q_EMIT this->ImageGenerated(image);
}

void Plugin_imageFileWriter::SetDefaultArguments(){
    this->RemoveArgument("showimage");
    this->RemoveArgument("layer");
    // arguments are defined with: name, placeholder for value, argument type,  description, default value
    mArguments.push_back({"folder", "<folder>",
                          QString( Plugin::ArgumentType[3] ),
                          "Parent folder to save images. Will be created if does not exist.",
                          QString(this->OutputFolder.c_str())});

    mArguments.push_back({"stream", "<stream name>",
                          QString( Plugin::ArgumentType[3] ),
                          "Write images only of a certain stream type, given as a string. If set to \"-\", it saves all.",
                          QString(sDefaultStreamTypeToWrite.c_str())});
    mArguments.push_back({"maxfiles", "<N>",
                          QString( Plugin::ArgumentType[1] ),
                          "Maximum number of images in a single folder, will create another folder if this is exceeded. If 0, all files in same folder.",
                          QString::number(this->subdivide_folders)});
    mArguments.push_back({"firstsubdivision", "<N>",
                          QString( Plugin::ArgumentType[1] ),
                          "Initial id for the subdivision folder.",
                          QString::number(this->first_subdivision)});

}

void Plugin_imageFileWriter::SetCommandLineArguments(int argc, char* argv[]){
    Plugin::SetCommandLineArguments(argc, argv);
    InputParser input(argc, argv, this->GetCompactPluginName().toLower().toStdString());

    {const std::string &argument = input.getCmdOption("folder");
        if (!argument.empty()){
            this->OutputFolder= argument.c_str();
        }}
    {const std::string &argument = input.getCmdOption("stream");
        if (!argument.empty()){
            this->m_streamtype_to_write = ifind::InitialiseStreamTypeSetFromString(argument.c_str());
        }}
    {const std::string &argument = input.getCmdOption("maxfiles");
        if (!argument.empty()){
            this->subdivide_folders= atoi(argument.c_str());
        }}
    {const std::string &argument = input.getCmdOption("firstsubdivision");
        if (!argument.empty()){
            this->first_subdivision= atoi(argument.c_str());
        }}
}


std::string Plugin_imageFileWriter::CreateFileName(ifind::Image::Pointer arg, unsigned int layer, bool withextension)
{
    std::ostringstream filename;
    filename << "image_transducer"
             << std::stoi(arg->GetMetaData<std::string>("TransducerNumber")) << "_"
             << std::stoi(arg->GetMetaData<std::string>("NDimensions")) << "D_"
             << arg->GetMetaData<std::string>("DNLTimestamp");

    if(layer >= 0){
        filename << "_layer" << layer;
    }

    if (withextension){
        filename << ".mhd";
    }

    return filename.str();
}

void Plugin_imageFileWriter::Write(ifind::Image::Pointer arg, bool headerOnly)
{

    /// TODO: make sure you don't save images from this plugin.
    QString streamfolder = QString::fromStdString(arg->GetMetaData<std::string>("StreamType"));
    QString subfolder;
    QString subfolder_("");
    if (arg->HasKey("Label")){
        subfolder_ = QString::fromStdString( arg->GetMetaData<std::string>("Label") );
    }

    if (this->subdivide_folders >0){
        if (this->write_count.find(subfolder_.toStdString())==this->write_count.end()){
            // first timer-
            std::array<int, 2> counts;
            counts.fill(0);
            counts[1] = this->first_subdivision;
            this->write_count[subfolder_.toStdString()] = counts;
        }
        auto counts = this->write_count[subfolder_.toStdString()];
        if (counts[0]++ > this->subdivide_folders){
            counts[0]=0; // number of frame
            counts[1]++; // number of subdivision"
        }

        subfolder  = subfolder_ + QString::number(counts[1]);
        this->write_count[subfolder_.toStdString()] = counts;
    } else {
        subfolder = subfolder_;
    }

    QDir sessiondirectory(QString::fromStdString(this->OutputFolder));
    QString dirname = sessiondirectory.filePath(streamfolder + QDir::separator() + subfolder);
    if (!sessiondirectory.mkpath(dirname))
    {
        qWarning() << "Plugin_imageFileWriter::Write() - Cannot create dir " << dirname;
        return;
    }

    this->m_Mutex.lock();

    //this->m_FileNames.clear();
    for(unsigned int l = 0; l < arg->GetNumberOfLayers(); l++)
    {
        /// extract name information
        std::string fname = this->CreateFileName(arg, l, true);
        std::string filename = dirname.toStdString() + std::string("/") + fname;
        //this->m_FileNames.push_back(filename);
        /// convert layer into ITK format
        ConverterType::Pointer converter;
        try {
            converter = ConverterType::New();
            converter->SetInput(arg->GetVTKImage(l));
            converter->Update();
        } catch( itk::ExceptionObject& ex ) {
            qDebug() << "Plugin_imageFileWriter::Write() - exception captured when converting formats";
            qDebug() << ex.what();
        }

        try {
            ifind::Image::Pointer layer = converter->GetOutput();
            layer->SetMetaDataDictionary(arg->GetMetaDataDictionary());
            /// write the image
            WriterType::Pointer writer = WriterType::New();
            writer->SetFileName(filename);
            writer->SetInput(layer);
            writer->Update();
        } catch( itk::ExceptionObject& ex ) {
            qDebug() << "Plugin_imageFileWriter::Write() - exception captured when writing to "<< filename.c_str();
            qDebug() << ex.what();
        }
    }

    this->m_Mutex.unlock();
}

QtPluginWidgetBase *Plugin_imageFileWriter::GetWidget(){
    return mWidget;
}

extern "C"
{
#ifdef WIN32
/// Function to return an instance of a new LiveCompoundingFilter object
__declspec(dllexport) Plugin* construct()
{
    return new Plugin_imageFileWriter();
}
#else
Plugin* construct()
{
    return new Plugin_imageFileWriter();
}
#endif // WIN32
}

