#include "Worker_planeDetection.h"
#include <iostream>
#include <QDebug>

#include <pybind11/embed.h>
#include <pybind11/numpy.h>

Worker_planeDetection::Worker_planeDetection(QObject *parent) : Worker(parent){
    this->labels.clear();
    this->confidences.clear();
    this->python_folder = "";
    this->temporalAverage = 0;
    this->background_threshold = -1;
    this->modelname = "ifind2_net_Jan15.pth";
    this->m_write_background = false;
}

void Worker_planeDetection::Initialize(){

    if (!this->PythonInitialized){
        try {
            py::initialize_interpreter(); // this will call Py_Initialize();
        }
        catch (py::error_already_set const &pythonErr) {
            std::cout << pythonErr.what();
        }
    }

    PyGILState_STATE gstate = PyGILState_Ensure();
    {

        py::exec("import sys");
        std::string command = "sys.path.append('" + this->python_folder + "')";
        py::exec(command.c_str());

        py::object processing = py::module::import("worker");
        /// Check for errors
        if (PyErr_Occurred())
        {
            PyErr_Print();
            return;
        }

        /// grabbing the functions from module
        this->PyImageProcessingFunction = processing.attr("dowork");
        py::object getLabelsFunction = processing.attr("getlabels");

        this->PyPythonInitializeFunction = processing.attr("initialize");
        this->PyPythonInitializeFunction(this->python_folder, this->modelname);

        py::list pylabels  = py::list(getLabelsFunction());
        this->labels.clear();
        for (auto& el : pylabels) this->labels.push_back(QString(el.cast<std::string>().data()));
        this->PythonInitialized = true;
    }
    PyGILState_Release(gstate);
}

Worker_planeDetection::~Worker_planeDetection(){
    /// Finalize python stuff
    /// @todo: this is causing segfault.
    py::finalize_interpreter();
}

