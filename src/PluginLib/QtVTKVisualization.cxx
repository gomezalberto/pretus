#include "QtVTKVisualization.h"

#include <vtkAxisActor2D.h>
#include <vtkLegendScaleActor.h>
#include <vtkGenericRenderWindowInteractor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkRenderer.h>
#include <vtkImageSliceMapper.h>
#include <vtkImageActor.h>
#include <vtkRendererCollection.h>
#include <vtkCamera.h>
#include <vtkImageBlend.h>
#include <vtkImageMapToColors.h>
#include <vtkNew.h>

#include <QVTKWidget.h>

#include <QResizeEvent>

#include "vtkLookupTableGenerator.h"
#include "VisualizationConsts.h"

static const bool sDefaultParametersShowRuler(false);
static const bool sDefaultParametersShowColorbar(false);
static const int sDefaultParametersBaseLayer(0);
static const int sDefaultOverlayLayer(0); // by default, only display one layer
static const int sDefaultParametersLutId(15);

QtVTKVisualization::Parameters::Parameters()
    : mShowRuler(sDefaultParametersShowRuler)
    , mShowColorbar(sDefaultParametersShowColorbar)
    , mBaseLayer(sDefaultParametersBaseLayer)
    , mOverlayLayer(sDefaultOverlayLayer)
    , mLutId(sDefaultParametersLutId)
{}

const bool QtVTKVisualization::Parameters::ShowRuler() const {
    return mShowRuler;
}

const bool QtVTKVisualization::Parameters::ShowColorbar() const {
    return mShowColorbar;
}

const int QtVTKVisualization::Parameters::BaseLayer() const {
    return mBaseLayer;
}

const int QtVTKVisualization::Parameters::OverlayLayer() const {
    return mOverlayLayer;
}

const int QtVTKVisualization::Parameters::LutId() const {
    return mLutId;
}

QVTKWidget *QtVTKVisualization::GetQVTKWidget(){
    return this->vtkWidget;
}

void QtVTKVisualization::Parameters::Usage()
{
    std::cout << "\t"
        << "\t-vs_showruler <0/1>\tWhether to show (1) or not (0) a ruler. Default: "
        << sDefaultParametersShowRuler << std::endl;
    std::cout << "\t"
        << "\t-vs_layer <number of layer>\tDisplay (if available) the requested layer. Default: "
        << sDefaultParametersBaseLayer << std::endl;
    std::cout << "\t"
        << "\t-vs_overlay <num>\tdesactivate (0) or activate (1,... n_overlays)  displaying multiple layers as overlay. If -1, display all. Default: "
        << sDefaultOverlayLayer << std::endl;
    std::cout << "\t"
        << "\t-vs_lut <num>\tLookup table (int) (0: SPECTRUM, 1: WARM, 2: COOL, 3: BLUES, 4: WILD_FLOWER, 5: CITRUS, etc). Default: "
        << sDefaultParametersLutId << std::endl;
}


void QtVTKVisualization::Parameters::SetBaseLayer(int a){
    this->mBaseLayer = a;
}

void QtVTKVisualization::Parameters::SetOverlayLayer(int a){
    this->mOverlayLayer = a;
}

void QtVTKVisualization::Parameters::SetLutId(int a){
    this->mLutId = a;
}

void QtVTKVisualization::Parameters::SetShowColorbar(bool a){
    this->mShowColorbar = a;
}

void QtVTKVisualization::Parameters::SetCommandLineArguments(const std::vector<std::string> &args) {

    for (auto argIter = args.cbegin(); argIter != args.cend(); ++argIter)
    {
        if (*argIter == "-vs_showruler")
        {
            mShowRuler = std::stoi(*(++argIter));
        }
        else if (*argIter == "-vs_layer")
        {
            mBaseLayer = std::stoi(*(++argIter));
        }
        else if (*argIter == "-vs_overlay")
        {
            mOverlayLayer = std::stoi(*(++argIter));
        }
        else if (*argIter == "-vs_lut")
        {
            mLutId = std::stoi(*(++argIter));
        }
    }
}


QtVTKVisualization::QtVTKVisualization(QWidget *parent)
    :
      QtPluginWidgetBase(parent),
      vtkWidget(nullptr),
      renderWindow(nullptr),
      lookupTables(),
      sliceExtractor(vtkSmartPointer<vtkExtractVOI>::New()),
      imageActor(nullptr),
      colorBar(nullptr),
      latestTimeStamp(std::numeric_limits<uint64_t>::digits10 + 1),
      zSlice(0)

{
    this->vtkWidget = new QVTKWidget(this);
    vtkWidget->resize(256, 256);
    this->renderWindow = this->vtkWidget->GetRenderWindow();
    this->mViewScale = 0.5;
}

