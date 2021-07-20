#include "BasicVTKVisualization.h"
#include <vtkAxisActor2D.h>
#include <vtkLegendScaleActor.h>
#include <QString>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkRenderer.h>
#include <vtkImageSliceMapper.h>
#include <vtkImageActor.h>
#include <vtkImageSlice.h>
#include <itkImageToVTKImageFilter.h>
#include <sstream>
#include <vtkJPEGWriter.h>
#include <vtkTextProperty.h>
#include <vtkRendererCollection.h>
#include <vtkCamera.h>
#include <ifindImagePeriodicTimer.h>
#include <vtkImageBlend.h>
#include <vtkImageMapToColors.h>
#include "vtkLookupTableGenerator.h"


BasicVTKVisualization::BasicVTKVisualization( QObject *parent){

    this->LatestTimeStamp = std::numeric_limits<uint64_t>::digits10 + 1;
    this->SliceExtractor = vtkSmartPointer<vtkExtractVOI>::New();
    this->corner_anotation = vtkSmartPointer<vtkCornerAnnotation>::New();
    this->ZSlice = 0;
    this->image_actor = nullptr;
    this->colorBar = nullptr;

    this->renderWindow = vtkSmartPointer<vtkRenderWindow>::New();

    this->biometrics = QList<float>({-1, -1, -1, -1});
    this->estimated_fetal_weight = -1.0;
}

void BasicVTKVisualization::Initialize(){

    this->SetupInteractor();
    this->SetGrayLookupTable(0);
    if (this->params.LutId < 0){
        this->SetJetLookupTable(1);
    } else {
        this->SetPredefinedLookupTable(1, this->params.LutId);
    }
}

void BasicVTKVisualization::slot_Terminate(void){
    std::cout << "Close visualization"<<std::endl;
    this->renderWindow->GetInteractor()->TerminateApp();
    this->renderWindow =NULL;
}

void BasicVTKVisualization::SetZSlice(int arg)
{
    if (this->ZSlice == arg)
        return;

    this->ZSlice = arg;
    this->ZSlice = std::max(-50, this->ZSlice);
    this->ZSlice = std::min( 50, this->ZSlice);


    vtkRenderer* renderer = this->renderWindow->GetRenderers()->GetFirstRenderer();
    if (!renderer)
        return;
    bool firstrender = true;
    vtkPropCollection* props = renderer->GetViewProps();
    props->InitTraversal();
    vtkProp* prop;
    while (prop = props->GetNextProp())
    {
        if (prop->IsA("vtkImageActor"))
        {
            firstrender = false;
            break;
        }
    }
    if (firstrender)
        return;

    vtkSmartPointer<vtkImageData> vtkimage = this->SliceExtractor->GetImageDataInput(0);
    if (!vtkimage)
        return;

    int* dims = vtkimage->GetDimensions();
    if (dims[2] <= 1)
        return;

    unsigned int slice = (unsigned int) ( (float)(dims[2]-1) * ( (float)(this->ZSlice) + 50.0 ) / 100.0 );
    this->SliceExtractor->SetVOI(0, dims[0]-1, 0, dims[1]-1, slice, slice);
    this->SliceExtractor->Update();
    renderer->ResetCameraClippingRange();
    this->renderWindow->Render();
}


void BasicVTKVisualization::SetViewToImageSize(){
    if (this->image_actor==nullptr){
        return;
    }
    vtkRenderer* renderer = this->renderWindow->GetRenderers()->GetFirstRenderer();
    vtkCamera* camera = renderer->GetActiveCamera();
    int extent[6];
    this->image_actor->GetMapper()->GetInput()->GetExtent(extent);
    double origin[3];
    this->image_actor->GetMapper()->GetInput()->GetOrigin(origin);
    double spacing[3];
    this->image_actor->GetMapper()->GetInput()->GetSpacing(spacing);

    double xc = origin[0] + 0.5*(extent[0] + extent[1])*spacing[0];
    double yc = origin[1] + 0.5*(extent[2] + extent[3])*spacing[1];
    //  float xd = (extent[1] - extent[0] + 1)*spacing[0]; // not used
    double yd = (extent[3] - extent[2] + 1)*spacing[1];

    double d = camera->GetDistance();
    camera->SetParallelScale(0.5f*static_cast<double>(yd));
    camera->SetFocalPoint(xc,yc,0.0);
    camera->SetPosition(xc,yc,-d);
    camera->SetViewUp(0, -1, 0);
}

