#pragma once
#include <QWidget>
#include "QtPluginWidgetBase.h"
#include <ifindImage.h>
#include <QPaintEvent>
#include <vtkSmartPointer.h>
#include <vtkImageActor.h>
#include <vtkLookupTable.h>
#include <vtkExtractVOI.h>
#include <vtkRenderWindow.h>
#include <vtkScalarBarActor.h>
#include <ifindStreamTypeHelper.h>
#include <QVector2D>

namespace {

void ClickCallbackFunction(vtkObject* caller, long unsigned int eventId,
                           void* clientData, void* callData);
}

class QVTKWidget;

class QtVTKVisualization : public QtPluginWidgetBase
{
    Q_OBJECT

public:

    class Parameters
    {
    public:
        Parameters();

        const bool ShowRuler() const;
        const bool ShowColorbar() const;
        const int BaseLayer() const;
        const int OverlayLayer() const;
        const int LutId() const;

        static void Usage();
        void SetCommandLineArguments(const std::vector<std::string> &args);

        void SetBaseLayer(int a);
        /**
         * @brief SetOverlayLayer
         * @param the id of the overlay layer. If negative, counts from the last.
         */
        void SetOverlayLayer(int a);
        void SetLutId(int a);
        void SetShowColorbar(bool a);

    private:

        bool mShowColorbar;
        bool mShowRuler;
        /**
         * @brief  Base Layer
         * Display (if available) the requested layer as base layer
         */
        int mBaseLayer;
        /**
         * @brief Set Overlay layer.
         * no overlay (0) or activate (1,... n_overlays)  displaying two layers
         * The base layer, and the one indicated here.
         *  If negative, starts from last: -1 is the last, -2 the one before the last, etc.
         */
        int mOverlayLayer;
        /**
         * @brief Lookup table  id
         * 0: SPECTRUM, 1: WARM, 2: COOL, 3: BLUES, 4: WILD_FLOWER, 5: CITRUS
         */
        int mLutId;
    };

    QtVTKVisualization(QWidget *parent = 0);

    void Initialize();

    const Parameters &Params() const;
    void SetParams(const Parameters &params);

    QVTKWidget *GetQVTKWidget();

    virtual void SendImageToWidgetImpl(ifind::Image::Pointer image);





public Q_SLOTS:
    virtual void slot_Terminate(void);

    /**
     * @brief Set the relative zslice to be chosen when the image is in 3D.
     * @param arg zslice in percentage of the z-dimension span: between -50 and 50
     *
     * When the input image is 3D, the argument is used to select which slice to be displayed.
     */
    virtual void SetZSlice(int newZSlice);
    virtual void SetViewScale(float viewScale);
    virtual void EnableOverlay(bool enabled);

Q_SIGNALS:
    void ZSliceChanged(int arg);
    void PointPicked(QVector2D &vec);

protected:
    virtual void SetupInteractor();

    // override, in order to be able to reize the vtk rendering
    virtual void resizeEvent(QResizeEvent *event);
    float mViewScale;


private:

    void SetGrayLookupTable(int id);
    void SetJetLookupTable(int id);
    void SetPredefinedLookupTable(int id, int LUTid);
    void SetLookupTable(unsigned int idx, vtkSmartPointer<vtkLookupTable> arg);

    /**
     * @brief Fit the view to the image size
     */
    void SetViewToImageSize();

    // raw pointer parented to 'this' - QT hierarchy will/should destroy
    // Also using a deprecated class as the suggested update is incompatible with 
    // our -DQT_NO_KEYWORDS declaration in the CMake - is uses slots rather than Q_SLOTS still
    QVTKWidget *vtkWidget;

    vtkSmartPointer<vtkRenderWindow> renderWindow;
    QList < vtkSmartPointer<vtkLookupTable> > lookupTables;
    vtkSmartPointer<vtkExtractVOI> sliceExtractor;
    vtkSmartPointer<vtkImageActor> imageActor;
    vtkSmartPointer<vtkScalarBarActor> colorBar;

    uint64_t latestTimeStamp;
    int zSlice;
    bool mShowOverlay;

    Parameters mParams;
};