void Worker_planeDetection::doWork(ifind::Image::Pointer image){

    if (!this->PythonInitialized){
        if (this->params.verbose){
            std::cout << "Worker_planeDetection::doWork() - python not initialised" <<std::endl;
        }
        return;
    }

    if (!Worker::gil_init) {
        if (this->params.verbose){
            std::cout << "Worker_planeDetection::doWork() - init GIL" <<std::endl;
        }
        Worker::gil_init = 1;
        PyEval_InitThreads();
        PyEval_SaveThread();

        ifind::Image::Pointer configuration = ifind::Image::New();
        configuration->SetMetaData<std::string>("Python_gil_init","True");
        Q_EMIT this->ConfigurationGenerated(configuration);
    }

    if (image == nullptr){
        if (this->params.verbose){
            std::cout << "Worker_planeDetection::doWork() - input image was null" <<std::endl;
        }
        return;
    }

    /// Extract central slice and crop

    GrayImageType2D::Pointer image_2d, output_2d;

    /// Use the appropriate layer
    if (this->params.inputLayer >=0){
        ifind::Image::Pointer layerImage = ifind::Image::New();
        layerImage->Graft(image->GetOverlay(this->params.inputLayer));
        image_2d = this->get2dimage(layerImage);
    } else {
        ifind::Image::Pointer layerImage = ifind::Image::New();
        layerImage->Graft(image->GetOverlay(image->GetNumberOfLayers() + this->params.inputLayer));
        image_2d = this->get2dimage(layerImage);
    }

    GrayImageType2D::Pointer image_2d_cropped = this->crop_ifind_2D_image_data(image_2d);

    if (this->params.verbose){
        std::cout << "Worker_planeDetection::doWork() - 2D image extracted" <<std::endl;
    }

    /// Detect the plane
    std::vector <unsigned long> dims = {image_2d_cropped->GetLargestPossibleRegion().GetSize()[1],
                                        image_2d_cropped->GetLargestPossibleRegion().GetSize()[0]};
    if (!image_2d_cropped->GetBufferPointer() || (dims[0] < 50) || (dims[1] < 50))
    {
        qWarning() << "[planedetect] image buffer is invalid";
        return;
    }

    QList<float> confidences_current;

    PyGILState_STATE gstate = PyGILState_Ensure();
    {
        if (this->params.verbose){
            std::cout << "Worker_planeDetection::doWork() - within PyGILState_Ensure;" <<std::endl;
        }
        py::array numpyarray(dims, static_cast<GrayImageType2D::PixelType*>(image_2d_cropped->GetBufferPointer()));
        py::object _function = this->PyImageProcessingFunction;

        if (this->params.verbose){
            std::cout << "Worker_planeDetection::doWork() - within PyGILState_Ensure - call predict;" <<std::endl;
        }
        /// predict standard plane
        py::tuple predictions = py::tuple(_function(numpyarray, false, this->params.verbose));
        //py::array predictions = py::tuple(_function(numpyarray));

        if (this->params.verbose){
            std::cout << "Worker_planeDetection::doWork() - within PyGILState_Ensure - process predictions;" <<std::endl;
        }
        /// Extract network output confidences (list of floating points)
        for (auto l : predictions){
            float lf = l.cast<float>();
            confidences_current.append(lf);
        }

        this->confidences.enqueue(confidences_current);
        while (this->confidences.size()>this->temporalAverage+1){
            this->confidences.dequeue();
        }
        if (this->params.verbose){
            std::cout << "Worker_planeDetection::doWork() - release PyGILState;" <<std::endl;
        }
    }
    PyGILState_Release(gstate);

    /// Sum all confidences
    std::vector<float> confidences_average(confidences_current.size(),0.0);
    for (auto l : confidences){
        for (int i=0; i<l.size(); i++){
            float lf = l[i] ; //.cast<float>();
            confidences_average[i]+=lf;
        }
    }
    /// Now divide by the temporal average,
    double max_confidence = -10000, min_confidence = 10000;
    int max_confidence_id = -1; // by default, background
    for (int i=0; i<confidences_average.size(); i++){
        confidences_average[i]/=(this->temporalAverage+1.0);
        if (confidences_average[i]>max_confidence){
            max_confidence  = confidences_average[i];
            max_confidence_id = i;
        }
        if (confidences_average[i]<min_confidence){
            min_confidence  = confidences_average[i];
        }
    }

    /// Now normalize to [0, 1] confidence
    for (int i=0; i<confidences_average.size(); i++){
        confidences_average[i]= (confidences_average[i]-min_confidence) /(max_confidence-min_confidence);
    }

    const int BACKGROUND_IDX = 3;
    if (this->background_threshold > 0){
        for (int i=0; i<confidences_average.size(); i++){
            if (i == BACKGROUND_IDX) {
                if (confidences_average[i] < this->background_threshold){
                    //std::cout << "Worker_planeDetection::doWork() - Confidences for "<< i << " are " << confidences_average[i]<<
                    //             ", and the threshold is "<< this->background_threshold << " so we set to 0"<< std::endl;
                    confidences_average[i] = 0;
                }
            }
        }
    }

    // convert to a string
    QStringList confidences_str;
    for (int i=0; i<confidences_average.size(); i++){

        confidences_str.append(QString::number(confidences_average[i]));
    }

    if (max_confidence_id <0){
        /// This can occur if confidences are NaN
        if (this->params.verbose){
            std::cout << "Worker_planeDetection::doWork() - image processed, but could not find any class -> confidence values were NaN : "<< max_confidence_id <<std::endl;
        }
        return;
    }

    /* if (std::strcmp(this->labels[max_confidence_id].toStdString().c_str(),"Background")==0){
        // We do not add Background images to the stream
        return;
    }
    */

    /// Now add the classification information to the image and send it to the next plug-in.
    image->SetMetaData<std::string>( mPluginName.toStdString() +"_labels", this->labels.join(",").toStdString() );
    image->SetMetaData<std::string>( mPluginName.toStdString() + "_confidences", confidences_str.join(",").toStdString() );
    image->SetMetaData<std::string>( mPluginName.toStdString() + "_label", this->labels[max_confidence_id].toStdString() );
    image->SetMetaData<std::string>( mPluginName.toStdString() + "_confidence", confidences_str[max_confidence_id].toStdString() );
    image->SetMetaData<std::string>( mPluginName.toStdString() + "_bckth", QString::number(this->background_threshold).toStdString() );
    image->SetMetaData<std::string>( mPluginName.toStdString() + "_tempAvg", QString::number(this->temporalAverage).toStdString() );


    if (!this->m_write_background && this->labels[max_confidence_id]=="Background"){
        image->SetMetaData<std::string>( "DO_NOT_WRITE", "DO_NOT_WRITE");
    }



    Q_EMIT this->ImageProcessed(image);

    if (this->params.verbose){
        std::cout << "Worker_planeDetection::doWork() - image processed, class: "<< this->labels[max_confidence_id].toStdString() <<std::endl;
    }

}

void  Worker_planeDetection::slot_bckThresholdValueChanged(int v){
    /// somehow communicate with the worker.
    this->background_threshold = double(v)/100.0;
}

void  Worker_planeDetection::slot_temporalAverageValueChanged(int v){
    /// somehow communicate with the worker.
    this->temporalAverage = double(v);
}

QStringList Worker_planeDetection::getLabels() const
{
    return this->labels;
}

void Worker_planeDetection::setLabels(const QStringList &value)
{
    this->labels = value;
}


