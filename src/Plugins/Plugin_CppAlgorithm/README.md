# Cpp Algorithm Plug-in (a tutorial)


# Summary

This plug-in carries out a simple image processing task in C++, and is provided as a tutorial exemplifying how to write plug-ins for PRETUS. This readme describes the process of creating the plug-in including the processing itself and visualization widgets for the results.

The task of choice is a simple image segmentation task based on thresholding using ITK.

# 1. Set up the files

A basic plug-in will needs one class (the plugin itself) and optionally three more classes: the worker (if the taask is to be done in a separate thread, or at a certain interval), and the two widgets: one for the metadata, and one for images. In this case we will implement all four:

* Class `Plugin_CppAlgorithm` will inherit from `Plugin` and manage argument passing, plugin initialization and configuration.
* Class `Worker_CppAlgorithm` will inherit from `Worker` and will carry out the processing task on a separate thread. This class is not required if the task does not need a separate thread and is simple enough to be incorporated into the `Plugin` class.
* Class `Widget_CppAlgorithm` will implement the widget where results and information about the plug-in are shown.
* For the image widget we will use the provided `QtVTKVisualization` class that performs basic image and overlay visualization.

# 2. Implementing the `Plugin_CppAlgorithm` class

The class `Plugin_CppAlgorithm` header file is listed below, with self-explanatory comments. Its implementation is quite unspecific to the worker task, hence the provided template might be useful for a number of plug-ins. If there is no widget, the widget functions can be omitted. The initialize function can also be omitted if there is nothign to initialize.

```cpp
#pragma once

#include <Plugin.h>
#include "Worker_CppAlgorithm.h"
#include "Widget_CppAlgorithm.h"
#include <QtVTKVisualization.h>

class Plugin_CppAlgorithm : public Plugin {
    Q_OBJECT

public:

    typedef Worker_CppAlgorithm WorkerType;
    typedef Widget_CppAlgorithm WidgetType;
    typedef QtVTKVisualization ImageWidgetType;

    Plugin_CppAlgorithm(QObject* parent = 0);

    QString GetPluginName(void){ return "Cpp Algorithm";}
    QString GetPluginDescription(void) {return "Sample plug-in that does a simple threshold based segmentation.";}

    void SetCommandLineArguments(int argc, char* argv[]);

    /**
    * Initialize the plug-in: take in the command line arguments,
    * and any configuration passed on by previous plug-ins. Send
    * configuration options to downstream plug-ins.
    */
    void Initialize(void);


public Q_SLOTS:
    /**
     * Receive configuration information from upstream plug-ins
     */
    virtual void slot_configurationReceived(ifind::Image::Pointer image);

protected:
    virtual void SetDefaultArguments();

};

```


The implementation of the different functions is as follows. The constructor needs to initialize the worker and the widgets, and set their default location. Also, it needs to set the default configuration for real-time processing in the pipeline. This is achieved by 1) setting the strem type that the plug-in processes to "Input" by default, 2) setting a framerate at which the worker thread will be invoked (20 fps by default) and 3) Enabling frame dropping (which means that if it is time to process a new image but the current processing has not finished, the upcoming frame is dropped to ensure no cumulated latency). Because we want to have the image widget show the input image and the segmentation overlay, we need to configure the image widget accordingly. By default, the image widget is not shown (instead, the input stream will be shown in the GUI plugin), but we will later show how this can be changed at run time:

```cpp
Plugin_CppAlgorithm::Plugin_CppAlgorithm(QObject *parent) : Plugin(parent)
{
      // Construct the worker
    {
        WorkerType::Pointer worker_ = WorkerType::New();
        this->worker = worker_;
    }
    this->mStreamTypes = ifind::InitialiseStreamTypeSetFromString("Input");
    this->setFrameRate(20); // by default 15fps
    this->Timer->SetDropFrames(true);

    {
        // create widget
        WidgetType * mWidget_ = new WidgetType;
        this->mWidget = mWidget_;

        // We want a slider in the widget to change, in real time, the threshold value. For this, we need to connect the slider in the widget to the worker:

        WorkerType *w = std::dynamic_pointer_cast< WorkerType >(this->worker).get();
        QObject::connect(mWidget_->mSlider, &QSlider::valueChanged, w, &WorkerType::slot_sigmaValueChanged);
    }
    {
        // create image widget
        ImageWidgetType * mWidget_ = new ImageWidgetType;
        this->mImageWidget = mWidget_;
        this->mImageWidget->SetStreamTypes(ifind::InitialiseStreamTypeSetFromString(this->GetCompactPluginName().toStdString()));
        this->mImageWidget->SetWidgetLocation(ImageWidgetType::WidgetLocation::hidden); // by default, do not show

        // set image viewer default options:
        // overlays, colormaps, etc
        ImageWidgetType::Parameters default_params = mWidget_->Params();
        default_params.SetBaseLayer(0); // use the input image as background image
        default_params.SetDisplayMultiLayers(1); // show 1 layer on top of the background
        default_params.SetLutId(15);
        default_params.SetShowColorbar(false);
        mWidget_->SetParams(default_params);
    }

    this->SetDefaultArguments();
}
```