void QtVTKVisualization::Initialize()
{
    this->SetupInteractor();
    this->SetGrayLookupTable(0);
    if (this->mParams.LutId() < 0){
        this->SetJetLookupTable(1);
    } else {
        this->SetPredefinedLookupTable(1, this->mParams.LutId());
    }
}

const QtVTKVisualization::Parameters & QtVTKVisualization::Params() const
{
    return mParams;
}

void QtVTKVisualization::SetParams(const Parameters &params)
{
    mParams = params;
}

void QtVTKVisualization::slot_Terminate(void)
{
    std::cout << "Close visualization"<<std::endl;
    this->renderWindow->GetInteractor()->TerminateApp();
    this->renderWindow =NULL;
}

void QtVTKVisualization::SendImageToWidgetImpl(ifind::Image::Pointer image)
{
    // verify that SetupInteractor() had been called
    auto renderer = this->renderWindow->GetRenderers()->GetFirstRenderer();
    if (!renderer)
    {
        std::cerr << "[QtVTKVisualization] error: no renderer in vtkRenderWindow"<<std::endl;
        return;
    }

    // check that we have a valid layer
    if (!ifind::IsImageOfStreamTypeSet(image, this->StreamTypes()))
    {
        return;
    }

    // Combine the images (blend takes multiple connections on the 0th input port)
    std::vector<int> layerOrder;

    // This is what the code was before
    //std::set_difference(
    //    VisualizationConsts::StandardLayerOrder().begin(),
    //    VisualizationConsts::StandardLayerOrder().end(),
    //    &this->mParams.BaseLayer(), // WTF! This is supposed to be an iterator
    //    &this->mParams.BaseLayer()+1, // and its end FFS! Unbeliveable
    //    std::inserter(layerOrder, layerOrder.begin()));
    //

    int baselayer = mParams.BaseLayer() >= 0 ? mParams.BaseLayer()  : image->GetNumberOfLayers() + mParams.BaseLayer();

    std::vector<int> baseLayers{ baselayer };
    std::set_difference(
                VisualizationConsts::StandardLayerOrder().begin(),
                VisualizationConsts::StandardLayerOrder().end(),
                baseLayers.begin(),
                baseLayers.end(),
                std::inserter(layerOrder, layerOrder.begin()));

    layerOrder.insert(layerOrder.begin(), baselayer); // This will always have 6 elements

    unsigned int upperlimit = this->mParams.OverlayLayer() != 0 ? image->GetNumberOfLayers() : 1;

    if (this->mParams.OverlayLayer() != 0){
        /// show only one layer, as overlay, selected by the user
        layerOrder.clear();
        layerOrder.push_back(baselayer);
        if (this->mParams.OverlayLayer() <0){
            layerOrder.push_back(image->GetNumberOfLayers() + this->mParams.OverlayLayer());
        } else {
            layerOrder.push_back(this->mParams.OverlayLayer());
        }
        upperlimit = 2;
    }

    vtkNew<vtkImageBlend> imageBlender;

    for (unsigned int idx=0; idx < upperlimit; idx++)
    {
        if (idx < this->lookupTables.length())
        {
            vtkNew<vtkImageMapToColors> mapper;
            mapper->PassAlphaToOutputOn();
            mapper->SetOutputFormatToRGBA();
            mapper->SetLookupTable(this->lookupTables.at(idx));

            auto layerImage = image->GetVTKImage(layerOrder[idx]);
            if (layerImage == nullptr){
                continue;
            }

            mapper->SetInputData(layerImage);
            imageBlender->AddInputConnection(mapper->GetOutputPort());

            // add colorbar if not already added
            if (this->Params().ShowColorbar() == true){
                if (this->colorBar == nullptr && idx>0)
                {
                    this->colorBar = vtkSmartPointer<vtkScalarBarActor>::New();
                    this->colorBar->SetLookupTable(this->lookupTables.at(idx));
                    this->colorBar->SetTitle("Attention");
                    this->colorBar->SetNumberOfLabels(this->lookupTables.at(idx)->GetNumberOfTableValues()+1);
                    this->colorBar->SetWidth(0.1);
                    this->colorBar->SetHeight(0.8);
                    //this->colorBar->SetUnconstrainedFontSize(true);
                    //this->colorBar->GetLabelTextProperty()->SetFontSize(24);

                    renderer->AddActor2D(this->colorBar);
                }
            }
        }
        else
        {
            imageBlender->AddInputData(image->GetVTKImage(layerOrder[idx]));
        }

        imageBlender->SetOpacity(idx, idx == 0 ? 1.0 : 0.5);
    }

    imageBlender->Update();

    vtkSmartPointer<vtkImageData> blendedImage = imageBlender->GetOutput();
    if (!blendedImage) {
        return;
    }

    /// if the received image is 3D, display the midslice in the z direction
    if ( std::stoi(image->GetMetaData<std::string>("ImageMode")) != ifind::Image::ImageMode::TwoD )
    {
        int* dims = blendedImage->GetDimensions();
        unsigned int slice = (unsigned int) ( (float)(dims[2]-1) * ( (float)(this->zSlice) + 50.0 ) / 100.0 );
        this->sliceExtractor->SetInputData(blendedImage);
        this->sliceExtractor->SetVOI(0, dims[0]-1, 0, dims[1]-1, slice, slice);
        this->sliceExtractor->Update();
        blendedImage = this->sliceExtractor->GetOutput();
    }
    else if (this->zSlice != 0)
    {
        this->zSlice = 0;
        Q_EMIT ZSliceChanged(this->zSlice);
        renderer->ResetCameraClippingRange();
    }

    //bool firstrender = true;
    //vtkPropCollection* props = renderer->GetViewProps();
    //props->InitTraversal();
    //vtkProp* prop;
    //while (prop = props->GetNextProp())
    //{
    //    if (prop->IsA("vtkImageActor"))
    //    {
    //        firstrender = false;
    //        break;
    //    }
    //}

    if (nullptr == this->imageActor)
    {
        /// Make sure the image fills the screen
        /// first render, add the vtkImageActor in the renderer
        vtkNew<vtkImageSliceMapper> mapper;
        mapper->SetInputData(blendedImage);

        this->imageActor = vtkSmartPointer<vtkImageActor>::New();
        this->imageActor->SetInterpolate(0);
        this->imageActor->SetMapper(mapper.GetPointer());
        
        renderer->AddViewProp(this->imageActor);
        renderer->ResetCamera();
        renderer->GetActiveCamera()->ParallelProjectionOn();

        /// Do this for all images, in case the depth changes
        this->SetViewToImageSize();

        if (this->mParams.ShowRuler())
        {
            vtkNew<vtkLegendScaleActor> axisActor;
            axisActor->SetLabelModeToDistance();
            axisActor->SetTopAxisVisibility(false);
            axisActor->SetLeftAxisVisibility(false);
            axisActor->SetBottomAxisVisibility(false);
            //axisActor->SetRightBorderOffset(-10);
            axisActor->LegendVisibilityOff();

            auto raxisActor = axisActor->GetRightAxis();
            raxisActor->SetNumberOfMinorTicks(3);
            raxisActor->SetNumberOfLabels(9);

            renderer->AddViewProp(axisActor.GetPointer());
        }

        this->renderWindow->GetInteractor()->Initialize(); /// @todo understand what is going on here. It does not allow me to interact!
    }
    else
    {
        // update the input for the vtk render engine
        //vtkImageActor* actor = reinterpret_cast<vtkImageActor*> (prop);
        this->imageActor->GetMapper()->SetInputData(blendedImage);
        this->SetViewToImageSize();
        ///    @todo reset the camera when the depth or width have changed since the last image:
        ///    if (this->HasFieldOfViewChanged(image))
        ///      renderer->ResetCamera();
    }

    /// render the scene
    this->renderWindow->Render();
}