void BasicVTKVisualization::SendImageToWidget(ifind::Image::Pointer image){

    /// verify that SetupInteractor() had been called
    vtkRenderer* renderer = this->renderWindow->GetRenderers()->GetFirstRenderer();
    if (!renderer)
    {
        std::cerr << "[BasicVTKVisualization] error: no renderer in vtkRenderWindow";
        return;
    }

    // Combine the images (blend takes multiple connections on the 0th input port)
    vtkSmartPointer<vtkImageBlend> blend = vtkSmartPointer<vtkImageBlend>::New();
    unsigned int upperlimit = this->params.DisplayMultiLayers!=0 ? image->GetNumberOfLayers() : 1;

    std::vector<int> standardLayerOrder {0,1,2,3,4,5};
    std::vector<int> layerOrder;
    std::set_difference(
        standardLayerOrder.begin(), 
        standardLayerOrder.end(), 
        &this->params.base_layer, 
        &this->params.base_layer+1,
        std::inserter(layerOrder, layerOrder.begin()));
    layerOrder.insert(layerOrder.begin(), this->params.base_layer);

    if (this->params.DisplayMultiLayers>0){
        /// show only one layer, as overlay, selected by the user
        layerOrder.clear();
        layerOrder.push_back(this->params.base_layer);
        layerOrder.push_back(this->params.DisplayMultiLayers);
        upperlimit = 2;
    }

    for (unsigned int idx=0; idx < upperlimit; idx++)
    {
        if (idx < this->LookupTables.length())
        {
            vtkSmartPointer<vtkImageMapToColors> mapper = vtkSmartPointer<vtkImageMapToColors>::New();
            mapper->PassAlphaToOutputOn();
            mapper->SetOutputFormatToRGBA();
            mapper->SetLookupTable(this->LookupTables.at(idx));
            vtkImageData *layerImage = image->GetVTKImage(layerOrder[idx]);
            if (layerImage == nullptr){
                continue;
            }
            mapper->SetInputData(layerImage);
            blend->AddInputConnection(mapper->GetOutputPort());

            // add colorbar if not already added
            if (this->colorBar == nullptr && idx>0 && this->params.showcolormap ){
                this->colorBar = vtkSmartPointer<vtkScalarBarActor>::New();
                this->colorBar->SetLookupTable(this->LookupTables.at(idx));
                this->colorBar->SetTitle("Attention");
                this->colorBar->SetNumberOfLabels(this->LookupTables.at(idx)->GetNumberOfTableValues()+1);
                this->colorBar->SetWidth(0.1);
                this->colorBar->SetHeight(0.8);
                //this->colorBar->SetUnconstrainedFontSize(true);
                //this->colorBar->GetLabelTextProperty()->SetFontSize(24);

                renderer->AddActor2D(this->colorBar);
            }

        }
        else
            blend->AddInputData(image->GetVTKImage(layerOrder[idx]));

        blend->SetOpacity(idx, idx == 0 ? 1. : .5);
    }
    blend->Update();
    vtkSmartPointer<vtkImageData> vtkimage = blend->GetOutput();

    if ( !vtkimage )
        return;

    /// if the received image is 3D, display the midslice in the z direction
    if ( std::stoi(image->GetMetaData<std::string>("ImageMode")) != ifind::Image::ImageMode::TwoD )
    {
        int* dims = vtkimage->GetDimensions();
        unsigned int slice = (unsigned int) ( (float)(dims[2]-1) * ( (float)(this->ZSlice) + 50.0 ) / 100.0 );
        this->SliceExtractor->SetInputData(vtkimage);
        this->SliceExtractor->SetVOI(0, dims[0]-1, 0, dims[1]-1, slice, slice);
        this->SliceExtractor->Update();
        vtkimage = this->SliceExtractor->GetOutput();
    }
    else if (this->ZSlice != 0)
    {
        this->ZSlice = 0;
        Q_EMIT ZSliceChanged(this->ZSlice);
        renderer->ResetCameraClippingRange();
    }

    bool firstrender = true;
    vtkPropCollection* props = renderer->GetViewProps();
    props->InitTraversal();
    vtkProp* prop;
    while (prop = props->GetNextProp())
    {
        if (prop->IsA("vtkImageActor"))
        {
            firstrender = false;
            break;
        }
    }

    if (firstrender)
    {
        /// Make sure the image fills the screen
        this->renderWindow->SetSize(this->renderWindow->GetScreenSize()); // set to full screen
        /// first render, add the vtkImageActor in the renderer
        vtkSmartPointer<vtkImageSliceMapper> newmapper = vtkSmartPointer<vtkImageSliceMapper>::New();
        newmapper->SetInputData(vtkimage);
        this->image_actor = vtkSmartPointer<vtkImageActor>::New();
        this->image_actor->SetInterpolate(0);
        this->image_actor->SetMapper(newmapper);
        renderer->AddViewProp(this->image_actor);
        renderer->ResetCamera();
        renderer->GetActiveCamera()->ParallelProjectionOn();

        /// Do this for all images, in case the depth changes
        this->SetViewToImageSize();


        if (this->params.showruler){

            vtkSmartPointer<vtkLegendScaleActor> axisActor = vtkSmartPointer<vtkLegendScaleActor>::New();
            axisActor->SetLabelModeToDistance();
            axisActor->SetTopAxisVisibility(false);
            axisActor->SetLeftAxisVisibility(false);
            axisActor->SetBottomAxisVisibility(false);
            //axisActor->SetRightBorderOffset(-10);
            axisActor->LegendVisibilityOff();
            vtkSmartPointer<vtkAxisActor2D> raxisActor = axisActor->GetRightAxis();
            raxisActor->SetNumberOfMinorTicks(3);
            raxisActor->SetNumberOfLabels(9);
            renderer->AddViewProp(axisActor);
        }
        this->renderWindow->GetInteractor()->Initialize(); /// @todo understand what is going on here. It does not allow me to interact!
    }
    else
    {
        // update the input for the vtk render engine
        vtkImageActor* actor = reinterpret_cast<vtkImageActor*> (prop);
        actor->GetMapper()->SetInputData(vtkimage);
        this->SetViewToImageSize();
        ///    @todo reset the camera when the depth or width have changed since the last image:
        ///    if (this->HasFieldOfViewChanged(image))
        ///      renderer->ResetCamera();
    }

    /// update the corner information with ifind::Image metadata
    this->UpdateCornerAnnotation(image);
    /// render the scene
    this->renderWindow->Render();
}