The `Initialize` method first needs to call the parent initialization function, and then initialize the worker and the image widget.  Then, the method generates any configuration that needs to be passed on to the next plug-ins, and finally the timer that controls the execution of the worker at the user defined frame rate is triggered. In this case, there is no configuration to be passed on, so the configuration lines are commented out.

```cpp
void Plugin_CppAlgorithm::Initialize(void){
Plugin::Initialize();
    reinterpret_cast< ImageWidgetType *>(this->mImageWidget)->Initialize();
    this->worker->Initialize();

    //ifind::Image::Pointer configuration = ifind::Image::New();
    //Q_EMIT this->ConfigurationGenerated(configuration);

    this->Timer->Start(this->TimerInterval);
}
```

The `slot_configurationReceived` method, in this case, does not need any information from upstream plug-ins. As a result, this method just propagates the configuration data to downstream plug-ins:

```cpp
void Plugin_CppAlgorithm::slot_configurationReceived(ifind::Image::Pointer image){

    /// Pass on the message in case we need to "jump" over plug-ins
    Q_EMIT this->ConfigurationGenerated(image);
}
```

Last, we need to add any command line arguments that are needed in this plug-in. This can be done by overloading the functions `SetDefaultArguments()` and `SetCommandLineArguments()`. In this case, the only argument is a number indicating the intesity threshold value:
```cpp
void Plugin_CppAlgorithm::SetDefaultArguments(){

    // arguments are defined with: name, placeholder for value, argument type,  description, default value
    mArguments.push_back({"threshold", "<val>",
                            QString( ArgumentType[1] ),
                            "Intensity threshold for segmentation.",
                              QString::number(std::dynamic_pointer_cast< WorkerType >(this->worker)->mThreshold )});

}

void Plugin_CppAlgorithm::SetCommandLineArguments(int argc, char* argv[]){
    Plugin::SetCommandLineArguments(argc, argv);
    InputParser input(argc, argv, this->GetCompactPluginName().toLower().toStdString());
    {const std::string &argument = input.getCmdOption("threshold");
        if (!argument.empty()){
            std::dynamic_pointer_cast< WorkerType >(this->worker)->mThreshold = atof(argument.c_str());
        }}
}
```

We have defined two functions, to describe the argument, and to read it.



# 3. Implementing the `Worker_CppAlgorithm` class

The worker is a relatively straightforward class. Three funtions need to be implemented: the constructor, the `Initialize()` function (after user options and arguments have been passed, not used in this example) and the `doWork()` function which impements the actual task. A slot is implemented to capture changes in the slider from the widget. The header also declares any other members or auxiliary functions if needed, in this case the threshold variable:

```cpp
#pragma once

#include <Worker.h>
#include <memory>

/// For image data. Change if image data is different
#include <ifindImage.h>

class Worker_CppAlgorithm : public Worker{
    Q_OBJECT

public:

    typedef Worker_CppAlgorithm            Self;
    typedef std::shared_ptr<Self>       Pointer;

    /** Constructor */
    static Pointer New(QObject *parent = 0) {
        return Pointer(new Self(parent));
    }

    unsigned char mThreshold;

public Q_SLOTS:

    virtual void slot_thresholdValueChanged(int v);

protected:
    Worker_CppAlgorithm(QObject* parent = 0);
    void doWork(ifind::Image::Pointer image);
};
```

In this case, the constructor just needs to initialize the threshold variable:
```cpp
Worker_CppAlgorithm::Worker_CppAlgorithm(QObject *parent) : Worker(parent){
    this->mThreshold = 50;
}

```
The slot to capture the changes in threshold value is straightforward:
```cpp
void  Worker_CppAlgorithm::slot_thresholdValueChanged(int v){
    /// somehow communicate with the worker.
    this->mThreshold = v;
}
```

There is nothing to do in the Initialize function, so no need to reimplement it from the parent class. The `doWork()` function implements the thresholding operation as follows:

