# Plug-ins

Author: Alberto Gomez (alberto.gomez@kcl.ac.uk)

# Summary

Plug-ins bring the functionality into the software and can be connected in sequence. All plug-ins inherit from the class [`Plugin`](Plugin.h). This class implements basic operations such as reding arguments from the comand line and receiving configuration parameters, however the actual functionality of the plug-in must be implemented in a separate class, inheriting from [`Worker`](Worker.h). This allows to properly execute the task on a separate, concurrent thread. Plug-ins may optionally implement a graphical interface to display information on a widget. This widget must inherit from the [`QtPluginWidgetBase`](QtPluginWidgetBase.h) class. A number of convenience widget components are included here, and described below.

See the architecture notes below to understand how they work and how to create your own plug-ins. Feel free to use the available plug-ins as templates or examples. The example plug-ins [CppAlgorithm](https://github.com/gomezalberto/pretus/tree/main/src/Plugins/Plugin_CppAlgorithm) and [PythonAlgorithm](https://github.com/gomezalberto/pretus/tree/main/src/Plugins/Plugin_PythonAlgorithm) are a good place to start for simple plug-in templates.

# Overview of the plug-in pipeline system and data streams

PRETUS interconnects plug-ins in a user-specified order, forming a pipeline. The first plug-in will normally generate images at a certain frame rate (for example, capturing images from a frame grabber or reading images from disk). An overview of the software is shown in the figure below:

![pipeline_overview](overview_pipeline.png)

In summary, plug-ins have a forward configuration line (black, top) where any plug-in can share with  downstream plug-ins configuration information, at startup, or at any given time. Then, at a certain frame rate, plugins read any images being transmitted in the data line (bottom) and inject their outputs as a new stream in the data line. All transmissions (configuration and data) are carried out using QT signals and slots. This is explained  below in more detail.

## Pipeline creation, configuration and software start-up

The main application is implemented in a `QApplication` object.

When user launches the program, a QApplication is created and then the pipeline is formed and the following events ocur, in order:

1. The constructor of every plug-in is called
2. Each plug-in is passed any extra command line arguments
3. A **forward configuration line** between plug-ins is established. As a result, if a plug-in generates configuration data, all plug-ins that are located after it in the pipeline will receive configuration information from that plug-in.
4. A **forward data line** between plug-ins is established. As a result, every time a plug-in generates an image, all plug-ins located after it in the pipeline will receive it. As described later, all images are tagged with a *stream* type, and the received images will be ignored by a plug-in if the plug-in is not configured to process images from that *stream*.
5. Any plug-in in the pipeline that can render widgets is passed a list with the widgets of all plug-ins that implement a widget, as well as a prefered location to display them.
6. All the plug-ins are *initialized* in order (their `Initialize()` method is called). This is normally when after initializing all class members a plug-in will emit configuration information downstream the pipeline, if relevant.
7. Last, and after all plug-ins have been initialized, all plugins are *activated*. For most plug-ins this will have no effect since they are active by default, but typically this triggers the first plug-in to start the acquisition process, and causes the image stream to start flowing between plug-ins.

## Streams
One of the key features of the software is the concept of *Streams*. A stream is a sequence of data produced by a plug-in in real-time. Streams have the following properties:

* Streams are named after the plug-in that generates it.
* A plug-in can "inject" a new piece of data into its stream after it has processed an input image. Therefore, the resulting stream might not have a uniform frame rate.
* A plug-in has access to all streams that are generated before that plug-in in the pipeline, and not just the stream from the plug-in immediately before it. 
* Similarly, multiple plug-ins can use the same stream.


# Intra- and Inter- plug-in data transmission

## Intra plug-in details: Interaction between the `Plugin`, the `Worker` and the `Widget`

The `Plugin` object receives the input data and transmits the output downstream, and is responsible for parsing command line arguments. It also may have a `Worker` object, and two `Widget` objects (an actual widget, and an image widget). These objects are interconnected through the plug-in: the plug-in sends and receives data to and from the widgets, and passes it to the worker. This is implemented using Qt signals and slots, and the developer is referred to the example plug-ins [CppAlgorithm](https://github.com/gomezalberto/pretus/tree/main/src/Plugins/Plugin_CppAlgorithm) and [PythonAlgorithm](https://github.com/gomezalberto/pretus/tree/main/src/Plugins/Plugin_PythonAlgorithm) for further details.


## Passing information between plug-ins

The system is designed to work as a succession of plug-ins, when starting from one or more *real-time* streams of data, image data is passed from one plug-in to any or all of the next ones. The data is asynchronously processed by the plug-ins, concurrently, and visualized or saved to disk.

## Concurrent image streams

Every image that is acquired is passed on from the first plug-in, to the next through the real-time path until it reaches the last plug-in. This is done by every plug-in emitting the signal `ImageGenerated(image);` which is connected to the slot `slot_imageReceived(ifind::Image::Pointer image)` of the next plug-ins. This slot does the following:

```c++
    /// Send the image for processing. This image may or may not
    /// be processed depending on the timer's frame rate
    if (this->IsOfExpectedStreamType(image)){
        this->Timer->SetIfindImage(image);
    }

    /// Send the image through the pipeline
    Q_EMIT this->ImageGenerated(image);
```

So images are  passed on to the next plug-in, and passed to the current plug-in's `Timer`. This `Timer` object calls the `worker` of the plug-in at regular intervals to process the image. A plug-in can restrict the type of stream it operates on by re-implementing the virtual method ` virtual bool IsOfExpectedStreamType(ifind::Image::Pointer image);`.

For the plug-in to emit a signal with the processed image, the the plug-in parent class implements, in initialization:

```c++
QObject::connect(this->worker.get(), &Worker::ImageProcessed,
                     this, &Plugin::slot_imageProcessed);
```


## Background execution for plug-ins

Plug-ins, through workers, can execute the task in the background without blocking the real time stream. This is achieved through a number of mechanisms that enable that lengthy processes are either buffered (so that all processed frames are sooner or later processed, potentially introducing drift and latency) or that some frames are dropped to ensure that regardless of how long the processing takes every time a new processing task starts it works on the most recent frame available. These mechanisms, implemented via Qt signals and slots, are the following:


1. A `Timer` class sends the image that has been most recently received by the plug-in to the worker at a  user-defined frame rate. For this, the plug-in `Plugin::Initialize` method implements:

    ```
     this->setFrameRate(20); // fps
     this->Timer->SetDropFrames(true); // if false, processings are buffered
     QObject::connect(this->Timer, &ifindImagePeriodicTimer::ImageReceived,
                         this->worker.get(), &WorkerType::slot_Work);
    ```

2. The `Worker` emits a signal to notify that the processing has finished and this is captured by the `Timer`, that knows that (only if frame dropping is activated) needs to wait for this signal before sending a new image to the worker. This is implemented in the plug-in `Plugin::Initialize` method:

    ```
     QObject::connect(this->worker.get(), &WorkerType::WorkFinished,
                         this->Timer, &ifindImagePeriodicTimer::ReadyToShootOn);
    ```

3. The worker carries out its task in a background thread. This is achieved with:

    ```
    QObject::connect(&this->workerThread, &QThread::finished,
                     this->worker.get(), &QObject::deleteLater);
    this->worker.get()->moveToThread(&this->workerThread);
    workerThread.start();
    ```

All the above is already implemented in the `Plugin` parent class, in the `Initialize` method. As a result, this does not need to be re-implemented, **as long as the user's plug-in `Plugin_XXX` calls the parent's function in it's `Initialize` method**:

```c++
void Plugin_XXX::Initialize(void){
    
    Plugin::Initialize();  // <----------
    
    this->worker->Initialize();
    // Do some plug-in specific initializations
    // ...
    ifind::Image::Pointer configuration = ifind::Image::New();
    // ....
    Q_EMIT this->ConfigurationGenerated(configuration);

    this->Timer->Start(this->TimerInterval);
}
```

This is illustrated in the available examples.

## Order of object creation

First, the plug-ins are loaded, with the `PluginQList plugin_list = LoadPlugins(argc, argv);` call. This triggers **(1) the plug-in constructor**, and **(2) passing the command line arguments to the plug-in**. After this, the plug-ins are **(3) connected** and then **(4) initialized**. In initialization the configuration is passed on to the next plug-in, so the next plug-in should receive the configuration *before* its initialization. Last, the plug-ins are **(4) activated with the `Set Activate(true)`** method.


# Representation of images, metadata and other information in Streams

All data is internally represented as a `ifind::Image` type, which is a specialisation of `itk::Image`. Objects of the class `ifind::Image` can store 2D and 3D image data with one or more layers, and any other information via metadata, in a metadata dictionary in the ITK style. For example, biometrics, predictions, and other information can be stored using image->SetMetaData<>("keyname", keyvalue).

This representation is efficient and highly convenient because as a result the [Image File Writer](https://github.com/gomezalberto/pretus/tree/main/src/Plugins/Plugin_imageFileWriter) plug-in will save images with all layers and metadata to disk.






