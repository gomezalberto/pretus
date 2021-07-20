#pragma once
#include <QObject>
#include <QList>
#include <memory>
#include <vtkSmartPointer.h>
#include <ifindImage.h>
#include <vtkImageActor.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCornerAnnotation.h>
#include <vtkLookupTable.h>
#include <vtkExtractVOI.h>
#include <vtkScalarBarActor.h>

class BasicVTKVisualization : public QObject{
    Q_OBJECT

public:

    struct Parameters
    {
        Parameters() {
            showruler = false;
            showcolormap = false;
            DisplayMultiLayers = 0;
            base_layer = 0;
            LutId = 15;
        }
        bool showruler;
        bool showcolormap;
        int base_layer;
        int DisplayMultiLayers;
        int LutId;
    };


    BasicVTKVisualization(QObject *parent = 0);

    void Initialize();
    Parameters params;

    void SetLookupTable(unsigned int idx, vtkSmartPointer<vtkLookupTable> arg)
    {
        if (idx != this->LookupTables.length())
        {
            std::cout  << "Cannot append lookup table at idx "<<idx<<std::endl;
            return;
        }
        this->LookupTables.append(arg);
    }


    void SetGrayLookupTable(int id);
    void SetJetLookupTable(int id);
    void SetPredefinedLookupTable(int id, int LUTid);

public Q_SLOTS:
    virtual void slot_Terminate(void);
    virtual void SendImageToWidget(ifind::Image::Pointer image);

    /**
     * @brief Set the relative zslice to be chosen when the image is in 3D.
     * @param arg zslice in percentage of the z-dimension span: between -50 and 50
     *
     * When the input image is 3D, the argument is used to select which slice to be displayed.
     */
    virtual void SetZSlice(int arg);

Q_SIGNALS:
    void ZSliceChanged(int arg);

protected:
    virtual void SetupInteractor();
    virtual void UpdateCornerAnnotation(ifind::Image::Pointer image);

private:

    /**
     * @brief Fit the view to the image size
     */
    void SetViewToImageSize();

    vtkSmartPointer<vtkRenderWindow> renderWindow;
    vtkSmartPointer<vtkCornerAnnotation> corner_anotation;
    QList < vtkSmartPointer<vtkLookupTable> > LookupTables;
    vtkSmartPointer<vtkExtractVOI> SliceExtractor;
    vtkSmartPointer<vtkImageActor> image_actor;
    vtkSmartPointer<vtkScalarBarActor> colorBar;

    uint64_t LatestTimeStamp;
    int ZSlice;

    QList< float > biometrics;
    double         estimated_fetal_weight;

    //void Init2();

};
