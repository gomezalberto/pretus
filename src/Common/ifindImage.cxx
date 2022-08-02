#include "ifindImage.h"

#include <vtkImageData.h>
#include <vtkMatrix4x4.h>


namespace ifind
{

Image::Image()
{
    this->m_Layers = std::vector<vtkSmartPointer<vtkImageData> >(0);
    this->m_LayerNames = std::vector< std::string> (0);
    ///// Initiaizing metadata
    this->SetMetaData<std::string>("ReorientMatrix", "1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1");
    this->SetMetaData<std::string>("TransducerMatrix", "1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1");
    this->SetMetaData<std::string>("CalibrationMatrix", "1 0 0 0 0 0 1 0 0 -1 0 0 0 0 0 1");
    this->SetMetaData<std::string>("TrackerMatrix", "1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1");
    this->SetMetaData<std::string>("NDimensions", "-1");
    this->SetMetaData<std::string>("ForceData", "0");
    this->SetMetaData<std::string>("LocalTimestamp", std::to_string(ifind::LocalTimeStamp()));
    this->SetMetaData<std::string>("FocusDepth", "0.0");
    this->SetMetaData<std::string>("TransducerNumber", "0");
    this->SetMetaData<std::string>("DepthOfScanField", "0");
    this->SetMetaData<std::string>("ForceTimestamp", "0");
    this->SetMetaData<std::string>("TrackerTimestamp", "0");
    this->SetMetaData<std::string>("TransducerTimestamp", "0");
    this->SetMetaData<std::string>("DNLTimestamp", "0");
    this->SetMetaData<std::string>("PatientName", "N/A");
    this->SetMetaData<std::string>("Debug", "0");
    this->SetMetaData<std::string>("ImageMode", "0");
    this->SetMetaData<std::string>("DNLLayerTimeTag", "0");
    this->SetMetaData<std::string>("SectorAngle", "0");
    this->SetMetaData<std::string>("EchoGain", "0");
    this->SetMetaData<std::string>("SectorWidth", "0");
    this->SetMetaData<std::string>("AcquisitionFrameRate", "0");
    this->SetMetaData<std::string>("TransmissionFrameRate", "0");
    this->SetMetaData<std::string>("FrameRate", "0");
    this->SetMetaData<std::string>("StreamType", "Input");
    this->SetMetaData<std::string>("StreamTypeHistory", "Input");
}

Image::StreamType Image::GetStreamType() 
{
    if (this->HasKey("StreamType")){
        return (this->GetMetaData<std::string>("StreamType"));
    }

    // just return empty if one is not found
    return StreamType();
};

void Image::SetStreamType(const Image::StreamType &st)
{
    // add to the history of streams
    std::string current_stream_type_history("Input");
    if (this->HasKey("StreamTypeHistory")){
        current_stream_type_history = this->GetMetaData<std::string>("StreamTypeHistory");
    }
    current_stream_type_history+=">"+st;
    this->SetMetaData<std::string>("StreamTypeHistory", current_stream_type_history);

    // Set the stream type
    //StreamType old_stream_type = this->GetMetaData<std::string>("StreamType"); - does nothing

    this->SetMetaData<std::string>("StreamType", st);
}

vtkSmartPointer<vtkMatrix4x4> Image::GetMetaDataMatrix (std::string key)
{
    vtkSmartPointer<vtkMatrix4x4> ret = vtkSmartPointer<vtkMatrix4x4>::New();
    std::istringstream is (this->GetMetaData<std::string>(key.c_str()));
    for (unsigned int i=0; i<4; i++)
        for (unsigned int j=0; j<4; j++)
        {
            double v = 0;
            is >> v;
            ret->SetElement(i, j, v);
        }
    return ret;
}

void Image::SetSpacingAllLAyers(const SpacingType &spacing){
    this->SetSpacing(spacing);
    int N = this->GetNumberOfLayers();
    for (int i=0; i<N; i++){
        if (this->GetOverlay(i) == nullptr){
            continue;
        }
        this->m_Converters[i]->GetInput()->SetSpacing(spacing);
    }
}

void Image::ShallowCopy (const Image *data){

    itk::MetaDataDictionary dic = data->GetMetaDataDictionary();
    Superclass::Graft(data);
    /// "Shallow" copy of overlays
    //this->GraftOverlay(data, 0);
    int N = data->GetNumberOfLayers();
    for (int i=0; i<N; i++){
        if (data->GetOverlay(i) == nullptr){
            continue;
        }
        std::string layername = data->GetLayerNames()[i];
        this->GraftOverlay(data->GetOverlay(i).GetPointer(), i, layername);
    }
    this->SetMetaDataDictionary(dic);
    // itk::MetaDataDictionary dic2 = this->GetMetaDataDictionary(); // this works, no crashes
}

void Image::ShallowMerge (const Image *data, bool mergeLayers){

    itk::MetaDataDictionary input_dic = data->GetMetaDataDictionary();
    //Superclass::Graft(data);
    if (mergeLayers){
        int N = data->GetNumberOfLayers();
        for (int i=0; i<N; i++){
            if (data->GetOverlay(i) == nullptr){
                continue;
            }
            std::string layername = data->GetLayerNames()[i];
            this->GraftOverlay(data->GetOverlay(i).GetPointer(), i, layername);
        }
    }
    //itk::MetaDataDictionary &this_dic = this->GetMetaDataDictionary(); // this works, no crashes
    itk::MetaDataDictionary::ConstIterator it = input_dic.Begin();
    while (it != input_dic.End())
    {
        std::stringstream ss;
        ss << it->second;
        this->SetMetaData<std::string>(it->first, ss.str());
        ++it;
    }

    //this->SetMetaDataDictionary(this_dic);

}

void Image::VeryShallowCopy (const Image *data){
    itk::MetaDataDictionary dic = this->GetMetaDataDictionary();
    Superclass::Graft(data);
    this->SetMetaDataDictionary(dic);
}

void Image::Graft(const Superclass *data, std::string layername)
{
    /// graft the input data but conserving the dictionary
    itk::MetaDataDictionary dic = this->GetMetaDataDictionary();
    Superclass::Graft(data);
    this->SetMetaDataDictionary(dic);
    this->GraftOverlay(data, 0, layername);
}


void Image::GraftOverlay (const Superclass *data, unsigned int index, std::string layername)
{
    if (index >= m_Converters.size())
    {
        for (unsigned int idx = m_Converters.size(); idx <= index; idx++)
            this->m_Converters.push_back(ConverterType::New());
    }
    /// convert the input data into vtk format
    ConverterType::Pointer converter = this->m_Converters[index];
    converter->SetInput(data);
    converter->Update();

    vtkSmartPointer<vtkImageData> vtkimage = converter->GetOutput();
    /// resize the layer array if necessary
    if (index >= m_Layers.size()){
        m_Layers.resize(index+1, nullptr);
        m_LayerNames.resize(index+1, "");
    }

    /// push the overlay in the array
    m_Layers[index] = vtkimage;
    m_LayerNames[index] = layername;
}

void Image::GraftOverlay(const vtkSmartPointer<vtkImageData> data, unsigned int index)
{
    /// resize the layer array if necessary
    if (index >= m_Layers.size())
        m_Layers.resize(index+1, nullptr);

    if (data == nullptr){
        m_Layers[index] = nullptr;
        return;
    }

    vtkSmartPointer<vtkImageData> vtkimage = vtkSmartPointer<vtkImageData>::New();
    vtkimage->ShallowCopy(data);
    /// push the overlay in the array
    m_Layers[index] = vtkimage;
}

vtkSmartPointer<vtkMatrix4x4> Image::GetTotalMatrix()
{
    vtkSmartPointer<vtkMatrix4x4> reorientMatrix = this->GetMetaDataMatrix("ReorientMatrix");
    vtkSmartPointer<vtkMatrix4x4> calibrationMatrix = this->GetMetaDataMatrix("CalibrationMatrix");
    vtkSmartPointer<vtkMatrix4x4> trackerMatrix = this->GetMetaDataMatrix("TrackerMatrix");
    vtkSmartPointer<vtkMatrix4x4> transducerMatrix = this->GetMetaDataMatrix("TransducerMatrix");

    vtkSmartPointer<vtkMatrix4x4> total = vtkSmartPointer<vtkMatrix4x4>::New();
    vtkSmartPointer<vtkMatrix4x4> Mimage = vtkSmartPointer<vtkMatrix4x4>::New();

    vtkMatrix4x4::Multiply4x4(calibrationMatrix, trackerMatrix, total);
    vtkMatrix4x4::Multiply4x4(total, transducerMatrix, total);
    vtkMatrix4x4::Multiply4x4(total, reorientMatrix, Mimage);

    return Mimage;
}


vtkSmartPointer<vtkImageData> Image::GetVTKImage(int layer) const
{
    if (this->m_Layers.size() <= layer){
        return nullptr;
    }
    return this->m_Layers[layer];
}


void Image::PrintSelf(std::ostream & os, itk::Indent indent) const
{
    itk::MetaDataDictionary dic = this->GetMetaDataDictionary();
    itk::MetaDataDictionary::ConstIterator it = dic.Begin();
    typedef itk::MetaDataObject< std::string > MetaDataStringType;
    os << indent << std::endl;
    os << indent << "************" << std::endl;
    os << indent << "  Meta-Data" << std::endl;
    os << indent << "************" << std::endl;
    os << indent << std::endl;
    while (it != dic.End())
    {
        itk::MetaDataObjectBase::Pointer  entry = it->second;
        MetaDataStringType::Pointer entryvalue = dynamic_cast< MetaDataStringType* >( entry.GetPointer() ) ;
        if( entryvalue )
        {
            std::string tagkey   = it->first;
            std::string tagvalue = entryvalue->GetMetaDataObjectValue();
            os << indent.GetNextIndent() << "[" << tagkey << "] " << tagvalue << std::endl;
        }
        else
        {
            std::string tagkey   = it->first;
            os << indent.GetNextIndent() << "[" << tagkey << "] " << "unknown value type" << std::endl;
        }
        ++it;
    }
    os << indent << std::endl;
    os << indent << "************" << std::endl;
    os << indent << "  itk::Image" << std::endl;
    os << indent << "************" << std::endl;
    os << indent << std::endl;

    this->Superclass::PrintSelf(os, indent.GetNextIndent());
}

} // end of namespace