void QtVTKVisualization::SetViewScale(float viewScale){

    mViewScale = viewScale;
}

void QtVTKVisualization::SetZSlice(int newZSlice)
{
    if (this->zSlice == newZSlice) {
        return;
    }

    this->zSlice = newZSlice;
    this->zSlice = std::max(-50, this->zSlice);
    this->zSlice = std::min(50, this->zSlice);


    auto renderer = this->renderWindow->GetRenderers()->GetFirstRenderer();
    if (!renderer) {
        return;
    }

    //bool firstrender = true;
    //auto props = renderer->GetViewProps();
    //props->InitTraversal();
    //vtkProp* prop;
    //while (prop = props->GetNextProp())
    //{
    //    if (prop->IsA("vtkImageActor"))
    //    {
    //        firstrender = false;
    //        break;
    //    }
    //}

    //if (firstrender) {
    if (nullptr == this->imageActor) {
        return;
    }

    auto vtkimage = this->sliceExtractor->GetImageDataInput(0);

    if (!vtkimage) {
        return;
    }

    int* dims = vtkimage->GetDimensions();

    if (dims[2] <= 1) {
        return;
    }

    unsigned int slice = (unsigned int)((float)(dims[2] - 1) * ((float)(this->zSlice) + 50.0) / 100.0);
    this->sliceExtractor->SetVOI(0, dims[0] - 1, 0, dims[1] - 1, slice, slice);
    this->sliceExtractor->Update();
    renderer->ResetCameraClippingRange();
    this->renderWindow->Render();
}