void BasicVTKVisualization::SetupInteractor(){

    vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    this->renderWindow->SetInteractor(iren);
    vtkSmartPointer<vtkInteractorStyleImage> style = vtkSmartPointer<vtkInteractorStyleImage>::New();
    this->renderWindow->GetInteractor()->SetInteractorStyle(style);

    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    this->renderWindow->AddRenderer(renderer);

    /// setup the corner annotation
    corner_anotation->SetLinearFontScaleFactor(1);
    corner_anotation->SetNonlinearFontScaleFactor(1);
    corner_anotation->SetMinimumFontSize(2);
    corner_anotation->SetMaximumFontSize(20);
    corner_anotation->GetTextProperty()->SetColor(1, 1, 1);
    renderer->AddViewProp(corner_anotation);

}

void BasicVTKVisualization::UpdateCornerAnnotation(ifind::Image::Pointer image)
{
    std::stringstream lowerleft;
    std::stringstream lowerright;
    std::stringstream topleft;
    std::stringstream topright;
    /// query metadata
    uint64_t t_dnl = std::atol(image->GetMetaData<std::string>("DNLTimestamp").c_str());
    lowerleft << "Timestamp DNL: " << t_dnl;

    std::string patientname = image->GetMetaData<std::string>("PatientName");
    lowerright << "Patient name: " << patientname;

    QStringList acq_framerates = QString(image->GetMetaData<std::string>("AcquisitionFrameRate").c_str()).split(" ");
    QStringList trans_framerates = QString(image->GetMetaData<std::string>("TransmissionFrameRate").c_str()).split(" ");

    /// estimate frame rate using this->LatestTimeStamp (if first image then --> 0)
    unsigned int rec_framerate = std::stoul(image->GetMetaData<std::string>("FrameRate"));
    /// update the current this->LatestTimeStamp
    this->LatestTimeStamp = t_dnl;

    topleft << "Frame rate (Hz): ";
    if (acq_framerates.size() && trans_framerates.size())
        topleft << "(" << acq_framerates[0].toStdString() << "/" << trans_framerates[0].toStdString() << ") - " << rec_framerate;
    topleft << std::endl;

    unsigned int depthofscanfield = std::stoi(image->GetMetaData<std::string>("DepthOfScanField"));
    topleft << "Depth of scan field: " << depthofscanfield << std::endl;
    float focusdepth = std::stof(image->GetMetaData<std::string>("FocusDepth"));
    topleft << "Focus depth: " << focusdepth << std::endl;
    QStringList widths = QString(image->GetMetaData<std::string>("SectorWidth").c_str()).split(" ");
    if (widths.size())
        topleft << "Sector width: " << widths[0].toStdString()<<std::endl;

    int dimensions = std::stoi(image->GetMetaData<std::string>("NDimensions"));

    int nlayers = 1;
    if (image->HasKey("NumberOfLayersInGeometry"))
        nlayers = std::stoi(image->GetMetaData<std::string>("NumberOfLayersInGeometry"));
    int datatype = 1;
    if (image->HasKey("LayerDataType"))
        datatype = std::stoi(image->GetMetaData<std::string>(nlayers > 1 ? "LayerDataType_1" : "LayerDataType"));

    std::string datatypestring = "";
    switch (datatype)
    {
    case 1:
        datatypestring = "Echo";
        break;
    case 2:
        datatypestring = "Color-Flow-Power";
        break;
    case 3:
        datatypestring = "Color-Flow-Velocity";
        break;
    case 4:
        datatypestring = "Color-Flow-Variance";
        break;
    case 5:
        datatypestring = "PW-Spectral-Doppler";
        break;
    case 6:
        datatypestring = "CW-Spectral-Doppler";
        break;
    default:
        datatypestring = "unknown-type";
        break;
    }
    topleft  << "Mode: " << dimensions << "D " << datatypestring << std::endl;
    if (image->HasKey("TransducerData")) topleft << "Transducer: "<< image->GetMetaData<std::string>("TransducerData");

    topright << "Number of layers: " << image->GetNumberOfLayers()<<std::endl;

    /// If the organ classifier was up and running display the result
    if (image->HasKey("Label_confidence")){
        auto cc = image->GetMetaData<std::string>("Label_confidence");
        if (image->HasKey("Label")) topright<< std::endl << "Organ: " << image->GetMetaData<std::string>("Label") <<std::endl;
        double confidence;
        { /// this little hack is necessary because although we type things as double, when saving/reading from file they will be strings.
        bool isvalid;
        confidence = image->GetMetaData<double>("Label_confidence",isvalid);
        if (!isvalid){
            std::string confidence_str = image->GetMetaData<std::string>("Label_confidence");
            //confidence = std::stof(confidence_str);
            /// @todo hack does not work...
        }
        topright << "Confidence: " <<  setprecision(2) << setw(2) << std::fixed << confidence <<std::endl;
        }
    }

    /// If the plane detector was up and running display the result
    if (image->HasKey("StandardPlanes")){
        std::string standard_planes_ = image->GetMetaData<std::string>("StandardPlanes");
        /// Transform to a list of strings
        QStringList standard_planes = QString::fromStdString(standard_planes_).split(",");

        std::string standard_planesc_ = image->GetMetaData<std::string >("StandardPlanes_confidences");
        QStringList standard_planesc = QString::fromStdString(standard_planesc_).split(",");

        int bigger = -1;
        float max_c = 0;
        for (int ii=0; ii<standard_planes.size(); ii++){
            float standard_plane_confidence = standard_planesc[ii].toFloat();
            if (standard_plane_confidence > max_c){
                max_c = standard_plane_confidence;
                bigger = ii;
            }
        }
        topright << std::endl;
        for (int ii=0; ii<standard_planes.size(); ii++){
            topright << setprecision(2) << setw(2) << std::fixed;
            if (ii==bigger){
                topright <<  "===>  ";
            }
            topright << standard_planes[ii].toStdString()<<": "<< standard_planesc[ii].toFloat() <<std::endl;
        }


    }

    if (image->HasKey("StandardPlanes_cluster")){

        std::string cluster = image->GetMetaData<std::string>("StandardPlanes_cluster");
        topright << std::endl;
        topright << "Cluster: "<< cluster << std::fixed;
    }

    /// If the biometric estimator was up and running display the result
    if (image->HasKey("Biometrics_plane"))
    {
        std::string plane = image->GetMetaData<std::string>("Biometrics_plane");

        float dice = -1;
        float circumference;
        float shortaxis;
        float longaxis;
        if (image->HasKey("Biometrics_dice"))
        {
            dice = image->GetMetaData<float>("Biometrics_dice");
            circumference = image->GetMetaData<float>("Biometrics_circumference(mm)");
            shortaxis = image->GetMetaData<float>("Biometrics_shortaxis(mm)");
            longaxis = image->GetMetaData<float>("Biometrics_longaxis(mm)");

            if (plane.find("Abdominal") != std::string::npos)
            {
                this->biometrics[0] = circumference;
            }
            else if (plane.find("Brain (Tv.)") != std::string::npos)
            {
                this->biometrics[1] = circumference;
                this->biometrics[2] = shortaxis;
            }
            else if (plane.find("Femur") != std::string::npos)
            {
                this->biometrics[3] = longaxis;
            }
            if (!this->biometrics.contains(-1))
            {
                // haddlock1 Log10BW=1.326–0.0000326 (ACxFL) × 0.00107(HC) + 0.00438 (AC) + 0.0158(FL)
                // haddlock (E&J) EFW (g) = 10^ (1.326 - 0.00326 * AC * FL + 0.0107 * HC + 0.0438 * AC + 0.158 * FL)4
                double temp = 1.326 - 0.00326 * this->biometrics[0]/10 * this->biometrics[3]/10 + 0.0107 * this->biometrics[1]/10 + 0.0438 * this->biometrics[0]/10 + 0.158 * this->biometrics[3]/10;
                this->estimated_fetal_weight = std::pow(10, temp);
            }
        }

        topright << std::endl;
        topright << setprecision(3) << setw(2) << std::fixed;
        topright << plane << "\t (" << dice << ")"  << std::endl;
        topright << "AC: " << this->biometrics[0] << " mm" << std::endl;
        topright << "HC: " << this->biometrics[1] << " mm" << std::endl;
        topright << "BPD: " << this->biometrics[2] << " mm" << std::endl;
        topright << "FL: " << this->biometrics[3] << " mm" << std::endl;
        topright << "EFW: " << this->estimated_fetal_weight << " g" << std::endl;

    }

    /// update the text of the corner annotation
    this->corner_anotation->SetText(0, lowerleft.str().c_str());
    this->corner_anotation->SetText(1, lowerright.str().c_str());
    this->corner_anotation->SetText(2, topleft.str().c_str());
    this->corner_anotation->SetText(3, topright.str().c_str());
}