```cpp
void Worker_CppAlgorithm::doWork(ifind::Image::Pointer image){

    if (image == nullptr){
        if (this->params.verbose){
            std::cout << "Worker_CppAlgorithm::doWork() - input image was null" <<std::endl;
        }
        return;
    }

    using FilterType = itk::BinaryThresholdImageFilter<ifind::Image, GrayImageType>;
    FilterType::Pointer filter = FilterType::New();
    filter->SetInput(image);
    filter->SetLowerThreshold(this->mThreshold);
    filter->SetUpperThreshold(255);
    filter->SetOutsideValue(0);
    filter->SetInsideValue(255);
    filter->Update();

    GrayImageType::Pointer segmentation = filter->GetOutput();

    image->GraftOverlay(segmentation.GetPointer(), image->GetNumberOfLayers());
    image->SetMetaData<int>(this->mPluginName.toStdString() + "_segmentation", image->GetNumberOfLayers() );
    image->SetMetaData<std::string>(this->mPluginName.toStdString() + "_threshold",  QString::number(this->mThreshold).toStdString());

    Q_EMIT this->ImageProcessed(image);
```
First, we check that the inputimage is not `nullpt`. Then we create a standard thresholding pipeline using the ITK library. Because the output of this filter is a segmentation, we add this to the input image using the `GraftOverlay` function, and add metadfata indicating what layer number the segmentation is in (in case the input image had overlays already). Last, we add the threshold value used to the metadata.

The last line emits the ImageProcessed signal, sending the image with overlay and metadata. This will be injected in the `cppalgorithm` stream, and also sent to the imagewidget and to the plugin widget.


# 4. Implementing the `Widget_CppAlgorithm` class

For this plug-in, the widget will simply show the user what is the threshold being used, and allow the user to change it with a slider. The class implements the constructor (when the GUI elements are declared) and the `SendImageToWidgetImpl` class, where the GUI is updated every time a new image is processed. 

```cpp
#pragma once
#include <QWidget>
#include <ifindImage.h>
#include <QtPluginWidgetBase.h>

class QLabel;
class QSlider;

class Widget_CppAlgorithm : public QtPluginWidgetBase
{
    Q_OBJECT

public:
    Widget_CppAlgorithm(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    virtual void SendImageToWidgetImpl(ifind::Image::Pointer image);

    QSlider *mSlider;
private:
    QLabel *mLabel;
};

```

The important thing in the constructor is to define the stream type as the plugin output. Here we also initialize the slider for the threshold, and at the end we add a call to the `AddImageViewCheckboxToLayout` method which will enable to show or hide the associated image widget.

```cpp
Widget_CppAlgorithm::Widget_CppAlgorithm(QWidget *parent, Qt::WindowFlags f): QtPluginWidgetBase(parent, f)
{
    this->SetWidgetLocation(WidgetLocation::top_right);
    mStreamTypes = ifind::InitialiseStreamTypeSetFromString("CppAlgorithm");

    //------ define gui ----------
    mLabel = new QLabel("Text not set", this);
    mLabel->setStyleSheet("QLabel { background-color : black; color : white; }");

    auto labelFont = mLabel->font();
    labelFont.setPixelSize(15);
    labelFont.setBold(true);
    mLabel->setFont(labelFont);
    //
    mSlider = new QSlider(Qt::Orientation::Horizontal);
    mSlider ->setStyleSheet("QSlider { background-color : black}"
    "QSlider::handle:horizontal { background: white; border: 2px solid white;  border-radius: 3px;}");
    mSlider->setMaximum(256);
    mSlider->setMinimum(1);
    mSlider->setAutoFillBackground(true);


    auto vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);
    this->setLayout(vLayout);

    vLayout->addWidget(mLabel);
    vLayout->addWidget(mSlider);
    this->AddImageViewCheckboxToLayout(vLayout);
}
```
In addition, we also define the default location for the widget, and the GUI elements using the Qt library. 

The `SendImageToWidgetImpl` function updates the GUI elements:

```cpp
void Widget_CppAlgorithm::SendImageToWidgetImpl(ifind::Image::Pointer image){

    std::stringstream stream;
    stream << "==" << this->mPluginName.toStdString() << "==" << std::endl;

    if (image->HasKey("CppAlgorithm_threshold")){
        stream << "Threshold: "<< image->GetMetaData<std::string>("CppAlgorithm_threshold") << std::endl;
    }

    mLabel->setText(stream.str().c_str());

    Q_EMIT this->ImageAvailable(image);
}
```

#5. Using the `CppAlgorithm` plugin

To test the plug-in within PRETUS, we build a simple pipeline where we read a video from file, apply the thresholding, and visualize the results. To this end we use, in this order, the `video manager` plug-in, the `Cpp Algorithm` plug-in, and the `GUI` plug-in, which have numbers 2, 3 and 5 respectively. These numbers may change depending on the build so the user should check by typing `pretus -h`. Then we call PRETUS as follows:

```
./bin/pretus -pipeline "2>3>5"  --videomanager_input /home/username/videos/video.MP4  --cppalgorithm_threshold 80  --videomanager_showimage 1 --cppalgorithm_showimage 1
```

With this call, we display both the input stream and show the output of the `CppAlgorithm` plug-in (`--cppalgorithm_showimage 1` option) side by side. An animation of the result is shown in the figure below. The program can be closed by entering `quit` in the command line.


![demo.gif](demo.gif)


This concludes the tutorial for implementing this plug-in. The `Plugin_PythonAlgorithm` describes more advanced options and how to use python from within plugins.