void QtVTKVisualization::SetupInteractor()
{
    vtkNew<vtkGenericRenderWindowInteractor> iren;
    this->renderWindow->SetInteractor(iren.GetPointer());
    vtkNew<vtkInteractorStyleImage> style;
    this->renderWindow->GetInteractor()->SetInteractorStyle(style.GetPointer());

    vtkNew<vtkRenderer> renderer;
    this->renderWindow->AddRenderer(renderer.GetPointer());
}

void QtVTKVisualization::resizeEvent(QResizeEvent *event)
{
    vtkWidget->resize(event->size().width(), event->size().height());
}

void QtVTKVisualization::SetGrayLookupTable(int id)
{
    int NValues=256;
    vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();
    lookupTable->SetNumberOfTableValues(NValues);
    lookupTable->SetRampToLinear();
    lookupTable->SetScaleToLinear();
    lookupTable->SetTableRange(0,255);
    for(int i = 0; i < NValues; i++){
        double val = static_cast<double>(i)/(static_cast<double>(NValues-1));
        lookupTable->SetTableValue(i, val, val, val, 1.0);
    }
    lookupTable->Build();
    this->SetLookupTable(id,lookupTable);
}

void QtVTKVisualization::SetJetLookupTable(int id)
{
    int NValues=256;
    vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();
    //lookupTable->SetNumberOfTableValues(NValues);
    lookupTable->SetNumberOfTableValues(NValues);
    lookupTable->SetRampToLinear();
    lookupTable->SetScaleToLinear();
    lookupTable->SetRange(0,255);
    lookupTable->SetTableRange(0,255);
    lookupTable->SetAlpha(1.0);
    /*

    lookupTable->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
    for (int i=1; i<NValues; i++){
        lookupTable->SetTableValue(i, float(i)/float(NValues), 1.0, float(NValues-i)/float(NValues), float(i)/float(NValues));
    }
    */

    lookupTable->Build();
    this->SetLookupTable(id,lookupTable);
}

void QtVTKVisualization::SetPredefinedLookupTable(int id, int LUTid)
{
    vtkLookupTableGenerator lookupTableGenerator;
    vtkSmartPointer<vtkLookupTable> lookupTable = lookupTableGenerator.GenerateLUT(LUTid);
    lookupTable->IndexedLookupOff();
    lookupTable->SetTableRange(0, 255);
    double rgba[4];
    double Ncolors = lookupTable->GetNumberOfTableValues();
    for (int i = 0; i< int(Ncolors); i++) {
        lookupTable->GetTableValue(i, &(rgba[0]));
        if (i < Ncolors / 2.0) {
            double alpha = double(i) / Ncolors;
            rgba[3] = alpha * alpha;
        }
        else {
            rgba[3] = double(i) / Ncolors;
        }
        lookupTable->SetTableValue(i, rgba);
    }
    // lookupTable->Build();
    //std::cout << "Lut" << std::endl;
    //lookupTable->Print(std::cout);

    this->SetLookupTable(id, lookupTable);
}

void QtVTKVisualization::SetLookupTable(
        unsigned int idx,
        vtkSmartPointer<vtkLookupTable> lookupTable)
{
    if (idx != this->lookupTables.length())
    {
        std::cout << "Cannot append lookup table at idx " << idx << std::endl;
        return;
    }
    this->lookupTables.append(lookupTable);
}

void QtVTKVisualization::SetViewToImageSize()
{
    if (this->imageActor == nullptr) {
        return;
    }

    auto renderer = this->renderWindow->GetRenderers()->GetFirstRenderer();
    auto camera = renderer->GetActiveCamera();
    
    int extent[6];
    this->imageActor->GetMapper()->GetInput()->GetExtent(extent);
    
    double origin[3];
    this->imageActor->GetMapper()->GetInput()->GetOrigin(origin);
    
    double spacing[3];
    this->imageActor->GetMapper()->GetInput()->GetSpacing(spacing);

    const double xc = origin[0] + 0.5*(extent[0] + extent[1])*spacing[0];
    const double yc = origin[1] + 0.5*(extent[2] + extent[3])*spacing[1];
    //const  float xd = (extent[1] - extent[0] + 1)*spacing[0]; // not used
    const double yd = (extent[3] - extent[2] + 1)*spacing[1];

    const double d = camera->GetDistance();

    camera->SetParallelScale(mViewScale * static_cast<double>(yd));
    //camera->SetParallelScale(0.5 * static_cast<double>(xd));
    camera->SetFocalPoint(xc, yc, 0.0);
    camera->SetPosition(xc, yc, -d);
    camera->SetViewUp(0.0, -1.0, 0.0);

    //int *screensize = this->renderWindow->GetScreenSize();
    //this->renderWindow->SetSize(this->renderWindow->GetScreenSize()); // set to full screen
    //this->renderWindow->SetSize(600, 400);
    //this->renderWindow->SetSize(screensize[0]/2, screensize[1]/2);
    //this->renderWindow->SetSize(screensize[0], screensize[1]);
}