void BasicVTKVisualization::SetPredefinedLookupTable(int id, int LUTid){
    vtkLookupTableGenerator* lg = new vtkLookupTableGenerator();
    vtkSmartPointer<vtkLookupTable> lut = lg->GenerateLUT(LUTid);
    lut->IndexedLookupOff();
    lut->SetTableRange(0,255);
    double rgba[4];
    double Ncolors = lut->GetNumberOfTableValues();
    for (int i=0; i< int(Ncolors); i++){
        lut->GetTableValue(i,&(rgba[0]));
        if (i < Ncolors/2.0){
            double alpha = double(i)/Ncolors;
            rgba[3]=alpha * alpha;
        } else {
            rgba[3]=double(i)/Ncolors;
        }
        lut->SetTableValue(i,rgba);
    }
    // lut->Build();
    //std::cout << "Lut" << std::endl;
    //lut->Print(std::cout);

    this->SetLookupTable(id,lut);
    delete(lg);
}

void BasicVTKVisualization::SetGrayLookupTable(int id){
    int NValues=256;
    vtkSmartPointer<vtkLookupTable> t = vtkSmartPointer<vtkLookupTable>::New();
    t->SetNumberOfTableValues(NValues);
    t->SetRampToLinear();
    t->SetScaleToLinear();
    t->SetTableRange(0,255);
    for(int i = 0; i < NValues; i++){
        double val = static_cast<double>(i)/(static_cast<double>(NValues-1));
        t->SetTableValue(i, val, val, val, 1.0);
    }
    t->Build();
    this->SetLookupTable(id,t);
}

void BasicVTKVisualization::SetJetLookupTable(int id){
    int NValues=256;
    vtkSmartPointer<vtkLookupTable> t = vtkSmartPointer<vtkLookupTable>::New();
    //t->SetNumberOfTableValues(NValues);
    t->SetNumberOfTableValues(NValues);
    t->SetRampToLinear();
    t->SetScaleToLinear();
    t->SetRange(0,255);
    t->SetTableRange(0,255);
    t->SetAlpha(1.0);
    /*

    t->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
    for (int i=1; i<NValues; i++){
        t->SetTableValue(i, float(i)/float(NValues), 1.0, float(NValues-i)/float(NValues), float(i)/float(NValues));
    }
    */

    t->Build();
    this->SetLookupTable(id,t);
}
